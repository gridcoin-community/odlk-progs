#include <iostream>
#include <fstream>
#include <list>
#include <set>
#include <ctime>
#include "kanonizator.h"

using namespace std;

typedef list<kvadrat>::iterator iter_dlk;
typedef set<kvadrat>::iterator iter_kf;

const char* input_2 = "input.txt";
const char* output = "output.txt";

list<kvadrat> baza_dlk;
set<kvadrat> baza_kf;

inline bool pars_arg(int argc, char* argv[]){
	bool yes = true, first = true, last = true;
	for(int i = 1; i < argc; i++){
		if(yes && argv[i] == "/m") yes = false;
		else if(first){input_2 = argv[i]; first = false;}
		else if(last){output = argv[i]; last = false;}
		else if(!yes) break;
	}
	return yes;
}

inline int error_input(const char* text, const char* file, int ret){
	cerr << text << file << endl;
	return ret;
}

void rabota();
void vyvod();
int init();

int main(int argc, char* argv[]){
	setlocale(LC_CTYPE, "rus");
	cout << "Канонизатор ДЛК10\n\n";
	bool yes = pars_arg(argc, argv);
	int ret;
	clock_t t0 = clock();
	if(!(ret = init())){
		rabota();
		vyvod();
	}
	clock_t t1 = clock();
	if(yes){
		cout << "Общее время работы: \t" << double(t1 - t0) / CLOCKS_PER_SEC << " сек\n\n";
	}
	return ret;
}

inline void zapis(char ch, kvadrat& kv, int& count){
	if(isdigit(ch)){
		kv[count++] = ch - '0';
		if(count == raz){
			if(is_dlk(kv)) baza_dlk.push_back(kv);
			count = 0;
		}
	}
}

int init(){
	clock_t t01 = clock();
	ifstream fin(input_2, ios::binary);
	if(!fin) return error_input("Нет файла ", input_2, 2);
	kvadrat tempk;
	int count = 0;
	static const int raz_buf = 256;
	char bufer[raz_buf];
	while(fin.read(bufer, raz_buf)) for(int i = 0; i < raz_buf; i++) zapis(bufer[i], tempk, count);
	if(fin.eof()) for(int i = 0; i < fin.gcount(); i++) zapis(bufer[i], tempk, count);
	if(baza_dlk.empty()) return error_input("Нет ДЛК в файле ", input_2, 3);
	clock_t t1 = clock();
	cout << "Введено ДЛК: \t\t" << baza_dlk.size() << endl;
	cout << "Время загрузки: \t" << double(t1 - t01) / CLOCKS_PER_SEC << " сек\n";
	return 0;
}

void rabota(){
	clock_t t0 = clock();
	kvadrat kf;
	for(iter_dlk q = baza_dlk.begin(); q != baza_dlk.end(); q++){
		Kanonizator_dlk::kanon(*q, kf);
		baza_kf.insert(kf);
	}
	clock_t t1 = clock();
	cout << "Найдено КФ: \t\t" << baza_kf.size() << endl;
	cout << "Время поиска: \t\t" << double(t1 - t0) / CLOCKS_PER_SEC << " сек\n";
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

void vyvod(){
	clock_t t0 = clock();
	ofstream fout(output, ios::binary);
	for(iter_kf q = baza_kf.begin(); q != baza_kf.end(); q++) out_kvadrat(fout, *q);
	clock_t t1 = clock();
	cout << "КФ записаны в файл: \t" << output << endl;
	cout << "Время записи: \t\t" << double(t1 - t0) / CLOCKS_PER_SEC << " сек\n";
}
