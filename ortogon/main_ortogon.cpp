#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include "ortogon/exact_cover.h"

using namespace std;

const int timeout = 5 * CLOCKS_PER_SEC;

list<kvadrat> baza_dlk;
map<kvadrat,list<kvadrat>> baza_mar;
int count_trans;
const char* input = "input.txt";

bool init();
void vyvod(const string& str);

int main(int argc, char* argv[]){
	setlocale(LC_CTYPE, "rus");
	cout << "Проверка ДЛК10 на марьяжность (ОДЛК)\n\n";
	if(argc > 1) input = argv[1];
	if(!init()){
		cout << "Для выхода нажмите любую клавишу: ";
		system("pause > nul");
		cout << endl;
		return 1;
	}
	clock_t t0 = clock(), tb, te;
	tb = t0;
	int count = 0;
	for(list<kvadrat>::iterator q = baza_dlk.begin(); q != baza_dlk.end(); q++){
		Exact_cover exact_cover{}; // todo: take out of loop once sure
		list<kvadrat> mates;
		exact_cover.search_mate(*q, mates);
		baza_mar.insert(std::make_pair(*q, mates));
		count++;
		if((te = clock()) - tb > timeout){
			cout << "Проверено ДЛК: " << count << " найдено ОДЛК: " << baza_mar.size() << " время: "
				<< (te - t0) / CLOCKS_PER_SEC << " сек\n";
			tb = te;
		}
	}
	vyvod(argc > 1 ? string("m") + input : string("output.txt"));
	clock_t t1 = clock();
	cout << "Найдено ОДЛК: " << baza_mar.size() << endl;
	cout << "Время работы: " << double(t1 - t0) / CLOCKS_PER_SEC << " сек\n\n";
	if(argc == 1){
		cout << "Для выхода нажмите любую клавишу: ";
		system("pause > nul");
		cout << endl;
	}
	return 0;
}

inline void zapis(char ch, kvadrat& kv, int& count){
	if(isdigit(ch)){
		kv[count++] = ch - '0';
		if(count == raz){
			if(Exact_cover::is_dlk(kv)) baza_dlk.push_back(kv);
			count = 0;
		}
	}
}

const int raz_buf = 256;
bool init(){
	ifstream fin(input, ios::binary);
	if(!fin){cerr << "Нет файла " << input << endl; return false;}
	kvadrat tempk;
	int count = 0;
	char bufer[raz_buf];
	while(fin.read(bufer, raz_buf)) for(int i = 0; i < raz_buf; i++) zapis(bufer[i], tempk, count);
	if(fin.eof()) for(int i = 0; i < fin.gcount(); i++) zapis(bufer[i], tempk, count);
	if(baza_dlk.empty()){cerr << "Нет ДЛК в файле " << input << endl; return false;}
	cout << "Введено ДЛК:  " << baza_dlk.size() << endl;
	return true;
}

void vyvod(const string& str){
	if(baza_mar.empty()) return;
	ofstream fout(str, ios::binary);
	int count;
	for(auto q = baza_mar.begin(); q != baza_mar.end(); q++){
		ostringstream ss;
		ss << "DLK(" << q->second.size() << "):\r\n";
		fout.write(ss.str().c_str(), ss.str().size());
		out_kvadrat(fout,q->first);
		count = 0;
		for(auto w = q->second.begin(); w != q->second.end(); w++){
			ostringstream ss;
			ss << "mate#" << ++count << ":\r\n";
			fout.write(ss.str().c_str(), ss.str().size());
			out_kvadrat(fout,*w);
		}
		fout.write("\r\n", 2);
	}
}



