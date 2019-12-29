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
	cout<<"twins:"<<endl;
	for(const auto& tuple : output.twins) {
		cout<<"W "<<tuple.start<<"("<<(tuple.ofs.size()+1)<<"):";
		for(const auto& d : tuple.ofs)
			cout<<" "<<d;
		cout<<endl;
	}
	cout<<"twin_tuples:"<<endl;
	for(const auto& tuple : output.twin_tuples) {
		cout<<"U "<<tuple.start<<"("<<tuple.k<<"):";
		for(const auto& d : tuple.ofs)
			cout<<" "<<d;
		cout<<endl;
	}
	cout<<"twin_cnt:"<<endl;
	for(const std::pair<unsigned,unsigned long>& p : output.twin_cnt) {
		cout<<"C "<<p.first<<" "<<p.second<<endl;
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
	inp.start= 500100101650038360;
	inp.end= inp.start + 1026800000;
	inp.mine_k= 16;
	inp.mino_k= 5;
	inp.max_k= 32;
	inp.upload = 0;
	inp.exit_early= 0;
	inp.out_last_primes= 1;
	inp.out_all_primes= 0;
	inp.primes_in.clear();
	inp.twin_k=5;
	inp.twin_cnt_k=255;
	inp.twin_min_k=0;
	inp.twin_max_k=0;
	write_input("sample.dat");
}

int main(int argc, char** argv){
	if(argc==2) {
		read_output(argv[1]);
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

//500001189895960000
//        8556680160 -  5 min
//       51340080960 - 30 min
