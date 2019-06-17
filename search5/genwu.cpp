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

#include "wio.cpp"
#include "gener.cpp"

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

class DB_GENITEM : public DB_BASE {
public:
	//needed: rule, minl, next
	DB_ID_TYPE id;
	int rule;
	int minl;
	NamerCHDLK10::NameStr next;
public:
    DB_GENITEM(DB_CONN* p=0) : DB_BASE("tot_segment", p?p:&boinc_db) {}
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
	std::stringstream qr;
	qr<<"where cur_wu is null and next is not null and enabled limit "<<gen_limit<<";";
	DB_GENITEM item;
	while(1) {
		int retval= item.enumerate(qr.str().c_str());
		if(retval) {
			if (retval != ERR_DB_NOT_FOUND) {
				cerr<<"db error"<<endl;
				exit(4);
			}
			break;
		}
		Input inp;
		CDynamicStream buf;
		inp.rule= item.rule;
		if(!NamerCHDLK10::fromName58(item.next,inp.start))
			exit(5); //todo
		inp.min_level= item.minl;
		inp.skip_below= 0;
		inp.skip_fast= 0;
		inp.skip_rule= {0};
		inp.lim_sn= 1905000000;
		inp.lim_kf= 112000;
		std::stringstream wuname;
		(wuname<<"tot5_"<<item.rule<<"a_").write(item.next.data(),item.next.size());
		cout<<"WU "<<wuname.str()<<endl;
		if(!f_write)
			continue;
		inp.writeInput(buf);
		std::stringstream fninp;
		fninp<<config.download_dir<<"/"<<wuname.str()<<".dat";
		std::ofstream fhinp(fninp.str(),ios::binary);
		fhinp.write((char*)buf.getbase(),buf.pos());
		fhinp.close(); if(!fhinp) {cerr<<"file error"<<endl;exit(6);}
		DB_WORKUNIT wu; wu.clear();
		wu.appid = app.id;
		strcpy(wu.name, wuname.str().c_str());
		wu.rsc_fpops_est = 14e12;  //TODO - 1 hour
		wu.rsc_fpops_bound = 1e16;
		wu.rsc_memory_bound = 1e8; //todo 100M
		wu.rsc_disk_bound = 1e8; //todo 100m
		wu.delay_bound = 43200; //todo 6h
		wu.target_nresults= wu.min_quorum = 1;
		wu.max_error_results= wu.max_total_results= 8;
		wu.max_success_results= 1;
		const char* infiles[1] = { fninp.str().c_str() };
		retval= create_work(wu, "templates/tot5_in","templates/tot5_out",0,infiles,1,config,0,0,0);
		if(retval) exit(6);
	}

	// TODOcommit
	return 1;
}
