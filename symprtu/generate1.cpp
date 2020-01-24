/* Generator */
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

#include "config.h"
#include "backend_lib.h"
#include "error_numbers.h"
#include "sched_config.h"
#include "sched_util.h"
#include "validate_util.h"
#include "credit.h"

#include "wio.cpp"

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
const char spt_template [] =
"<input_template><file_info><number>0</number></file_info><workunit><file_ref>"
"<file_number>0</file_number><open_name>input.dat</open_name></file_ref>"
"</workunit></input_template>\n";


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
	if (spt_app.lookup("where name='stpt'")) {
		std::cerr<<"can't find app stpt\n";
		exit(4);
	}
}

void post_batch_msg(int batch,uint64_t first,uint64_t next,unsigned long count, const char* label, const char * msg)
{
	//into message
	std::stringstream post;
	post<<"insert into post set thread= 3055, user= 1, timestamp= UNIX_TIMESTAMP(), modified= 0, parent_post= 0, score= 0, votes= 0, signature= 1, hidden= 0, content='";
	post<<"Batch "<<batch<<": "<<first<<" .. "<<next<<" -1\nCount: "<<count<<"\n";
	post<<msg;
	post<<"';";
	retval=boinc_db.do_query(post.str().c_str());
	if(retval) throw EDatabase("batch forum post insert failed");
	DB_ID_TYPE message_id = boinc_db.insert_id();
	std::stringstream qr;
	//into batch
	qr<<"insert into batch set id= "<<batch<<", short_descr= '"<<label<<"', forum_msg= "<<message_id<<";";
	retval=boinc_db.do_query(qr.str().c_str());
	if(retval) throw EDatabase("batch descr insert failed");
}

void submit_wu_in(uint64_t start, uint64_t end, int batch)
{
	std::stringstream wuname;
	DB_WORKUNIT wu; wu.clear();
	TInput inp;

		inp.start= start;
		inp.end= end;
		inp.mine_k= 16;
		inp.mino_k= 13;
		inp.max_k= 32;
		inp.upload = 0;
		inp.exit_early= 0;
		inp.out_last_primes= 1;
		inp.out_all_primes= 0;
		inp.twin_k=7;
		inp.twin_min_k=8;
		inp.twin_gap_k=6;
		inp.twin_gap_min=88;
		inp.twin_gap_kmin=496;
		inp.primes_in.clear();
		wu.appid = spt_app.id;
		//14e12 is one hour on mangan-pc
		wu.rsc_fpops_est = (inp.end - inp.start) * 15;
		wu.rsc_fpops_bound = wu.rsc_fpops_est * 24;
		wu.rsc_memory_bound = 399e6;
		wu.rsc_disk_bound = 1e8; //todo 100m
		wu.delay_bound = 5 * 24 * 3600;
		wu.priority = 10;
		wu.batch= batch;
		wu.target_nresults= wu.min_quorum = 1;
		wu.max_error_results= wu.max_total_results= 8;
		wu.max_success_results= 1;

	wuname<<"spt_"<<wu.batch<<"_"<<inp.start;
	std::cout<<" WU "<<wuname.str()<<" "<<inp.end<<endl;
	strcpy(wu.name, wuname.str().c_str());
	CFileStream buf;
	inp.writeInput(buf);
	std::stringstream fninp;
	fninp<<config.download_dir<<"/"<<wuname.str()<<".in";
	try{
		buf.writeFile(fninp.str().c_str());
	}
	catch(std::runtime_error& e) {
		throw EDatabase("Unable to write next input file");
	}
	vector<INFILE_DESC> infile_specs{1};
	infile_specs[0].is_remote = false;
	strcpy(infile_specs[0].name, (wuname.str()+".in").c_str());
	retval= create_work2(wu, spt_template,"templates/spt_out",0,infile_specs,config,0,0,0);
	if(retval) throw EDatabase("create_work2 failed");
}

int main(int argc, char** argv) {

	//node: min app version is set in app table
	initz();
	if(boinc_db.start_transaction())
		exit(4);

	uint64_t start= 545771407000000000;
	uint64_t   end= 600000000000000000;
	uint64_t  step=      1965000000000;
	unsigned maxcnt = 16000;
	int batch = 57;
	uint64_t next = start;
	unsigned long count = 0;
	while(1) {
		uint64_t curr = next;
		if(curr > end)
			break;
		if(count>=maxcnt)
			break;

		next = curr + step;
		submit_wu_in(curr, next, batch);
		count++;
	}
	post_batch_msg(batch,start,next,count,"stpt-onv","Continue with the new application");
	cerr<<"Count: "<<count<<endl;
	cerr<<"First: "<<start<<endl;
	cerr<<"Next : "<<next<<endl;

	if(boinc_db.commit_transaction()) {
		cerr<<"failed to commit transaction"<<endl;
		exit(1);
	}
	
	boinc_db.close();
	return 0;
}

