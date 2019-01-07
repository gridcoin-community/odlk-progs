#include <iostream>
#include <fstream>
#include <list>
#include <set>
#include <ctime>
#include <unistd.h>
#include <cstdlib>
#include "kanonizator_dlk_1.03/kanonizator.h"

using namespace std;

extern "C" void ASS_DLK10A(int*, void (*cb)(void*,int*),void *);
extern "C" void PSEVDOASS_DLK_NEW(int*, void (*cb)(void*,int*),void *);

list<kvadrat> baza_dlk;
unsigned gen_count;
typedef list<kvadrat>::iterator iter_dlk;
typedef set<kvadrat>::iterator iter_kf;
set<kvadrat> baza_kf;

inline int error_input(const char* text, const char* file, int ret){
	cerr << text << file << endl;
	return ret;
}

void generator_output(void* arg, int* square)
{
	kvadrat tempk;
	for(int j=0; j<raz; ++j)
	{
		tempk[j]= square[j];
	}
	++gen_count;
	if(is_dlk(tempk))
	{
		baza_dlk.push_back(tempk);
	}
}


inline void out_kvadrat(ostream& out, const kvadrat& kv){
	static const int raz_kvb = 212;
	array<char,raz_kvb> tempk;
	char* p = tempk.data();
	for(int i = 0; i < raz; i += por){
		*p++ = kv[i] + '0';
		for(int j = 1; j < por; j++){
			*p++ = ' ';
			*p++ = kv[i + j] + '0';
		}
		*p++ = '\r';
		*p++ = '\n';
	}
	*p++ = '\r';
	*p++ = '\n';
	out.write((char*)tempk.data(), sizeof(tempk));
}

const char help_text[] =
"Experimental generator of Pseudo-Associative Diagonal Latin Squares.\n"
"generator.exe -abk output X1 X2 X3 X4 X5 X6 X7 X8\n"
" -a : use ASS_DLK10A generator\n"
" -b : use PSEVDOASS_DLK_NEW generator\n"
" -k : find canonicial form\n"
" output : file to write results to\n"
" X1-8   : generator parameters\n"
/*
	-m : mariazhnye
	-t : threads
*/
;



int main( int argc, char* argv[] )
{
	bool f_genA=false, f_genB=false, f_kanon=false;
	char *output_name;
	int inputx[8];
	{
		int c ;
		while( ( c = getopt (argc, argv, "abk") ) != -1 ) 
		{
			switch(c)
			{
				case 'a':
					f_genA = true;
					break;
				case 'b':
					f_genB= true;
					break;
				case 'k':
					f_kanon = true;
					break;
				default:
					cerr << "Unknown Option" << endl << help_text;
					return 2;
			}
		}

		if( argc - optind != 9 ){
			cerr << "Expected 9 arguments" << endl << help_text;
			return 2;
		}

		output_name = argv[optind++];

		for( int i = 0; i < 8; ++i){
			inputx[i] = std::atoi(argv[optind]);
			optind++;
		}
	}

	clock_t t00 = clock();

	if(f_genA){
		gen_count=0;
		ASS_DLK10A( inputx, generator_output, 0 );
		clock_t t01 = clock();
		cout << "ASS_DLK10A: " << gen_count << endl;
		cout << "Step took: " << double(t01 - t00) / CLOCKS_PER_SEC << " s\n";
	}

	if(f_genB){
		gen_count=0;
		PSEVDOASS_DLK_NEW( inputx, generator_output, 0 );
		clock_t t01 = clock();
		cout << "PSEVDOASS_DLK_NEW: " << gen_count << endl;
		cout << "Step took: " << double(t01 - t00) / CLOCKS_PER_SEC << " s\n";
	}

	cout << "Have DLS: " << baza_dlk.size() << endl;

	if(f_kanon){
		clock_t t20 = clock();
		kvadrat kf;
		for(iter_dlk q = baza_dlk.begin(); q != baza_dlk.end(); q++){
			kanonizator::kanon(*q, kf);
			baza_kf.insert(kf);
		}
		clock_t t21 = clock();
		cout << "Found CF: " << baza_kf.size() << endl;
		cout << "Step took: " << double(t21 - t20) / CLOCKS_PER_SEC << " s\n";
	}

	clock_t t30 = clock();
	ofstream fout(output_name, ios::binary);

	if(f_kanon){
		for(iter_kf q = baza_kf.begin(); q != baza_kf.end(); q++) out_kvadrat(fout, *q);
		cout << "КФ записаны в файл: \t" << output_name << endl;
	} else {
		for(auto q = baza_dlk.begin(); q != baza_dlk.end(); q++) out_kvadrat(fout, *q);
		cout << "ДЛК записаны в файл: \t" << output_name << endl;
	}

	clock_t t31 = clock();
	cout << "Step took: " << double(t31 - t30) / CLOCKS_PER_SEC << " s\n";

	cout << "Total run time: " << double(t31 - t00) / CLOCKS_PER_SEC << " s\n";

	return 0;
}
