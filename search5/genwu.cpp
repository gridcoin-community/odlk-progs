/* Generate workunits from segments that are idle */
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

#include "odlkcommon/namechdlk10.cpp"
#include "odlkcommon/kvio.cpp"

struct EDatabase	: std::runtime_error { using runtime_error::runtime_error; };
struct EInvalid	: std::runtime_error { using runtime_error::runtime_error; };
static int retval;

#include "wio.cpp"
#include "gener.cpp"
#include "create_wu.cpp"

gen_padls_cfg wu_cfg;

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

int main(int argc, char** argv) {
	bool f_write;
	long gen_limit;
	int batchno;
	char *check1, *check2;
	DB_APP app;
	if(argc!=4) {
			cerr<<"Expect 3 command line argument: f_write batch limit"<<endl;
			exit(2);
	}
	f_write = (argv[1][0]=='y');
	batchno = strtol(argv[2],&check1,10);
	gen_limit = strtol(argv[3],&check2,10);
	if((argv[1][0]!='n' && !f_write) || *check1 || *check2) {
			cerr<<"Invalid argument format"<<endl;
			exit(2);
	}
	cerr<<"f_write="<<f_write<<" limit="<<gen_limit<<endl;
	//connect db if requested
	initz();
	if (wu_cfg.app.lookup("where name='tot5'")) {
		cerr<<"can't find app tot5\n";
		exit(4);
	}
	char* in_template;
	if (read_file_string(config.project_path("templates/tot5_in"), wu_cfg.in_template)) {
			cerr<<"can't read input template templates/tot5_in\n";
			exit(4);
	}
	wu_cfg.batch=batchno;
	if(boinc_db.start_transaction())
		exit(4);
	//do generate actually
	NamerCHDLK10::init();
	std::stringstream qr;
	qr<<"where cur_wu is null and next is not null and enabled limit "<<gen_limit<<";";
	DB_SEGMENT item;
	while(1) {
		int retval= item.enumerate(qr.str().c_str());
		if(retval) {
			if (retval != ERR_DB_NOT_FOUND) {
				cerr<<"db error"<<endl;
				exit(4);
			}
			break;
		}
		gen_padls_wu(item,wu_cfg);
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
