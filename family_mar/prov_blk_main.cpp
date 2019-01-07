#include "kanonizator.h"

list<kvadrat> baza_lk;
list<transver> d_trans;
vector<transver> trans[ch_srez];
list<pair<kvadrat, pair<transver, transver>>> kf_trans[ch_srez];
set<kvadrat> baza_kf, baza_mar;
const char* input, *input_tabl = "hash_tabl.bin";
int cnt_trans, count_dlk;

inline bool error_input(const char* text, const char* file){
	cerr << text << file << endl;
	return false;
}

inline bool is_lk(const kvadrat& kv){
	for(int i = 0, flag; i < raz; i += por){
		flag = 0;
		for(int j = 0, t; j < por; j++){
			if(flag & (t = 1 << kv[i + j])) return false;
			flag |= t;
		}
	}
	for(int i = 0, flag; i < por; i++){
		flag = 0;
		for(int j = 0, t; j < raz; j += por){
			if(flag & (t = 1 << kv[i + j])) return false;
			flag |= t;
		}
	}
	return true;
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

bool init(){
	ifstream fin(input_tabl, ios::binary);
	if(!fin) return error_input("Нет файла ", input_tabl);
	fin.read((char*)kanonizator::hash_tabl, sizeof(kanonizator::hash_tabl));
	fin.close();
	fin.open(input, ios::binary);
	if(!fin) return error_input("Нет файла ", input);
	kvadrat tempk;
	const int raz_buf = 0x1000;
	char bufer[raz_buf];
	while(fin.read(bufer, raz_buf)) for(int i = 0; i < raz_buf; i++) zapis(bufer[i], tempk);
	if(fin.eof()) for(int i = 0; i < fin.gcount(); i++) zapis(bufer[i], tempk);
	if(baza_lk.empty()) return error_input("Нет ЛК в файле ", input);
	for(int i = 0; i < ch_srez; i++) trans[i].reserve(max_trans);
	trans_dlx::nodes.resize(trans_dlx::max_nodes);
	return true;
}

void find_d_trans(const pair<transver, transver>& simm_tr, const vector<transver>& tr){
	d_trans.clear();
	for(int i = 0, c1, c2; i < cnt_trans; i++){
		c1 = c2 = 0;
		for(int j = 0, t; j < por; j++){
			if((t = tr[i][j]) == simm_tr.first[j]) c1++;
			if(c1 > 1) goto next;
			if(t == simm_tr.second[j]) c2++;
			if(c2 > 1) goto next;
		}
		if(c1 == 1 && c2 == 1) d_trans.push_back(tr[i]);
		next:;
	}
}

void rabota(const kvadrat& lk){
	trans_dlx::search_trans(lk);
	if((cnt_trans = trans[0].size()) <= 1) return;
	kvadrat tempk[ch_srez - 1];
	const kvadrat* srez[ch_srez] = {&lk, &tempk[0], &tempk[1]};
	for(int i = 0; i < raz; i += por) for(int j = 0; j < por; j++) tempk[0][i + lk[i + j]] = j;
	for(int j = 0; j < por; j++) for(int i = 0; i < por; i++) tempk[1][lk[i * por + j] * por + j] = i;
	trans_dlx::search_symm_trans(srez);
	for(int i = 0; i < ch_srez; i++){
		count_dlk += kf_trans[i].size();
		for(auto q = kf_trans[i].begin(); q != kf_trans[i].end(); q++){
			find_d_trans(q->second, trans[i]);
			if(trans_dlx::is_mar()) baza_mar.insert(q->first);
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
	*p = '\n';
	out.write((char*)tempk.data(), sizeof(tempk));
}


int main(int argc, char* argv[]){
	setlocale(LC_CTYPE, "rus");
	cerr << "Поиск марьяжных ДЛК (кроме симметричных) для семейства ЛК" << endl;

	input = argv[1];
	string output = argv[2];

	if(init()){
		cerr << "Введено ЛК: " << baza_lk.size() << endl;
		clock_t t0 = clock();

		for(auto q = baza_lk.begin(); q != baza_lk.end(); q++)
			rabota(*q);

		cerr << "Проверено ДЛК: " << count_dlk << endl;
		cerr << "Время работы в сек: " << double(clock() - t0) / CLOCKS_PER_SEC << endl;
		cerr << "Найдено марьяжных ДЛК: " << baza_mar.size() << endl;

		ofstream fout(output, ios::binary);
		for(auto q = baza_mar.begin(); q != baza_mar.end(); q++)
			out_kvadrat(fout, *q);
		cerr << "Они записаны в файл " << output << endl;

		return 0;
	} else
		return 1;
}
