/* generate segments into database */
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


int main(int argc, char** argv) {
	bool f_db; // write to database (orherwise write to stdout)
	long rule; // rule number
	long level;
	long gen_mod; // only generate matching modulus
	char *check1,*check2,*check3;
	if(argc!=5) {
			cerr<<"Expect 4 command line argument: f_db rule level mod"<<endl;
			exit(2);
	}
	f_db = (argv[1][0]=='y');
	rule = strtol(argv[2],&check1,10);
	gen_mod = strtol(argv[4],&check2,10);
	level = strtol(argv[3],&check3,10);
	if((argv[1][0]!='n' && !f_db) || *check1 || *check2) {
			cerr<<"Invalid argument format"<<endl;
			exit(2);
	}
	cerr<<"f_db="<<f_db<<" rule="<<rule<<" level="<<level<<" mod="<<gen_mod<<endl;
	//connect db if requested
	if(f_db) {
		initz();
		if(boinc_db.start_transaction())
			exit(4);
	}
	int db_minl= level; //TODO off-by-one?
	//do generate actually
	Generator gen1;
	NamerCHDLK10::init();
	kvadrat start1{0};
	gen1.init(rule-1,start1); //this will return false as lk is invalid, but would still initialize
	gen1.max_l=level;
	unsigned long long cnt1=0, cnt2=0;
	while(gen1.next()) {
		cnt1++;
		if(cnt1%gen_mod)
			continue;
		Generator gen2;
		gen2.init(rule-1,gen1.dlk); // this also returns false as dlk is incomplete
		gen2.min_l=db_minl;
		if(gen2.next()) {
			NamerCHDLK10::NameStr n58;
			if(!NamerCHDLK10::getName58(gen2.dlk,n58))
				continue; //TODO
			if(f_db) {
				std::stringstream qr{};
				qr<<"insert into tot_segment (rule,ix,start,next,minl) VALUES (";
				qr<<rule <<","<<cnt1 <<",";
				qr.write(n58.data(),n58.size()) <<",";
				qr.write(n58.data(),n58.size()) <<",";
				qr <<db_minl <<");";
				int retval = boinc_db.do_query(qr.str().c_str());
				if(retval) 
					continue;
			} else {
				write_square(cout,gen2.dlk,CSquareReader::NAME58);
			}
			cnt2++;
		}
	}
	cerr <<"cnt1="<<cnt1<<" cnt2="<<cnt2<<endl;
	if((cnt1/gen_mod)!=cnt2) {
		cerr<<"Error: generator failed"<<endl;
		exit(3);
	}
	if(f_db) {
		if(!boinc_db.commit_transaction()) {
			cerr<<"Can't commit transaction!";
			exit(1);
		}
		boinc_db.close();
	}
	return 0;
}
