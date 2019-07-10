#include "prov_blk_trans.h"
#include <cassert>
#include <sstream>
#include <algorithm>
#include <climits>
#include <sys/stat.h>
#ifdef USE_THREADS
#include <thread>
#include <mutex>
#endif
#include "inih/INIReader.h"
#include "odlkcommon/namechdlk10.h"

/*
	Load config.
		threads
		formats
		checkpoint
	Open input and output.
	Load checkpoint if exists.
		offset, input mtime
	if output exists and checkpoint does not -> error
	if checkpoint mtime!=input mtime -> error
	Start enough threads.
		lock, read input batch, unlock
		process
		lock, checkpoint, write result, unlock
	If ctrl-c, set exit flag.
	Wait all threads.
*/

const char* config;
mutex cs_main;

unsigned num_threads;

clock_t t3 = clock(), t3_begin, t3_end;
static const int timeout2 = (double)7680 / CLOCKS_PER_SEC;


CSquareReader::format_t GetFormatConst(const string str, CSquareReader::format_t dflt) {
	if(str=="text") return CSquareReader::TEXT;
	if(str=="txt") return CSquareReader::TEXT;
	if(str=="n58") return CSquareReader::NAME58;
	if(str=="bin") return CSquareReader::BINARY;
	return dflt;
}

unsigned cfgMaxThreads;
bool cfgVerbose;
bool cfgContinueAnyway;
string cfgInputName;
string cfgOutputName;
CSquareReader::format_t cfgInputFormat;
CSquareReader::format_t cfgOutputFormat;
long cfgCheckpointPeriod;
long chkptOffset;
struct stat input_stat;

CSquareReader input_reader;
long input_pos;
ofstream output_file;
vector<long> checkpoints;
bool exit_flag;

int init(){
  NamerCHDLK10::init();
	config = "family_mar.conf";
	exit_flag= false;

  {
		INIReader ini(config);
		if(ini.ParseError()) {
			cerr<<"Config parse error "<<ini.ParseError()<<endl;
			exit(1);
		}
		cfgMaxThreads= ini.GetInteger("","maxthreads",64);
		cfgVerbose= ini.GetBoolean("","verbose",true);
		cfgContinueAnyway= ini.GetBoolean("","continueanyway",false);
		cfgInputName= ini.Get("","inputname","input.txt");
		cfgOutputName= ini.Get("","outputname","output.txt");
		cfgInputFormat= GetFormatConst(ini.Get("","inputformat","text"),CSquareReader::TEXT);
		cfgOutputFormat= GetFormatConst(ini.Get("","outputformat","text"),CSquareReader::TEXT);
		cfgCheckpointPeriod= ini.GetReal("","checkpointmin",1)*60;
	}

	string chkptProtection;
  {
		string checkpointfn = "checkpoint.txt";
		INIReader ini(checkpointfn);
		if(ini.ParseError()<0 && errno==ENOENT) {
			chkptProtection="";
			chkptOffset=0;
			//no checkpoint, start from beginning
		} else {
			if(ini.ParseError()) {
				cerr<<"Checkpoint parse error "<<ini.ParseError()<<endl;
				exit(1);
			}
			chkptProtection= ini.GetInteger("checkpoint","protection",0);
			chkptOffset= ini.GetInteger("checkpoint","offset",0);
		}
	}

	if(chkptOffset!=0) {
		struct stat out_stat;
		if(stat(cfgInputName.c_str(), &input_stat)<0) {
			cerr<<"Input open error "<<endl;
			exit(1);
		}
		if(stat(cfgOutputName.c_str(), &out_stat)<0) {
			cerr<<"Output open error "<<endl;
			exit(1);
		}
		ostringstream str;
		str<<cfgInputName<<":"<<input_stat.st_size<<":"<<input_stat.st_mtime<<":"<<cfgOutputName<<":"<<int(cfgOutputFormat)<<":"<<out_stat.st_size;
		if(str.str()!=chkptProtection) {
			cerr<<"Checkpoint possibly corrupted or input file changed or output file changed."<<endl;
			cerr<<"Remove checkpoint to start from beginning"<<endl;
			exit(1);
		}
		//for(chkptOffset
	}

  input_reader.open(cfgInputName, cfgInputFormat);
  output_file = ofstream(cfgOutputName, ios::binary);
  if(!output_file) {
		cerr<<"Output open error "<<endl;
		exit(1);
	}

	//TODO for(int i = 0; i < ch_srez; i++) trans[i].reserve(max_trans); may increase speed
	return true;
}

inline void process_batch(Trans_DLx& trans_dlx, const vector<kvadrat>& batch, vector<kvadrat>& odlks, long long& l_count)
{
	for(const kvadrat& lk : batch) {
		trans_dlx.search_trans(lk);
		if(trans_dlx.cnt_trans <= 1) continue;
		trans_dlx.search_symm_trans(lk);
		for(int i = 0; i < Trans_DLx::ch_srez; i++) {
			l_count += trans_dlx.kf_trans[i].size();
			for(auto q = trans_dlx.kf_trans[i].begin(); q != trans_dlx.kf_trans[i].end(); q++) {
				trans_dlx.find_d_trans(q->second, trans_dlx.trans[i]);
				if(trans_dlx.is_mar()) {
					odlks.push_back(q->first);
				}
			}
		}
	}
}

void write_checkpoint(const vector<kvadrat>& odlks) {
	//append odls to output file
	for(const kvadrat& odlk : odlks) {
		write_square(output_file, odlk, cfgOutputFormat);
	}
	output_file.flush();
	if(!output_file) {
		cerr<<"Output write failed"<<endl;
		exit(2);
	}
	unsigned min_offset=UINT_MAX;
	for(unsigned i=0; i<num_threads; ++i) {
		if(checkpoints[i]<min_offset) min_offset=checkpoints[i];
	}
	ofstream chf("checkpoint.txt", ios::binary);
	chf<<"[checkpoint]\nprotection=";
	chf<<cfgInputName<<":"<<input_stat.st_size<<":"<<input_stat.st_mtime<<":"<<cfgOutputName<<":"<<int(cfgOutputFormat)<<":"<<output_file.tellp();
	chf<<"\noffset="<<min_offset<<"\n";
	chf.close();
	if(!chf) {
		cerr<<"Checkpoint write failed"<<endl;
		exit(2);
	}
}

void read_input_batch(vector<kvadrat>& batch, unsigned pos) {
	while( batch.size() < 100 ) {
		kvadrat kv;
		if input_reader.read(kv);
		input_ofs++;
		batch.push_back(kv);
	}
	end = input_ofs;
	if(batch.size()==0)
}

void thread_main(int index)
{
	unsigned long long my_end_point = 0;
	vector<kvadrat> batch;
	vector<kvadrat> odlks;
	Trans_DLx trans_dlx{};
	long long l_count{0};
	while(1) {
		cs_main.lock();
		checkpoints[index]= my_end_point;
		write_checkpoint(odlks);
		odlks.clear();
		if(exit_flag) { cs_main.unlock(); return; }
		read_input_batch(batch, my_end_point);
		if(exit_flag) { cs_main.unlock(); return; }
		cs_main.unlock();
		process_batch(trans_dlx, batch, odlks, l_count);
	}
}



const char help_text[] =
"Search for Fancy by Family Diagonal Latin Squares (not symmetric).\n"
"family_mar.exe\n"
;

int main(int argc, char* argv[]){
	setlocale(LC_CTYPE, "rus");
	cerr << "Find Orthogonal DLS (except symmetrical) for family LS" << endl;

	if(argc!=1){
		cerr << "Expected 0 arguments" << endl << help_text;
		return 2;
	}

	if(init()){
		clock_t t0 = clock();
		time_t rt0 = time(0);
		clock_t t1;
		time_t rt1;

		num_threads=std::max(1U, std::min(std::thread::hardware_concurrency(), cfgMaxThreads));

		cerr << "num_threads: "<<num_threads<<endl;

		vector<std::thread> my_threads {num_threads};
		for(unsigned thi = 1; thi < num_threads; ++thi)
		{
			my_threads[thi-1] = std::thread( &thread_main, thi );
		}
		thread_main(0);

		for( auto& th : my_threads)
			th.join();

		cerr << endl;

		//cerr << "Checked DLK: " << count_dlk << endl;
#ifdef _WIN32
#ifdef USE_THREADS
if( nb_threads > 1 ){
		////////////////////////////////////////////////////////////////////////////////////////////////
		// This for compile in windows (mingw64, ver 5, ver 8 tested) and thread more 1
		// Problem place for correct count time work in windows system
		cerr << "Run Time (s): " << double(clock() - t0) / CLOCKS_PER_SEC * (double)nb_threads << endl;
		// original code:
		// cerr << "Run Time (s): " << double(clock() - t0) / CLOCKS_PER_SEC << endl;
		// allways have same time for "Run Time (s)" and "Real Run Time (s)"
		////////////////////////////////////////////////////////////////////////////////////////////////
}else{
		//This for windows and tread == 1
		cerr << "Run Time (s): " << double(clock() - t0) / CLOCKS_PER_SEC << endl;
}
#else
		// all other variants
		cerr << "Run Time (s): " << double(clock() - t0) / CLOCKS_PER_SEC << endl;
#endif
#else
		// This correct work on unix system
		cerr << "Run Time (s): " << double(clock() - t0) / CLOCKS_PER_SEC << endl;
#endif
		cerr << "Real Run Time (s): " << (time(0) - rt0) << endl;
		//cerr << "Found Fancy DLS: " << baza_mar.size() << endl;


		return 0;
	} else
		return 1;
}
