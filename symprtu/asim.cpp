/* Assimilator: Process finished tasks and update segments. */
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <bitset>
#include <ctime>
#include <unistd.h>
#include <cstdlib>
#include <sys/stat.h>

#include "boinc_api.h"
#include "Stream.cpp"

using std::vector;
using std::cerr;
using std::endl;
using std::string;

#include "config.h"
#include "backend_lib.h"
#include "error_numbers.h"
#include "sched_config.h"
#include "sched_util.h"
#include "validate_util.h"
#include "credit.h"
#include <mysql.h>

#include "wio.cpp"

//TODO: deduplicate this code

struct EApp : std::runtime_error { using std::runtime_error::runtime_error; };
struct EBoincApi : std::exception {
	int retval;
	const char* msg;
	std::string msg2;
	EBoincApi(int _retval, const char* _msg)
		:retval(_retval), msg(_msg)
	{
		std::stringstream ss;
		ss<<"boinc_api: "<<msg<<": "<<boincerror(retval);
		msg2=ss.str();
	}
	const char * what () const noexcept {return msg2.c_str();}
};
struct EDatabase	: std::runtime_error { using runtime_error::runtime_error; };
struct EInvalid	: std::runtime_error { using runtime_error::runtime_error; };
static int retval;

class CFileStream
	: public CDynamicStream
{
	public:
	using CDynamicStream::CDynamicStream;
	CFileStream( const char* name )
	{
		FILE* f = boinc_fopen(name, "r");
		if(!f) {
			//bug: boinc on windows is stupid and this call does not set errno if file does not exist
			//Go to hell!!
			if(errno==ENOENT) throw EFileNotFound();
			if(!boinc_file_exists(name)) throw EFileNotFound();
			throw std::runtime_error("fopen");
		}
		struct stat stat_buf;
		if(fstat(fileno(f), &stat_buf)<0)
			throw std::runtime_error("fstat");
		this->setpos(0);
		this->reserve(stat_buf.st_size);
		if( fread(this->getbase(), 1, stat_buf.st_size, f) !=stat_buf.st_size)
			throw std::runtime_error("fread");
		this->setpos(0);
		fclose(f);
	}
	void writeFile( const char* name )
	{
		FILE* f = boinc_fopen("tmp_write_file", "w");
		if(!f)
			throw std::runtime_error("fopen");
		if( fwrite(this->getbase(), 1, this->pos(), f) !=this->pos())
			throw std::runtime_error("fwrite");
		fclose(f);
		if( rename("tmp_write_file",name) <0)
			throw std::runtime_error("rename");
	}
};

DB_APP spt_app;
DB_APP stpt_app;
MYSQL_STMT* spt_result_stmt;

void initz() {
	int retval = config.parse_file();
	if (retval) {
			log_messages.printf(MSG_CRITICAL,
					"Can't parse config.xml: %s\n", boincerror(retval)
			);
			exit(1);
	}

	retval = boinc_db.open(
			config.db_name, config.db_host, config.db_user, config.db_passwd
	);
	if (retval) {
			log_messages.printf(MSG_CRITICAL,
					"boinc_db.open failed: %s\n", boincerror(retval)
			);
			exit(1);
	}
	if (spt_app.lookup("where name='spt'")) {
		std::cerr<<"can't find app spt\n";
		exit(4);
	}
	if (stpt_app.lookup("where name='stpt'")) {
		std::cerr<<"can't find app stpt\n";
		exit(4);
	}
	
	spt_result_stmt = mysql_stmt_init(boinc_db.mysql);
	char stmt[] = "insert into spt_result SET id=?, input=?, output=?, uid=?";
	if(mysql_stmt_prepare(spt_result_stmt, stmt, sizeof stmt ))
		throw EDatabase("spt_result insert prepare");
}

int read_output_file(RESULT const& result, CDynamicStream& buf) {
    char path[MAXPATHLEN];
		path[0]=0;
    string name;
		double usize = 0;
		double usize_max = 0;
    MIOFILE mf;
    mf.init_buf_read(result.xml_doc_out);
    XML_PARSER xp(&mf);
    while (!xp.get_tag()) {
			if (!xp.is_tag) continue;
			if (xp.match_tag("file_info")) {
				while(!xp.get_tag()) {
					if (!xp.is_tag) continue;
					if(xp.parse_string("name",name)) continue;
					if(xp.parse_double("nbytes",usize)) continue;
					if(xp.parse_double("max_nbytes",usize_max)) continue;
					if (xp.match_tag("/file_info")) {
						if(!name[0] || !usize) {
							return ERR_XML_PARSE;
						}
						dir_hier_path(
							name.c_str(), config.upload_dir,
							config.uldl_dir_fanout, path
						);

						FILE* f = boinc_fopen(path, "r");
						if(!f && ENOENT==errno) return ERR_FILE_MISSING;
						if(!f) return ERR_READ;
						struct stat stat_buf;
						if(fstat(fileno(f), &stat_buf)<0) return ERR_READ;
						buf.setpos(0);
						buf.reserve(stat_buf.st_size);
						if( fread(buf.getbase(), 1, stat_buf.st_size, f) !=stat_buf.st_size)
							return ERR_READ;
						buf.setpos(0);
						fclose(f);
						return 0;
					}
				}
			}
		}
    return ERR_XML_PARSE;
}

const float credit_m= 3.748e-10;
//credit/200 = gigaflop

void process_result(DB_RESULT& result) {
	std::stringstream qr;
	DB_WORKUNIT wu;
	if(wu.lookup_id(result.workunitid)) throw EDatabase("Workunit not found");
	// Read the result file
	CDynamicStream buf;
	retval=read_output_file(result,buf);
	/* edit: skip processing if file error */
	if(retval && 0) {
		cerr<<"error: Can't read the output file. "<<result.name<<endl;
		return;
	}
	if(ERR_FILE_MISSING==retval) throw EInvalid("Output file absent");
	if(retval) throw EDatabase("can't read the output file");
	TOutput rstate;
	try {
		rstate.readOutput(std::move(buf));
	} catch (EStreamOutOufBounds& e){ throw EInvalid("can't deserialize output file"); }

	CDynamicStream inbuf;
	try {
		std::stringstream fn;
		fn<<"download/"<<wu.name<<".in";
		inbuf = CFileStream( fn.str().c_str() );
	} catch (EStreamOutOufBounds& e){ throw EDatabase("can't read input file"); }

	if(rstate.status!=TOutput::x_end)
		throw EInvalid("incomplete run");
	// check if the tuple offsets are all even and nonzero
	for( const auto& tuple : rstate.tuples) {
		if(tuple.k==0)
			throw EInvalid("bad tuple k");
		for(unsigned i=1; i<tuple.ofs.size(); ++i) {
			if( (tuple.ofs[i]==0) // must not be zero
				||(tuple.ofs[i]&1)  // must be even
			) throw EInvalid("bad tuple offset");
		}
	}

	//TODO: more consistency checks

	/* Insert into result db */
	unsigned long bind_2_length = inbuf.length();
	unsigned long bind_3_length = buf.length();
	MYSQL_BIND bind[] = {
		{.buffer=&result.id, .buffer_type=MYSQL_TYPE_LONG, 0},
		{.length=&bind_2_length, .buffer=inbuf.getbase(), .buffer_type=MYSQL_TYPE_BLOB, 0},
		{.length=&bind_3_length, .buffer=buf.getbase(), .buffer_type=MYSQL_TYPE_BLOB, 0},
		{.buffer=&result.userid, .buffer_type=MYSQL_TYPE_LONG, 0}
	};
	if(mysql_stmt_bind_param(spt_result_stmt, bind))
		throw EDatabase("spt_result insert bind");
	if(mysql_stmt_execute(spt_result_stmt))
		throw EDatabase("spt_result insert");

	/* insert into the prime tuple db */
	for( const auto& tuple : rstate.tuples) {
		qr=std::stringstream();
		qr<<"insert into spt set batch="<<result.batch;
		qr<<", start="<<tuple.start;
		qr<<", k="<<tuple.k;
		qr<<", kind='spt'";
		if(tuple.k==0)
			throw EInvalid("bad tuple k");
		qr<<", ofs='"<<tuple.ofs[0];
		for(unsigned i=1; i<tuple.ofs.size(); ++i) {
			if(tuple.ofs[i]<=1)
				throw EInvalid("bad spt ofs");
			qr<<" "<<tuple.ofs[i];
		}
		qr<<"' on duplicate key update id=id";
		retval=boinc_db.do_query(qr.str().c_str());
		if(retval) throw EDatabase("spt row insert failed");
	}

	/* insert into the twin prime tuple db */
	for( const auto& tuple : rstate.twins) {
		qr=std::stringstream();
		qr<<"insert into spt set batch="<<result.batch;
		qr<<", start="<<tuple.start;
		qr<<", k="<<(tuple.ofs.size()+1);
		qr<<", kind='tpt'";
		if(tuple.ofs.size()==0)
			throw EInvalid("bad tuple size");
		qr<<", ofs='"<<tuple.ofs[0];
		for(unsigned i=1; i<tuple.ofs.size(); ++i) {
			if(tuple.ofs[i]<=2)
				throw EInvalid("bad tpt ofs");
			qr<<" "<<tuple.ofs[i];
		}
		qr<<"' on duplicate key update id=id";
		retval=boinc_db.do_query(qr.str().c_str());
		if(retval) throw EDatabase("spt row insert failed");
	}

	/* insert into the symmetric twin prime tuple db */
	for( const auto& tuple : rstate.twin_tuples) {
		qr=std::stringstream();
		qr<<"insert into spt set batch="<<result.batch;
		qr<<", start="<<tuple.start;
		qr<<", k="<<(tuple.ofs.size()+1);
		qr<<", kind='stpt'";
		if(tuple.k==0)
			throw EInvalid("bad tuple k");
		qr<<", ofs='"<<tuple.ofs[0];
		for(unsigned i=1; i<tuple.ofs.size(); ++i) {
			if(tuple.ofs[i]<=2)
				throw EInvalid("bad stpt ofs");
			qr<<" "<<tuple.ofs[i];
		}
		qr<<"' on duplicate key update id=id";
		retval=boinc_db.do_query(qr.str().c_str());
		if(retval) throw EDatabase("spt row insert failed");
	}

	//TODO
	float credit = credit_m* (rstate.last-rstate.start);

	/*
	size_t logsz = strlen(result.stderr_out);
	snprintf(result.stderr_out+logsz,BLOB_SIZE-logsz,"Validator: OK! Log deleted to save space. "
		"ODLS=%lu CF=%lu SN=%llu ended=%d seg_dbid=%ld res_dbid=%lu\n",
		(unsigned long)rstate.odlk.size(),
		(unsigned long)rstate.nkf,
		(unsigned long long)rstate.nsn,
		(int)rstate.ended,
		have_segment? (long)segment.id : -1,
		(long)result_id
	);*/
	DB_HOST host;
	qr=std::stringstream();
	if(host.lookup_id(result.hostid)) throw EDatabase("Host not found");
	//is_valid
	double turnaround = result.received_time - result.sent_time;
	compute_avg_turnaround(host, turnaround);
	DB_HOST_APP_VERSION hav,hav0;
	retval = hav_lookup(hav0, result.hostid,
			generalized_app_version_id(result.app_version_id, result.appid)
	);
	hav=hav0;
	hav.max_jobs_per_day++;
	hav.consecutive_valid++;
	//grant_credit
	result.validate_state=VALIDATE_STATE_VALID;
	result.file_delete_state=FILE_DELETE_READY;
	if(result.granted_credit==0) {
		result.granted_credit = credit;
		grant_credit(host, result.sent_time, result.granted_credit);
		if (config.credit_by_app) {
			grant_credit_by_app(result, credit);
		}
	}
	if(host.update()) throw EDatabase("Host update error");
	//update result (?)
	if(result.update()) throw EDatabase("Result update error");
	//update wu
	wu.assimilate_state = ASSIMILATE_DONE;
	//wu.file_delete_state=FILE_DELETE_READY;
	wu.need_validate = 0;
	wu.transition_time = time(0);
	//todo: unsent -> RESULT_OUTCOME_DIDNT_NEED
	if(wu.canonical_resultid==0) {
		wu.canonical_resultid = result.id;
		wu.canonical_credit = result.granted_credit;
	}
	if(wu.update()) throw EDatabase("Workunit update error");
	if (hav.host_id && hav.update_validator(hav0)) throw EDatabase("Host-App-Version update error");
}

void set_result_invalid(DB_RESULT& result) {
	DB_WORKUNIT wu;
	if(wu.lookup_id(result.workunitid)) throw EDatabase("Workunit not found");
	DB_HOST_APP_VERSION hav, hav0;
	retval = hav_lookup(hav0, result.hostid,
			generalized_app_version_id(result.app_version_id, result.appid)
	);
	hav= hav0;
	hav.consecutive_valid = 0;
	if (hav.max_jobs_per_day > config.daily_result_quota) {
			hav.max_jobs_per_day--;
	}
	//TODO: reset workunit transition time
	result.validate_state=VALIDATE_STATE_INVALID;
	//result.file_delete_state=FILE_DELETE_READY; - keep for analysis
	if(result.update()) throw EDatabase("Result update error");
	if(wu.update()) throw EDatabase("Workunit update error");
	if (hav.host_id && hav.update_validator(hav0)) throw EDatabase("Host-App-Version update error");
}

int main(int argc, char** argv) {
	bool f_write;
	long gen_limit;
	int batchno;
	char *check1;
	if(argc!=3) {
			cerr<<"Expect 2 command line argument: f_write limit"<<endl;
			exit(2);
	}
	f_write = (argv[1][0]=='y');
	gen_limit = strtol(argv[2],&check1,10);
	if((argv[1][0]!='n' && !f_write) || *check1) {
			cerr<<"Invalid argument format"<<endl;
			exit(2);
	}
	cerr<<"f_write="<<f_write<<" limit="<<gen_limit<<endl;
	//connect db if requested
	initz();
	if(boinc_db.start_transaction())
		exit(4);
	//enumerate results
	std::stringstream enum_qr;
	//enum_qr<<"where appid="<<spt_app.id
	enum_qr<<"where appid in ("<<spt_app.id<<","<<stpt_app.id<<")"
	<<" and server_state="<<RESULT_SERVER_STATE_OVER
	<<" and outcome="<<RESULT_OUTCOME_SUCCESS<<" and validate_state="<<VALIDATE_STATE_INIT<<" limit "<<gen_limit<<";";
	DB_RESULT result;
	while(1) {
		int retval= result.enumerate(enum_qr.str().c_str());
		if(retval) {
			if (retval != ERR_DB_NOT_FOUND) {
				cerr<<"db error"<<endl;
				exit(4);
			}
			break;
		}
		std::cout<<"result "<<result.name<<endl;
		try {
			process_result(result);
		} catch (EInvalid& e) {
			std::cout<<" Invalid: "<<e.what()<<endl;
			strncat(result.stderr_out,"Validator: ",BLOB_SIZE-1);
			strncat(result.stderr_out,e.what(),BLOB_SIZE-1);
			set_result_invalid(result);
		}
		// Possible outcomes:
		// a) invalid - no credit, no results
		// b) error - unexpected error
		// c) valid - result saved, segment updated, credit granted
		// d) redundant/unnown - result saved, segment not found, credit granted -> valid
		
	}
	if(f_write) {
		if(boinc_db.commit_transaction()) {
			cerr<<"Can't commit transaction!"<<endl;
			exit(1);
		}
	}
	boinc_db.close();
	return 0;
}

