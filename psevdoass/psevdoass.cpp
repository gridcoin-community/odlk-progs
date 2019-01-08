#include <iostream>
#include <fstream>
#include <list>
#include <set>
#include <ctime>
#include <unistd.h>
#include <cstdlib>
#ifdef USE_THREADS
#include <thread>
#include <mutex>
#endif
#include "kanonizator_dlk_1.03/kanonizator.h"
#include "family_mar/prov_blk_trans.h"

using namespace std;

extern "C" void ASS_DLK10A(int*, void (*cb)(void*,int*),void *);
extern "C" void PSEVDOASS_DLK_NEW(int*, void (*cb)(void*,int*),void *);

list<kvadrat> baza_dlk;
unsigned gen_count;
typedef list<kvadrat>::iterator iter_dlk;
typedef set<kvadrat>::iterator iter_kf;
set<kvadrat> baza_kf;
set<kvadrat> baza_mar;
long long count_dlk;
#ifdef USE_THREADS
mutex cs_main;
#endif

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
"generator.exe -abkmt output X1 X2 X3 X4 X5 X6 X7 X8\n"
" -a : use ASS_DLK10A generator\n"
" -b : use PSEVDOASS_DLK_NEW generator\n"
" -k : find canonicial form\n"
" -m : search for Fancy DLS\n"
" -t : paralel search\n"
" output : file to write results to\n"
" X1-8   : generator parameters\n"
/*
*/
;

void kusok_raboty_mar(set<kvadrat>::iterator lki, set<kvadrat>::iterator lkend){
	Trans_DLx trans_dlx{};
	list<kvadrat> l_mar{};
	long long l_count{0};
	for(; lki!=lkend; lki++) {
		const kvadrat& lk = * lki;
		trans_dlx.search_trans(lk);
		if(trans_dlx.cnt_trans <= 1) return;
		kvadrat tempk[ch_srez - 1];
		const kvadrat* srez[ch_srez] = {&lk, &tempk[0], &tempk[1]};
		for(int i = 0; i < raz; i += por) for(int j = 0; j < por; j++) tempk[0][i + lk[i + j]] = j;
		for(int j = 0; j < por; j++) for(int i = 0; i < por; i++) tempk[1][lk[i * por + j] * por + j] = i;
		trans_dlx.search_symm_trans(srez);
		for(int i = 0; i < ch_srez; i++){
			l_count += trans_dlx.kf_trans[i].size();
			for(auto q = trans_dlx.kf_trans[i].begin(); q != trans_dlx.kf_trans[i].end(); q++){
				trans_dlx.find_d_trans(q->second, trans_dlx.trans[i]);
				if(trans_dlx.is_mar()){
					l_mar.push_back(q->first);
				}
			}
		}
	}
	#ifdef USE_THREADS
	cs_main.lock();
	#endif
	//cout<<"batch finish, dlk "<<l_count<<endl;
	copy(l_mar.begin(),l_mar.end(),inserter(baza_mar,baza_mar.begin()));
	count_dlk += l_count;
	#ifdef USE_THREADS
	cs_main.unlock();
	#endif
}

int main( int argc, char* argv[] )
{
	bool f_genA=false, f_genB=false, f_kanon=false, f_mar=false, f_thread=false;
	char *output_name;
	int inputx[8];
	{
		int c ;
		while( ( c = getopt (argc, argv, "abkmt") ) != -1 ) 
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
				case 'm':
					f_mar = true;
					break;
				case 't':
					f_thread = true;
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

	if(f_mar){
		if(!f_kanon) copy(baza_dlk.begin(),baza_dlk.end(),inserter(baza_kf,baza_kf.begin()));
		cerr << "Search for Fancy by Family Diagonal Latin Squares (not symmetric)" << endl;
		clock_t t0 = clock();
		time_t rt0 = time(0);

		#ifdef USE_THREADS
    set<kvadrat>::iterator baza_lk_i = baza_kf.begin();
		unsigned nb_threads = std::thread::hardware_concurrency();
		if(!nb_threads) nb_threads = 8;
		if(!f_thread) nb_threads = 1;
		cerr << "nb_threads: "<<nb_threads<<endl;
		unsigned batch_size = baza_kf.size() / nb_threads;
		unsigned batch_reminder = baza_kf.size() % nb_threads;
    std::vector< std::thread > my_threads(nb_threads-1);

		for(unsigned thi = 0; thi < nb_threads-1; ++thi)
		{
			auto ibegin=baza_lk_i;
			advance(baza_lk_i, batch_size);
			my_threads[thi] = std::thread( &kusok_raboty_mar, ibegin, baza_lk_i );
		}

		kusok_raboty_mar(baza_lk_i, baza_kf.end() );

		for( auto& th : my_threads)
			th.join();
		#else
		kusok_raboty_mar(baza_kf.begin(), baza_kf.end() );
		#endif

		cerr << "Checked DLS: " << count_dlk << endl;
		cerr << "CPU time: " << double(clock() - t0) / CLOCKS_PER_SEC << endl;
		cerr << "Step took: " << (time(0) - rt0) << endl;
		cerr << "Found Fancy DLS: " << baza_mar.size() << endl;
	}

	clock_t t30 = clock();
	ofstream fout(output_name, ios::binary);

	if(f_mar) {
		for(iter_kf q = baza_mar.begin(); q != baza_mar.end(); q++) out_kvadrat(fout, *q);
		cout << "Fancy DLS written to: \t" << output_name << endl;
	} else if(f_kanon){
		for(iter_kf q = baza_kf.begin(); q != baza_kf.end(); q++) out_kvadrat(fout, *q);
		cout << "CF DLS written to: \t" << output_name << endl;
	} else {
		for(auto q = baza_dlk.begin(); q != baza_dlk.end(); q++) out_kvadrat(fout, *q);
		cout << "DLK written to: \t" << output_name << endl;
	}

	clock_t t31 = clock();
	cout << "Step took: " << double(t31 - t30) / CLOCKS_PER_SEC << " s\n";

	cout << "Total run time: " << double(t31 - t00) / CLOCKS_PER_SEC << " s\n";

	return 0;
}
