#include "odlkcommon/common.h"
#include "prov_blk_trans.h"
#include <cassert>
#ifdef USE_THREADS
#include <thread>
#include <mutex>
#endif


vector<kvadrat> baza_lk;
set<kvadrat> baza_mar;
const char* input;
long long count_dlk;
#ifdef USE_THREADS
mutex cs_main;
#endif

inline bool error_input(const char* text, const char* file){
	cerr << text << file << endl;
	return false;
}

inline void zapis(char ch, kvadrat& kv){
	static int count;
	if(ch >= '0' && ch <= '9'){
		kv[count++] = ch - '0';
		if(count == raz){
			if(is_lk(kv)) baza_lk.push_back(kv);
			count = 0;
		}
	}
}

int init(){
	ifstream fin(input, ios::binary);
	if(!fin) return error_input("File not Found ", input);
	kvadrat tempk;
	const int raz_buf = 0x1000;
	char bufer[raz_buf];
	while(fin.read(bufer, raz_buf)) for(int i = 0; i < raz_buf; i++) zapis(bufer[i], tempk);
	if(fin.eof()) for(int i = 0; i < fin.gcount(); i++) zapis(bufer[i], tempk);
	if(baza_lk.empty()) return error_input("No LS in file ", input);
	//TODO for(int i = 0; i < ch_srez; i++) trans[i].reserve(max_trans); may increase speed
	return true;
}

void kusok_raboty(vector<kvadrat>::iterator lki, vector<kvadrat>::iterator lkend){
	Trans_DLx trans_dlx{};
	//find only non-symmetric solutions
	trans_dlx.f_simm= false;
	list<kvadrat> l_mar{};
	long long l_count{0};
	for(; lki!=lkend; lki++) {
		kvadrat& lk = * lki;
		trans_dlx.search_trans(lk);
		if(trans_dlx.cnt_trans <= 1) continue;
		kvadrat tempk[Trans_DLx::ch_srez - 1];
		const kvadrat* srez[Trans_DLx::ch_srez] = {&lk, &tempk[0], &tempk[1]};
		for(int i = 0; i < raz; i += por) for(int j = 0; j < por; j++) tempk[0][i + lk[i + j]] = j;
		for(int j = 0; j < por; j++) for(int i = 0; i < por; i++) tempk[1][lk[i * por + j] * por + j] = i;
		trans_dlx.search_symm_trans(srez);
		for(int i = 0; i < Trans_DLx::ch_srez; i++){
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



const char help_text[] =
"Search for Fancy by Family Diagonal Latin Squares (not symmetric).\n"
"family_mar.exe input output\n"
" input : file to read Latin Squares from\n"
" output: file to write Fancy Diagonal Latin Squares to\n"
;

int main(int argc, char* argv[]){
	setlocale(LC_CTYPE, "rus");
	cerr << "Поиск марьяжных ДЛК (кроме симметричных) для семейства ЛК" << endl;

	if(argc!=3){
		cerr << "Expected 2 arguments" << endl << help_text;
		return 2;
	}

	input = argv[1];
	string output = argv[2];

	if(init()){
		cerr << "Have LS: " << baza_lk.size() << endl;
		clock_t t0 = clock();
		time_t rt0 = time(0);
		clock_t t1;
		time_t rt1;

		#ifdef USE_THREADS
    vector<kvadrat>::iterator baza_lk_i = baza_lk.begin();
		unsigned nb_threads = std::thread::hardware_concurrency();
		if(!nb_threads) nb_threads = 8;
		cerr << "nb_threads: "<<nb_threads<<endl;
		unsigned batch_size = baza_lk.size() / nb_threads;
		unsigned batch_reminder = baza_lk.size() % nb_threads;
    std::vector< std::thread > my_threads(nb_threads-1);

		for(unsigned thi = 0; thi < nb_threads-1; ++thi)
		{
			my_threads[thi] = std::thread( &kusok_raboty, baza_lk_i, baza_lk_i+batch_size );
			baza_lk_i+=batch_size;
		}

		kusok_raboty(baza_lk_i, baza_lk_i+batch_size+batch_reminder );
		baza_lk_i+=batch_size+batch_reminder;

		for( auto& th : my_threads)
			th.join();
		assert(baza_lk_i==baza_lk.end());
		#else
		kusok_raboty(baza_lk.begin(), baza_lk.end() );
		#endif

		cerr << "Checked DLK: " << count_dlk << endl;
		cerr << "Run Time (s): " << double(clock() - t0) / CLOCKS_PER_SEC << endl;
		cerr << "Real Run Time (s): " << (time(0) - rt0) << endl;
		cerr << "Found Fancy DLS: " << baza_mar.size() << endl;

		ofstream fout(output, ios::binary);
		for(auto q = baza_mar.begin(); q != baza_mar.end(); q++)
			out_kvadrat(fout, *q);
		cerr << "Они записаны в файл " << output << endl;

		return 0;
	} else
		return 1;
}
