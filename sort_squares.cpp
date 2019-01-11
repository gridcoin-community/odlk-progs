#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <algorithm>
#include "kanonizator_dlk_1.03/kanonizator.h"

using namespace std;

vector<kvadrat> baza;

inline int error_input(const char* text, const string& file, int ret){
	cerr << text << file << endl;
	return ret;
}

inline void zapis(char ch, kvadrat& kv, int& count){
	if(isdigit(ch)){
		kv[count++] = ch - '0';
		if(count == raz){
			baza.push_back(kv);
			count = 0;
		}
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
  if(argc!=3) { cerr<<"argument error"<<endl; return 1; }
  string input_2 = argv[1];
  string output = argv[2];
	ifstream fin(input_2, ios::binary);
  if(!fin) return error_input("No file ", input_2, 2);
	kvadrat tempk;
	int count = 0;
	static const int raz_buf = 256;
	char bufer[raz_buf];
	while(fin.read(bufer, raz_buf)) for(int i = 0; i < raz_buf; i++) zapis(bufer[i], tempk, count);
	if(fin.eof()) for(int i = 0; i < fin.gcount(); i++) zapis(bufer[i], tempk, count);

  std::sort(baza.begin(), baza.end());

	ofstream fout(output, ios::binary);
	for(auto q = baza.begin(); q != baza.end(); q++) out_kvadrat(fout, *q);

  return 1;
}

