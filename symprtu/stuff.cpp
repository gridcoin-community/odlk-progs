#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <ctime>
#include <sys/stat.h>

#include "primesieve.hpp"
#include "primesieve/PrimeGenerator.hpp"

#include "boinc_api.h"
#include "Stream.cpp"

using std::vector;
using std::array;
using std::cerr;
using std::endl;
using std::string;

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

class CBoincFileStream
	: public CDynamicStream
{
	public:
	using CDynamicStream::CDynamicStream;
	CBoincFileStream( int mode, const char* name )
	{
		std::string fn2;
		if(mode&2) {
			int retval= boinc_resolve_filename_s(name,fn2);
			if(retval) throw EFileNotFound();
		}
		else fn2 = name;
		FILE* f = boinc_fopen(fn2.c_str(), "r");
		if(!f) {
			//bug: boinc on windows is stupid and this call does not set errno if file does not exist
			//Go to hell!!
			if(errno==ENOENT) throw EFileNotFound();
			if(!boinc_file_exists(fn2.c_str())) throw EFileNotFound();
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
	void writeFile(  int mode, const char* name )
	{
		std::string fn2;
		if(mode&2) {
			if(boinc_resolve_filename_s(name,fn2))
				throw std::runtime_error("boinc_resolve_filename_s");
		}
		else fn2=name;
		FILE* f = boinc_fopen("tmp", "w");
		if(!f)
			throw std::runtime_error("fopen");
		if( fwrite(this->getbase(), 1, this->pos(), f) !=this->pos())
			throw std::runtime_error("fwrite");
		fclose(f);
		if( rename("tmp",fn2.c_str()) <0)
			throw std::runtime_error("rename");
	}
};

TInput input;
TOutput output;
bool flag;

void read_input(const char* fn) {
	try {
		input.readInput(CBoincFileStream(0,fn));
	}
	catch( const std::exception& e ) {
		cerr<<e.what()<<endl;
	}
}

void read_output(const char* fn) {
	try {
		output.readOutput(CBoincFileStream(0,fn));
	}
	catch( const std::exception& e ) {
		cerr<<e.what()<<endl;
	}
	auto& cout = std::cout;
	cout<<output.start<<".."<<output.chkpt<<endl;
	//uint64_t last;  // last prime
	cout<<"nprime: "<<output.nprime<<endl;
	cout<<"primes:";
	for(const auto& p : output.primes)
		cout<<" "<<p;
	cout<<endl<<"tuples:"<<endl;
	for(const auto& tuple : output.tuples) {
		cout<<"T "<<tuple.start<<"("<<tuple.k<<"):";
		for(const auto& d : tuple.ofs)
			cout<<" "<<d;
		cout<<endl;
	}
	#if 1
	cout<<"twins:"<<endl;
	for(const auto& tuple : output.twins) {
		cout<<"W "<<tuple.start<<"("<<(tuple.ofs.size()+1)<<"):";
		for(const auto& d : tuple.ofs)
			cout<<" "<<d;
		cout<<endl;
	}
	#else
	cout<<"twin_gap_seq:"<<endl;
	long maxgap = 0;
	for(const auto& tuple : output.twins) {
		long gap=0;
		for(const auto& d : tuple.ofs)
			if (d>gap)
				gap= d;
		if(gap>maxgap) {
			cout<<"W "<<tuple.start<<"("<<(tuple.ofs.size()+1)<<"):";
			for(const auto& d : tuple.ofs)
				cout<<" "<<d;
			cout<<" ["<<(gap+2)<<"]"<<endl;
			maxgap= gap;
		}
	}
	#endif
	cout<<"twin_tuples:"<<endl;
	for(const auto& tuple : output.twin_tuples) {
		cout<<"U "<<tuple.start<<"("<<tuple.k<<"):";
		for(const auto& d : tuple.ofs)
			cout<<" "<<d;
		cout<<endl;
	}
	cout<<"twin_gap_6d: "<<output.twin_gap_6d<<endl;
	cout<<"twin_gap_d: "<<output.twin_gap_d<<endl;
	cout<<"twin_gap:"<<endl;
	for(const auto& tuple : output.twin_gap) {
		cout<<"G "<<tuple.start<<"("<<(tuple.ofs.size()+1)<<"):";
		for(const auto& d : tuple.ofs)
			cout<<" "<<d;
		cout<<endl;
	}
	cout<<"status: "<<int(output.status)<<endl;
	cout<<"sieve_init_ms"<<(output.sieve_init_cs*10)<<endl;
}

void write_input(const char* fn) {
	try {
		CBoincFileStream fs;
		input.writeInput(fs);
		fs.writeFile(1,fn);
	}
	catch( const std::exception& e ) {
		cerr<<e.what()<<endl;
	}
}
void write_output(const char* fn) {
	try {
		CBoincFileStream fs;
		output.writeOutput(fs);
		fs.writeFile(1,fn);
	}
	catch( const std::exception& e ) {
		cerr<<e.what()<<endl;
	}
}

void mksample() {
	auto& inp = input;
	inp.start=    530051400000000000;
	inp.end= inp.start  +65500000000;
	inp.mine_k= 16;
	inp.mino_k= 13;
	inp.max_k= 32;
	inp.upload = 0;
	inp.exit_early= 0;
	inp.out_last_primes= 1;
	inp.out_all_primes= 0;
	inp.primes_in.clear();
	inp.twin_k=6;
	inp.twin_min_k=8;
	inp.twin_gap_k=6;
	inp.twin_gap_min=88;
	inp.twin_gap_kmin=496;
	write_input("sample.dat");
}

int main(int argc, char** argv){
	if(argc==2) {
		if(string(argv[1])=="-s") {
			mksample();
		} else {
			read_output(argv[1]);
		}
		return 0;
	} else while(1) {
		cerr<<"use debugger"<<endl;
		std::string s;
		std::cin>>s;
		return 1;
	}
}



// there are max 64 primes returned, 64-bit vector thing
// to accomodate all returned plus the 32 chaining, 96 entry circular buffer
// use 128 for efficient bit operations

//530051400000000000 (zirkon)  5469 fpMIPS 16968 iMIPS
//        8556680160 - 10 s
//       51340080960 - 47 s
//       65500000000 -  1 min

// mangan 3888 FpMIPS 10918 iMIPS
//       65500000000 -  1 min 20s

