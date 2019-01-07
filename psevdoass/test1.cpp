#include <iostream>
#include <fstream>
#include <list>
#include <set>
#include <ctime>
#include "kanonizator_dlk_1.03/kanonizator.h"

using namespace std;

extern "C" void ASS_DLK10A(int*, void (*cb)(void*,int*),void *);
extern "C" void PSEVDOASS_DLK_NEW(int*, void (*cb)(void*,int*),void *);

list<kvadrat> baza_dlk;
unsigned gen_count;
const char* const input_1 = "hash_tabl.bin";
const char* output = "output.txt";
typedef list<kvadrat>::iterator iter_dlk;
typedef set<kvadrat>::iterator iter_kf;
set<kvadrat> baza_kf;

inline int error_input(const char* text, const char* file, int ret){
	cerr << text << file << endl;
	return ret;
}

void gen_output(void* arg, int* square)
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

int main(int argc, char* argv[]){

  int inputx[] = {4,  3,  7,  2,  1,  9,  5,  6};
  //int inputx[] = {1,  2,  3,  4,  5,  6,  7,  8};
	cout << "Running generator ASS_DLK10A\n";
  gen_count=0;
	clock_t t00 = clock();
  ASS_DLK10A( inputx, gen_output, 0 );
	clock_t t01 = clock();
	cout << "Generator ASS_DLK10A produced: \t\t" << gen_count << endl;
	cout << "Время работы: \t" << double(t01 - t00) / CLOCKS_PER_SEC << " сек\n";

	cout << "Running generator PSEVDOASS_DLK_NEW\n";
  gen_count=0;
	clock_t t10 = clock();
  PSEVDOASS_DLK_NEW( inputx, gen_output, 0 );
	clock_t t11 = clock();
	cout << "Generator PSEVDOASS_DLK_NEW produced: \t\t" << gen_count << endl;
	cout << "Время работы: \t" << double(t11 - t10) / CLOCKS_PER_SEC << " сек\n";

	cout << "Введено ДЛК: \t\t" << baza_dlk.size() << endl;

	clock_t t20 = clock();
	kvadrat kf;
	for(iter_dlk q = baza_dlk.begin(); q != baza_dlk.end(); q++){
		kanonizator::kanon(*q, kf);
		baza_kf.insert(kf);
	}
	clock_t t21 = clock();
	cout << "Найдено КФ: \t\t" << baza_kf.size() << endl;
	cout << "Время поиска: \t\t" << double(t21 - t20) / CLOCKS_PER_SEC << " сек\n";

  clock_t t30 = clock();
	ofstream fout(output, ios::binary);
	for(iter_kf q = baza_kf.begin(); q != baza_kf.end(); q++) out_kvadrat(fout, *q);
	clock_t t31 = clock();
	cout << "КФ записаны в файл: \t" << output << endl;
	cout << "Время записи: \t\t" << double(t31 - t30) / CLOCKS_PER_SEC << " сек\n";

  cout << "Общее время работы: \t" << double(t31 - t00) / CLOCKS_PER_SEC << " сек\n";
  return 0;
}

/*
  gen -apk -o output.txt 1 2 3 4 5 6 7 8
  -a : associative
  -p : pseudo-associative
  -k : canonicial form
  -m : mariazhnye
  -t : threads
*/

