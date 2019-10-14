#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <ctime>
#include "primesieve.hpp"
#include "primesieve/include/primesieve/PrimeGenerator.hpp"

using namespace std;

typedef unsigned long long ull;


primesieve::PrimeGenerator gen {500100101650038360,5001001016500383600};
uint64_t primes[128];
unsigned short disp[128];
unsigned y= 0;
unsigned z= 0;

void fill() {
	vector<uint64_t> newprimes;
	size_t newcnt = 0;
	newprimes.resize(64);
	newcnt = 0;
	gen.fill(newprimes,&newcnt);
	cout<<"new "<<newcnt<<endl;
	for(unsigned i=0; i<newcnt; ++i) {
		primes[(y+i)%128]=newprimes[i];
		disp[(y+i)%128]= primes[(y+i)%128] - primes[(y+i-1)%128];
	}
	y+=newcnt;
}

bool testit(unsigned k) {
	for(unsigned i=1; i<=((k-2)/2); ++i) {
		if( disp[(z+i)%128] != disp[(z+k-i)%128] )
			return false;
	}
	return true;
}

int main(){

	do {
		while ( (y-z)%128 < 8 ) {
			fill();
		}
		for(unsigned i=0; i<8; ++i)
			cout<<primes[(z+i)%128]<<" ";
		cout<<"$"<<endl;
		for(unsigned i=0; i<8; ++i)
			cout<<disp[(z+i)%128]<<" ";
		cout<<"|"<<endl;
		if(testit(6)){
			cout<<"6-tuple "<<primes[z%128]<<" : 0 "<<(primes[(z+1)%128]-primes[z%128])
			<<" "<<(primes[(z+2)%128]-primes[z%128])<<" "<<(primes[(z+3)%128]-primes[z%128])
			<<" "<<(primes[(z+4)%128]-primes[z%128])<<" "<<(primes[(z+5)%128]-primes[z%128])<<endl;
		}
		if(testit(16)){
			cout<<"16-tuple "<<primes[z%128]<<endl;
		}
		z++;

	} while(1);
	return 0;
}
// there are max 64 primes returned, 64-bit vector thing
// to accomodate all returned plus the 32 chaining, 96 entry circular buffer
// use 128 for efficient bit operations
