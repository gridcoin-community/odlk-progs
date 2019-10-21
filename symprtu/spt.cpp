#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
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
		FILE* f = boinc_fopen(fn2.c_str(), "rb");
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
		FILE* f = boinc_fopen("tmp", "wb");
		if(!f)
			throw std::runtime_error("fopen");
		if( fwrite(this->getbase(), 1, this->pos(), f) !=this->pos())
			throw std::runtime_error("fwrite");
		fclose(f);
#ifdef WIN32
		unlink(fn2.c_str());
#endif
		if( rename("tmp",fn2.c_str()) <0)
			throw std::runtime_error("rename");
	}
};

std::unique_ptr<primesieve::PrimeGenerator> gen;
uint64_t primes[128];
unsigned short disp[128];
unsigned y= 0;
unsigned z= 0;
vector<uint64_t> newprimes;
TInput input;
TOutput output;

void fill();
void initialize() {
	/* init boinc api */
	BOINC_OPTIONS opts;
	APP_INIT_DATA binitdata;
	char buf[64];
	boinc_options_defaults(opts);
	opts.direct_process_action = 1;
	int retval = boinc_init_options(&opts);
	if(retval) 
			throw EBoincApi(retval, "boinc_init_options");
	boinc_get_init_data(binitdata);
	/* now init the app */
	input.readInput(CBoincFileStream(2,"input.dat"));
	try {
		output.readOutput(CBoincFileStream(0,"checkpoint"));
	} catch(EFileNotFound& e) {
		output.start = input.start;
		output.chkpt = input.start-1;
		output.last = 0;
		output.nprime = 0;
		output.primes.clear();
		output.tuples.clear();
		output.sieve_init_cs=0;
	}
	if(input.min_k!=16 || input.max_k!=32
		|| input.exit_early || input.out_all_primes
		|| input.primes_in.size() ){
		throw EApp{"Unsupported input options"};
	}
	clock_t t0 = clock();
	gen = std::make_unique<primesieve::PrimeGenerator>(
		output.chkpt+1, UINT64_MAX
	);
	fill();
	output.sieve_init_cs += double(clock()-t0)*100/CLOCKS_PER_SEC;
	// todo: verify the +1 is correct
	/* no abort prevention logic */
}

void checkpoint(int mode) {
	if(input.out_last_primes) {
		output.primes.resize(128);
		std::copy(std::begin(primes), std::end(primes), output.primes.begin());
	}
	if(mode==2) {
		output.status = TOutput::x_end;
		CBoincFileStream fs;
		output.writeOutput(fs);
		fs.writeFile(3,"output.dat");
	} else if(mode==1) {
		output.status = TOutput::x_chkpt;
		CBoincFileStream fs;
		output.writeOutput(fs);
		fs.writeFile(1,"checkpoint");
	}
	boinc_checkpoint_completed();
}

void save_tuple(unsigned k) {
	output.tuples.emplace_back();
	auto& el = output.tuples.back();
	el.start = primes[(z)%128];
	el.k = k;
	el.ofs.resize( (k + 1) / 2 );
	for(unsigned i=1; i<((k+3)/2); ++i)
		el.ofs[i-1] = disp[(z+i)%128];
}

void fill() {
	size_t newcnt = 0;
	newcnt = 0;
	gen->fill(newprimes,&newcnt);
	if(gen->finished())
		throw EApp{"Prime Generator failed"};
	output.nprime += newcnt;
	output.last=newprimes[newcnt-1];
	for(unsigned i=0; i<newcnt; ++i) {
		primes[(y+i)%128]=newprimes[i];
		disp[(y+i)%128]= primes[(y+i)%128] - primes[(y+i-1)%128];
	}
	y+=newcnt;
}

bool testit(unsigned k) {
	bool r=true;
	for(unsigned i=1; i<=((k-1)/2); ++i) {
		if( disp[(z+i)%128] != disp[(z+k-i)%128] )
			r=false;
	}
	return r;
}

void testit2(bool r[]) {
	for(unsigned k=16; k<=32; ++k) {
		r[k-16] = disp[(z+1)%128] == disp[(z+k-1)%128];
	}
	for(unsigned i=2; i<=((32-2)/2); ++i) {
		for(unsigned k=16; k<=32; ++k) {
			if(i <= ((k-1)/2)) {
				r[k-16] &= disp[(z+i)%128] == disp[(z+k-i)%128];
			}
		}
	}
}

int main(){
	newprimes.resize(64);
	clock_t tlast = clock();
	uint64_t iter = 0;
	uint64_t zi = 0;


	try {
		initialize();

		do {
			while ( (y-z)%128 < 32 ) {
				fill();
				iter++;
			}
			for(unsigned k=16; k<=64; ++k) {
				if( testit(k) )
					save_tuple(k);
			}

			output.chkpt = primes[(z)%128]; // this is the last prime fully checked

			if(output.chkpt >= input.end) {
				checkpoint(2);
				boinc_finish(0);
				break;
			}
			if(iter > 4500000) {
				float fraction = float(output.chkpt - input.start)/(input.end - input.start);
				boinc_fraction_done(fraction);
				iter = 0;
				if(boinc_time_to_checkpoint()) {
					checkpoint(1);
				}
			}

			z++;
		} while(1);
	}
	catch( const std::exception& e ) {
		std::string msg="Program failed due to exception. "+std::string(e.what());
		cerr<<msg<<endl;
		boinc_finish_message(1,msg.c_str(),false);
	}
	return 1;
}



// there are max 64 primes returned, 64-bit vector thing
// to accomodate all returned plus the 32 chaining, 96 entry circular buffer
// use 128 for efficient bit operations

//500001189895960000
//        8556680160 -  5 min
//       51340080960 - 30 min
