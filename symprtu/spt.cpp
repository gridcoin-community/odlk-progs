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
uint64_t twinp;
vector<unsigned short> twind;
short twinz;
bool twin_enable;
int twins_min;
int twin_md;

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
		output = TOutput();
		output.start = input.start;
		output.chkpt = input.start-1;
	}
	twind.clear();
	twinz=-1;
	twin_enable = input.twin_k<255 || input.twin_min_k<255;
	twins_min = std::min(input.twin_k, input.twin_gap_k) - 1;
	output.twin_gap_d=  input.twin_gap_min;
	output.twin_gap_6d= input.twin_gap_kmin;
	if(input.mine_k<2 || input.mino_k<3 || !input.mino_k&1 || input.mine_k&1
		|| input.twin_min_k<4
		|| input.twin_k<1
		|| input.max_k>64
		|| (input.twin_gap_k!=6 && input.twin_gap_k!=255)
		|| (input.twin_gap_k!=255 && !twin_enable)
		|| input.exit_early || input.out_all_primes
		|| input.primes_in.size() ){
		throw EApp{"Unsupported input options"};
	}
	newprimes.resize(64);
	z= 32;
	y= 31;
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

void process_twin_seq() {
	if(twind.size()+1 >= input.twin_k) {
		output.twins.emplace_back();
		auto& el = output.twins.back();
		el.start = twinp;
		el.k = 0;
		el.ofs.resize( twind.size() );
		std::copy(twind.begin(), twind.end(), el.ofs.begin());
	}
	bool twin_gap_cond6 = (twin_md>output.twin_gap_6d && (twind.size()+1)>5);
	bool twin_gap_cond1 = (twin_md>output.twin_gap_d);
	if( twin_gap_cond6 || twin_gap_cond1 )
	{
		if(twin_gap_cond6) output.twin_gap_6d = twin_md;
		if(twin_gap_cond1) output.twin_gap_d  = twin_md;
		output.twin_gap.emplace_back();
		auto& el = output.twin_gap.back();
		el.start = twinp;
		el.k = 0;
		el.ofs.resize( twind.size() );
		std::copy(twind.begin(), twind.end(), el.ofs.begin());
	}
}

static void check_twins() {
	if(disp[(z)%128]==2) {
		if(twinz==-1) {
			//start of twin prime sequence
			twinp= primes[(z-1)%128];
			twinz = z%2;
			twind.clear();
			twin_md= 0;
		} else {
			//continuation of sequence
			auto d = disp[(z-1)%128];
			twind.push_back( d );
			if(d>twin_md)
				twin_md= d;
		}
	}
	else if( z%2 == twinz ) {
		//end of twin prime sequence
		if( twind.size() >= twins_min
			|| twin_md > output.twin_gap_d
		  )
			process_twin_seq();
		twinz=-1;
	}
}

void save_tuple(unsigned k, bool twin) {
	auto& lst = twin? output.twin_tuples : output.tuples;
	lst.emplace_back();
	auto& el = lst.back();
	unsigned st= z-(k/2);
	el.start = primes[(st)%128];
	el.k = k;
	el.ofs.resize( k / 2 );
	st++;
	for(unsigned i=0; i < (k/2); ++i)
		el.ofs[i] = disp[(st+i)%128];
}

void fill() {
	size_t newcnt = 0;
	newcnt = 0;
	gen->fill(newprimes,&newcnt);
	if(gen->finished())
		throw EApp{"Prime Generator failed"};
	output.nprime += newcnt;
	if(newcnt)
		output.last=newprimes[newcnt-1];
	for(unsigned i=0; i<newcnt; ++i) {
		primes[(y+i)%128]=newprimes[i];
		disp[(y+i)%128]= primes[(y+i)%128] - primes[(y+i-1)%128];
	}
	y+=newcnt;
}

static long get_twin_seq_len(unsigned h)
{
	long h2=-1;
	for(long i=h; i>=0; i-=2) {
		if(2!=disp[(z+i)%128])
			h2=i-2;
	}
	return (h2+1)*2;
}

static unsigned get_even_seq_len()
{
	// even k-tuple has first d of zero and then odd length palindrome
	unsigned h2=0;
	for(unsigned h = 1; h < 32; ++h ) {
		if(disp[(z-h)%128]==disp[(z+h)%128])
			h2= h;
		else break;
	}
	return h2;
}

static void check_even_tuples() {
	unsigned h= get_even_seq_len();
	unsigned k= (h+1)*2;
	long kw;
	if( k>=input.mine_k )
		save_tuple(k,0);
	if( k>=input.twin_min_k && (kw=get_twin_seq_len(h)) >=input.twin_min_k)
		save_tuple(kw,1);
}

static void check_odd_tuples() {
	// odd k-tuple has first d of zero and then even length palindrome
	unsigned h2=0;
	for(unsigned h = 1; h < 32; ++h ) {
		if(disp[(z+1-h)%128]==disp[(z+h)%128])
			h2= h;
		else break;
	}
	//unsigned k=(h2+1)*2;
	unsigned k=h2*2+1;
	if( k>=input.mino_k )
		save_tuple(k,0);
	//odd tuples cant be twin
}

int main(){
	clock_t tlast = clock();
	uint64_t iter = 0;

	try {
		initialize();

		do {
			// Ensure there are 31 primes in the front and one current one
			while ( (y-z)%128 < 32 ) {
				fill();
				iter++;
			}

			if(input.max_k) {
				check_even_tuples();
				check_odd_tuples();
			}
			if(twin_enable)
				check_twins();

			output.chkpt = primes[(z-1)%128]; // this is the last prime fully checked

			/* The -1 is there because 3 is the smallest tuple we check for and that
			tuple starts at z-1. Larger tuples start earlier. */

			if(output.chkpt >= input.end && twinz==-1) {
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
