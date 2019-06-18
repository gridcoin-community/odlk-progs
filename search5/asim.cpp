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
	<<" and outcome="<<RESULT_OUTCOME_SUCCESS<<" limit "<<gen_limit<<";";
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
		// Read the result file
		CDynamicStream buf;
		retval=read_output_file(result,buf);
		if(retval) { log_messages.printf(MSG_CRITICAL,
            "[RESULT#%lu %s] can't read the output file %d\n",
            result.id, result.name, retval); exit(4);	}
		State rstate;
		try {
			rstate.readState(buf);
		} catch (EStreamOutOufBounds& e){ log_messages.printf(MSG_CRITICAL,
            "[RESULT#%lu %s] can't deserialize output file %s\n",
						// TODO: This means the result is invalid - mark and continue
            result.id, result.name,e.what()); exit(4);	}
		// get the segment
		NamerCHDLK10::NameStr sn_first, sn_last_kf, sn_next;
		NamerCHDLK10::getName58(rstate.start,sn_first);
		NamerCHDLK10::getName58(rstate.last_kf,sn_last_kf);
		NamerCHDLK10::getName58(rstate.next,sn_next);
		std::stringstream qr; qr<<"where cur_wu="<<result.workunitid<<" limit 1";
		DB_SEGMENT segment;
		retval = segment.lookup(qr.str().c_str());
		if(retval) { log_messages.printf(MSG_CRITICAL,
            "[RESULT#%lu %s] segment not found\n",
						 // TODO: still grant credit and proper error report
            result.id, result.name); exit(4);	}
		// TODO copare segment with rstate
		// create tot_result
		qr=std::stringstream(); qr<<"insert into tot_result "
		<<"(segment, minl, skip, ended, userid, n_sn, n_kf, n_daugh, n_kf_skip_below, n_kf_skip_rule, max_trans, n_trans, resume_cnt, boinc_result, "
		"first, next, last_kf)"
		<<" VALUES("
		<<segment.id<<","<<rstate.min_level<<","<<rstate.skip<<","<<rstate.ended<<","<<result.userid
		<<","<<rstate.nsn<<","<<rstate.nkf<<","<<rstate.ndaugh<<","<<rstate.nkf_skip_below<<","<<rstate.nkf_skip_rule
		<<","<<rstate.max_trans<<","<<rstate.ntrans<<","<<rstate.interval_rsm<<","<<result.id;
		qr<<",'"; qr.write(sn_first.data(),sn_first.size());
		qr<<"','"; qr.write(sn_next.data(),sn_next.size());
		qr<<"','"; qr.write(sn_last_kf.data(),sn_last_kf.size());
		qr<<"');";
		retval=boinc_db.do_query(qr.str().c_str());
		DB_ID_TYPE result_id = boinc_db.insert_id();
		// insert into tot_odlk (odlk) values('...');
		// insert into tot_result_odlk (segment,result,odlk) values(segment.id,result_id,LAST_INSERT_ID());
		for( const NamerCHDLK10::NameBin& odlk_bin : rstate.odlk) {
			NamerCHDLK10::Name odlk_n58;
			NamerCHDLK10::NameStr odlk_nm;
			NamerCHDLK10::decodeNameBin(odlk_bin, odlk_nm);
			NamerCHDLK10::encodeName58(odlk_name, odlk_n58);
		}
		qr=std::stringstream();
		qr<<"insert into tot_odlk (odlk) values('";
		qr.write(odlk_n58.data(),odlk_n58.size());
		qr<<"'); insert into tot_result_odlk (segment,result,odlk) values("
		<<segment.id<<","<<result_id
		<<",LAST_INSERT_ID());";
		//TODO
		
	}

	return 69;
	if(f_write) {
		if(boinc_db.commit_transaction()) {
			cerr<<"Can't commit transaction!"<<endl;
			exit(1);
		}
	}
	boinc_db.close();
	return 0;
}

