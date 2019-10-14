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


#include "boinc_api.h"
#include "Stream.cpp"

#include "config.h"
#include "backend_lib.h"
#include "error_numbers.h"
#include "sched_config.h"
#include "sched_util.h"
#include "validate_util.h"
#include "credit.h"


struct EDatabase	: std::runtime_error { using runtime_error::runtime_error; };
struct EInvalid	: std::runtime_error { using runtime_error::runtime_error; };
static int retval;

#include "wio.cpp"
#include "gener.cpp"
#include "create_wu.cpp"

DB_APP scpn_app;
string scpn_template;

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
	if (wu_gen_cfg.app.lookup("where name='tot5'")) {
		cerr<<"can't find app tot5\n";
		exit(4);
	}
	if (read_file_string(config.project_path("templates/tot5_in"), wu_gen_cfg.in_template)) {
			cerr<<"can't read input template templates/tot5_in\n";
			exit(4);
	}
}

void generate_workunit(void seg) {
	//check if segment is suitable for wu generation
	/* 0=disabled, 1=enabled, 2=finished, 3=wingman */
	if(seg.state!=1)
		return;
	//generate new wu in a segment
	Input inp;
	CDynamicStream buf;
	inp.start= seg.current;
	inp.end  = seg.end;
	inp.upload = 90*60;
	inp.exit_early= 1;
	inp.out_last_primes= 0;
	inp.out_all_primes= 1;
	inp.primes_in.clear();
	std::stringstream wuname;
	wuname<<"scpn_"<<inp.start<<"_"<<char(seg.state-1+'a');
	cout<<" WU "<<wuname.str()<<endl;
	inp.writeInput(buf);
	std::stringstream fninp;
	fninp<<config.download_dir<<"/"<<wuname.str()<<".in";
	std::ofstream fhinp(fninp.str(),ios::binary); // todo - noclobber
	fhinp.write((char*)buf.getbase(),buf.pos());
	fhinp.close(); if(!fhinp) {
		throw EDatabase("Unable to write next input file");
	}
	DB_WORKUNIT wu; wu.clear();
	wu.appid = cfg.app.id;
	wu.batch=20+cfg.batch;
	strcpy(wu.name, wuname.str().c_str());
	wu.rsc_fpops_est = 4e13; // 2x 1h
	wu.rsc_fpops_bound = 1e16;
	wu.rsc_memory_bound = 1e8; //todo 100M
	wu.rsc_disk_bound = 1e8; //todo 100m
	wu.delay_bound = 604800; // 7 days
	wu.priority = 1 + item.prio_adjust;
	wu.target_nresults= wu.min_quorum = 1;
	wu.max_error_results= wu.max_total_results= 8;
	wu.max_success_results= 1;
	
	vector<INFILE_DESC> infile_specs(1);
	infile_specs[0].is_remote = false;
	strcpy(infile_specs[0].name, (wuname.str()+".in").c_str());
	//maybe have the template constant inline here
	retval= create_work2(wu, cfg.in_template.c_str(),"templates/tot5_out",0,infile_specs,config,0,0,0);
	if(retval) throw EDatabase("create_work2 failed");
	//update the segment +writeback
	seg.wu_cur= wu.id;
	seg.update();
}

void process_result(DB_RESULT& result) {
	/* if finished, just update the segment
	 * if did not fininsh, but still have result, generate new wu
	 * othervise resend the wu
	 */
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

	...
	generate_workunit(segment);
}


void asim_results() {
	std::stringstream enum_qr;
	enum_qr<<"where appid="<<app.id<<" and server_state="<<RESULT_SERVER_STATE_OVER
	<<" and outcome="<<RESULT_OUTCOME_SUCCESS<<" and validate_state="<<VALIDATE_STATE_INIT<<" limit "<<select_limit<<";";
	DB_RESULT result;
	cerr<<"Assimilating results"<<endl;
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
		// d) redundant/unnown - ???
	}
}


int main(int argc, char** argv) {
	bool f_write;
	long select_limit;
	char *check1;
	DB_APP& app = wu_gen_cfg.app;
	if(argc!=3) {
			cerr<<"Expect 2 command line argument: f_write limit"<<endl;
			exit(2);
	}
	f_write = (argv[1][0]=='y');
	select_limit = strtol(argv[2],&check1,10);
	if((argv[1][0]!='n' && !f_write) || *check1) {
			cerr<<"Invalid argument format"<<endl;
			exit(2);
	}
	cerr<<"f_write="<<f_write<<" limit="<<gen_limit<<endl;
	//connect db if requested
	initz();
	if(boinc_db.start_transaction())
		exit(4);

	check(boinc_db.start_transaction(),"start_transaction");

	asim_results();

	if(f_write) {
		check(boinc_db.commit_transaction(),"commit_transaction");
		check(boinc_db.start_transaction(),"start_transaction");
	}

	init_segments();

	if(f_write)
		check(boinc_db.commit_transaction(),"commit_transaction");
	
	boinc_db.close();
	return 0;
}

