/* Assimilator: Process finished tasks and update segments. */
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <bitset>
#include <ctime>
#include <unistd.h>
#include <cstdlib>
#include <sys/stat.h>

#include "odlkcommon/common.h"
#include "odlkcommon/namechdlk10.h"
#include "family_mar/prov_blk_trans.h"

#include "boinc_api.h"
#include "Stream.cpp"

#include "config.h"
#include "backend_lib.h"
#include "error_numbers.h"
#include "sched_config.h"
#include "sched_util.h"
#include "validate_util.h"
#include "credit.h"

#include "odlkcommon/namechdlk10.cpp"
#include "odlkcommon/kvio.cpp"

#include "wio.cpp"
#include "gener.cpp"

/* validate?
 * yes, find all that need to be validated
 * fully process it
	 * read output file
	 * find the tot_segment
	 * create tot_result, tot_odlk, tot_result_odlk
 * then mark the result as assimilated and assign credit,
 * then mark the workunit as assimilated
 * do not forget to update tot_segment
 * 
 * should also generate new workunits? good idea, but no - leave it to genwu
*/

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

void readFile(const std::string& fn, CDynamicStream& buf) {
		FILE* f = boinc_fopen(fn.c_str(), "r");
		if(!f) {
			if(errno==ENOENT) throw runtime_error("file not found");
			throw std::runtime_error("fopen");
		}
		struct stat stat_buf;
		if(fstat(fileno(f), &stat_buf)<0)
			throw std::runtime_error("fstat");
		buf.setpos(0);
		buf.reserve(stat_buf.st_size);
		if( fread(buf.getbase(), 1, stat_buf.st_size, f) !=stat_buf.st_size)
			throw std::runtime_error("fread");
		buf.setpos(0);
		fclose(f);
}

class DB_SEGMENT : public DB_BASE {
public:
	//needed: rule, minl, next
	DB_ID_TYPE id;
	int rule;
	int minl;
	NamerCHDLK10::NameStr next;
public:
    DB_SEGMENT(DB_CONN* p=0) : DB_BASE("tot_segment", p?p:&boinc_db) {}
    void db_parse(MYSQL_ROW &r) {
			id = atoi(r[0]);
			rule = atoi(r[1]);
			minl = atoi(r[4]);
			std::copy(r[5],r[5]+next.size(),next.begin());
		}
};

struct EDatabase	: std::runtime_error { using runtime_error::runtime_error; };
struct EInvalid	: std::runtime_error { using runtime_error::runtime_error; };
static int retval;

void validate_result_output(State& rstate) {
	return; //todo
}

const float credit_m=0.007975* 1e-6;
const float
	credit_sn=1.8898,
	credit_kf=1942.5503,
	//credit_trans=0, todo
	credit_daugh=49.8299;

void process_result(DB_RESULT& result) {
	// Read the result file
	CDynamicStream buf;
	retval=read_output_file(result,buf);
	if(retval) throw EDatabase("can't read the output file");
	// TODO: if file missing -> EInvalid("Output file absent")
	State rstate;
	try {
		rstate.readState(buf);
	} catch (EStreamOutOufBounds& e){ throw EInvalid("can't deserialize output file"); }
	// get the segment
	NamerCHDLK10::NameStr sn_first, sn_last_kf, sn_next;
	NamerCHDLK10::getName58(rstate.start,sn_first);
	NamerCHDLK10::getName58(rstate.last_kf,sn_last_kf);
	NamerCHDLK10::getName58(rstate.next,sn_next);
	std::stringstream qr; qr<<"where cur_wu="<<result.workunitid<<" limit 1";
	DB_SEGMENT segment;
	retval = segment.lookup(qr.str().c_str());
	bool have_segment = (retval==0);
	if(retval && retval!=ERR_DB_NOT_FOUND) throw EDatabase("DB error reading segment");
	if(have_segment) {
		// copare segment with rstate
		if(rstate.rule!=segment.rule || rstate.min_level!=segment.minl) throw EInvalid("Result config does not match segment");
		if(sn_first!=segment.next) throw EInvalid("Result config.start does not match segment");
	} else {
		strncat(result.stderr_out,"Validator: segment not found, but validating anyway\n",BLOB_SIZE);
	}
	validate_result_output(rstate);
	// create tot_result
	qr=std::stringstream(); qr<<"insert tot_result SET ";
	if(have_segment) {
		qr<<"segment="<<segment.id<<", ";
	}
	qr<<"minl="<<rstate.min_level<<", skip="<<rstate.skip<<", ended="<<rstate.ended<<", userid="<<result.userid<<", ";
	qr<<"n_sn="<<rstate.nsn<<", n_kf="<<rstate.nkf<<", n_daugh="<<rstate.ndaugh<<", boinc_result="<<result.id<<", ";
	qr<<"n_kf_skip_below="<<rstate.nkf_skip_below<<", n_kf_skip_rule="<<rstate.nkf_skip_rule<<", ";
	qr<<"max_trans="<<rstate.max_trans<<", n_trans="<<rstate.ntrans<<", resume_cnt="<<rstate.interval_rsm;
	qr<<", first='"; qr.write(sn_first.data(),sn_first.size());
	qr<<"', next='"; qr.write(sn_next.data(),sn_next.size());
	if(rstate.nkf) {
		qr<<"', last_kf='"; qr.write(sn_last_kf.data(),sn_last_kf.size());
	}
	qr<<"';";
	retval=boinc_db.do_query(qr.str().c_str());
	if(retval) throw EDatabase("result row insert failed");
	DB_ID_TYPE result_id = boinc_db.insert_id();
	// insert into tot_odlk (odlk) values('...');
	// insert into tot_result_odlk (segment,result,odlk) values(segment.id,result_id,LAST_INSERT_ID());
	for( const NamerCHDLK10::NameBin& odlk_bin : rstate.odlk) {
		NamerCHDLK10::Name odlk_nm;
		NamerCHDLK10::NameStr odlk_n58;
		NamerCHDLK10::decodeNameBin(odlk_bin, odlk_nm);
		NamerCHDLK10::encodeName58(odlk_nm, odlk_n58);
		qr=std::stringstream();
		qr<<"insert into tot_odlk set odlk='";
		qr.write(odlk_n58.data(),odlk_n58.size());
		qr<<"' on duplicate key update id=LAST_INSERT_ID(id)";
		retval=boinc_db.do_query(qr.str().c_str());
		if(retval) throw EDatabase("odlk row insert failed");
		DB_ID_TYPE odlk_id = boinc_db.insert_id();
		if(have_segment) {
			qr=std::stringstream();
			qr<<"insert into tot_result_odlk SET segment="<<segment.id<<", ";
			qr<<"result="<<result_id<<", odlk="<<odlk_id;
			retval=boinc_db.do_query(qr.str().c_str());
			if(retval) throw EDatabase("odlk row insert failed");
		}
	}
	// update tot_segment set next=rstate.next where id=segment.id
	if(have_segment) {
		qr=std::stringstream();
		qr<<"update tot_segment set next='";
		qr.write(sn_next.data(),sn_next.size());
		qr<<"' where id="<<segment.id<<";";
		retval=boinc_db.do_query(qr.str().c_str());
		if(retval) throw EDatabase("tot_segment row update failed");
	}
	//TODO
	float credit = credit_m*( rstate.nsn*credit_sn + rstate.nkf*credit_kf + rstate.ndaugh*credit_daugh );
	DB_HOST host;
	DB_WORKUNIT wu;
	qr=std::stringstream();
	if(host.lookup_id(result.hostid)) throw EDatabase("Host not found");
	if(wu.lookup_id(result.workunitid)) throw EDatabase("Workunit not found");
	//is_valid
	double turnaround = result.received_time - result.sent_time;
	compute_avg_turnaround(host, turnaround);
	//-hav.max_jobs_per_day++;
	//-hav.consecutive_valid++; not for unreplicated
	//grant_credit
	result.granted_credit = credit;
	result.validate_state=VALIDATE_STATE_VALID;
	result.file_delete_state=FILE_DELETE_READY;
	grant_credit(host, result.sent_time, result.granted_credit);
	if(host.update()) throw EDatabase("Host update error");
	//update result (?)
	if(result.update()) throw EDatabase("Result update error");
	//update wu
	wu.assimilate_state = ASSIMILATE_DONE;
	//wu.file_delete_state=FILE_DELETE_READY;
	wu.need_validate = 0;
	wu.transition_time = time(0);
	//todo: unsent -> RESULT_OUTCOME_DIDNT_NEED
	if(have_segment) {
		wu.canonical_resultid = result.id;
		wu.canonical_credit = result.granted_credit;
	} else {
		//wu.assimilate_state = ASSIMILATE_DONE;
		// what to do? abort it?
	}
	if(wu.update()) throw EDatabase("Workunit update error");
	cout<<" have_segment "<<have_segment<< " credit="<<result.granted_credit<<endl;
}

void set_result_invalid(DB_RESULT& result) {
	DB_WORKUNIT wu;
	if(wu.lookup_id(result.workunitid)) throw EDatabase("Workunit not found");
	//hav - do not care
	result.validate_state=VALIDATE_STATE_INVALID;
	//result.file_delete_state=FILE_DELETE_READY; - keep for analysis
	if(result.update()) throw EDatabase("Result update error");
	if(wu.update()) throw EDatabase("Workunit update error");
}

int main(int argc, char** argv) {
	bool f_write;
	long gen_limit;
	int batchno;
	char *check1;
	DB_APP app;
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
	if (app.lookup("where name='tot5'")) {
		cerr<<"can't find app tot5\n";
		exit(4);
	}
	if(boinc_db.start_transaction())
		exit(4);
	//do generate actually
	NamerCHDLK10::init();
	std::stringstream enum_qr;
	//what is neede? the result
	//enumerate results
	//the updating is incredibly stupid, but we will see
	enum_qr<<"where appid="<<app.id<<" and server_state="<<RESULT_SERVER_STATE_OVER
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
		cout<<"result "<<result.name<<endl;
		try {
			process_result(result);
		} catch (EInvalid& e) {
			cout<<" Invalid: "<<e.what()<<endl;
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

