#include <iostream>
#include <fstream>
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

#include "odlkcommon/namechdlk10.cpp"
#include "odlkcommon/kvio.cpp"

using std::vector, std::array, std::cerr, std::endl;

#include "wio.cpp"
#include "gener.cpp"

struct EFileNotFound	: std::exception { const char * what () const noexcept {return "File Not Found";} };

void writeAtomFile(const char* fn, const CStream& buf, bool resolv=false) {
	if(resolv) {
		std::string fn2;
		if(boinc_resolve_filename_s(fn,fn2))
			throw std::runtime_error("boinc_resolve_filename_s");
		writeAtomFile(fn2.c_str(),buf,false);
	} else {
#ifdef WIN32
		FILE* f = boinc_fopen(fn, "wb");
		if(!f)
			throw std::runtime_error("fopen");
		if( fwrite(buf.getbase(), 1, buf.pos(), f) !=buf.pos()) {
			fclose(f);
			throw std::runtime_error("fwrite");
		}
		fclose(f);
#else
		FILE* f = boinc_fopen("tmp", "wb");
		if(!f)
			throw std::runtime_error("fopen");
		if( fwrite(buf.getbase(), 1, buf.pos(), f) !=buf.pos()) {
			fclose(f);
			throw std::runtime_error("fwrite");
		}
		fclose(f);
		int retval=rename("tmp",fn);
		int tmp=errno;
		if( retval <0)
			throw std::runtime_error("rename");
#endif
	}
}

void readFile(const char* fn, CDynamicStream& buf, bool resolv=false) {
	if(resolv) {
		std::string fn2;
		int retval= boinc_resolve_filename_s(fn,fn2);
		if(retval) throw EFileNotFound();
		readFile(fn2.c_str(),buf,false);
	} else {
		FILE* f = boinc_fopen(fn, "rb");
		if(!f) {
			//bug: boinc on windows is stupid and this call does not set errno if file does not exist
			//Go to hell!!
			if(errno==ENOENT) throw EFileNotFound();
			if(!boinc_file_exists(fn)) throw EFileNotFound();
			throw std::runtime_error("fopen");
		}
		struct stat stat_buf;
		if(fstat(fileno(f), &stat_buf)<0) {
			fclose(f);
			throw std::runtime_error("fstat");
		}
		buf.setpos(0);
		buf.reserve(stat_buf.st_size);
		if( fread(buf.getbase(), 1, stat_buf.st_size, f) !=stat_buf.st_size) {
			fclose(f);
			throw std::runtime_error("fread");
		}
		buf.setpos(0);
		fclose(f);
	}
}

void message(const std::string& str, int err, bool notice) {
	char buf[256];
	if(err<0) {
		fprintf(stderr,
					"%s %s\n",
					boinc_msg_prefix(buf, sizeof(buf)), str.c_str()
			);
	} else {
		fprintf(stderr,
					"%s %s\n",
					boinc_msg_prefix(buf, sizeof(buf)), str.c_str()
			);
		boinc_finish_message(err, str.c_str(),notice);
		exit(err);
	}
}

State state;
Generator generator;

void init() {
	BOINC_OPTIONS opts;
	APP_INIT_DATA binitdata;
	boinc_options_defaults(opts);
	opts.direct_process_action = 0; // I handle abort/suspend myself
	int retval = boinc_init_options(&opts);
	if(retval) 
			message("boinc_init_options failed",206/*EXIT_INIT_FAILURE*/,0);
	boinc_get_init_data(binitdata);
	NamerCHDLK10::init();
	try {
		CDynamicStream buf;
		readFile("checkpoint",buf,false);
		state.readState(buf);
		state.interval_rsm++;
	} catch(EFileNotFound& e) {
		CDynamicStream buf;
		readFile("input.dat",buf,true);
		state.readInput(buf);
		state.userid= binitdata.userid;
		message("Start from Input",-1,0);
	}
	retval= generator.init(state.rule-1, state.next);
	if(!retval)
		message("Failed to initialize generator",2,0);
	generator.min_l= state.min_level;
}

void checkpoint() {
	CDynamicStream s;
	state.next= generator.dlk; //important FIXME off by one
	state.writeState(s);
	writeAtomFile("checkpoint",s);
}

void result_upload() {
	CDynamicStream s;
	state.next= generator.dlk; //important FIXME off by one
	state.writeState(s);
	writeAtomFile("output.dat",s,true);
}

void report_cpu_ops() {
	return; // do nothing
}

void maybe_checkpoint(){
	if( boinc_time_to_checkpoint() || boinc_status.suspended
		|| boinc_status.quit_request || boinc_status.no_heartbeat
		|| boinc_status.abort_request
		//|| (state.nkf>=state.lim_kf)
		)
	{
		bool upload1= state.nsn>=state.lim_sn || state.nkf>=state.lim_kf;
		if(( ( upload1 && !boinc_status.no_heartbeat)
			|| boinc_status.abort_request
			) && 1)
		{
			result_upload();
			message("Final checkpoint",0,false);
		} else {
			checkpoint();
			message("Checkpoint",-1,false);
		}
		report_cpu_ops();
		boinc_checkpoint_completed();
		if(boinc_status.quit_request) {
			message("Exiting as Requested",-1,false);
			exit(0);
		}
		if(boinc_status.no_heartbeat) {
			message("No Heartbeat!",-1,false);
			exit(204 /*EXIT_UNKNOWN*/);
		}
		if(boinc_status.abort_request) {
			/*std::string s ("output.dat"); boinc_upload_file(s);*/
			message("EXIT_ABORTED_BY_CLIENT",0,false);
		}
		if(boinc_status.suspended) {
			while(boinc_status.suspended) sleep(10);
		}
	}
	//report fraction done?
}

// sn^1 + kf^1 + cnt_trans^2 + daugh^1
// max_sn (iteration of the sndlk generator + kf check
// max_kf (search_trans dlx invocations)
// max_??
//credit sn(next+is), kf(trans+symm), dau(mar)
//next+kf (79985sn) 0.151157s
// +search_trans (79985kf) 248.13s
// +search_symm   1553.9s
// mar 647kf w11.877s 181678dk i20.93s =9.053s/181678

void check_kf(const kvadrat& sn) {
	Trans_DLx trans_dlx;
	unsigned long l_count=0;
	//find_trans
	trans_dlx.search_trans(sn);
	state.max_trans= std::max( (signed long) trans_dlx.cnt_trans, state.max_trans );
	state.ntrans+= trans_dlx.cnt_trans;
	//find_simm
	trans_dlx.search_symm_trans(sn); // this is slow (cnt_trans^2)
	//find_d_trans & is_mar
	for(int i = 0; i < Trans_DLx::ch_srez; i++){
		l_count += trans_dlx.kf_trans[i].size();
		for(auto q = trans_dlx.kf_trans[i].begin(); q != trans_dlx.kf_trans[i].end(); q++){
			trans_dlx.find_d_trans(q->second, trans_dlx.trans[i]);
			if(trans_dlx.is_mar()){
				NamerCHDLK10::Name nm;
				NamerCHDLK10::NameBin nb;
				if(NamerCHDLK10::getName(q->first,nm)) {
					NamerCHDLK10::encodeNameBin(nm,nb);
					state.odlk.push_back(nb);
				} else throw std::runtime_error("Failed to get sndlk name for odlk");
			}
		}
	}
	state.ndaugh+=l_count;
	return;
}

void work() {
	//clock_t t1 = clock();
	while(1) {
		state.nsn++;
		if(generator.is_kf()) {
			state.nkf++;
			state.last_kf = generator.dlk; //optimize copy
			check_kf(generator.dlk);
		}
		if(!generator.next())
			break;
		else maybe_checkpoint();
	}
	//cout << "took: " << double(clock() - t1) / CLOCKS_PER_SEC << " s\n";
	//cout << "sn: "<<state.nsn<<" kf: "<<state.nkf<<" dk: "<<state.ndaugh<<endl;
	state.ended= true;
	state.next= state.start; // this is not necessary
	//if state.ended -> checkpoint, upload, set correct flops, exit
	result_upload();
	report_cpu_ops();
	message("End of Segment",0,false);
}

int main(void) {
	try {
		init();
		work();
	}
	catch( const std::exception& e ) {
		message("Program failed due to exception. "+std::string(e.what()),1,false);
	}
}
