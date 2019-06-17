#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include "kanonizator_gen.h"

using namespace std;

int X[n], S[n], l, col[por], row[por], lin, raz_bz, cnt_bz, start[raz], raz_st;
ull cnt_sndlk;
kanonizator is_kf;
vector<kvadrat_b> baza;

inline void update_downdate(int l){
	int t = rc[l];
	int x = X[l];
	row[t >> 4] ^= x;
	col[t & 0xf] ^= x;
}

inline bool init_gen(){
	for(int i = 0; i < por; i++){
		row[i] = (1 << i) | (1 << diag[lin][i]);
		col[i] = (1 << i) | (1 << diag[lin][por - 1 - i]);
	}
	if(!raz_st) return true;
	for(int x, s, r, c; l < raz_st; l++){
		x = 1 << start[l];
		s = (row[r = rc[l] >> 4] | col[c = rc[l] & 0xf]) ^ all;
		if(!(s & x)) return true;
		X[l] = x;
		row[r] |= x;
		col[c] |= x;
		S[l] = s & -x;
	}
	if(l < n) return true;
	while(--l >= 0){
		update_downdate(l);
		if(S[l] ^= X[l]){
			X[l] = (S[l] & (-S[l]));
			update_downdate(l++);
			return true;
		}
	}
	return false;
}

inline bool vizit(const kvadrat& kf){
	char* p = &baza[cnt_bz][0];
	for(int i = 0; i < raz; i += por){
		*p++ = kf[i] + '0';
		for(int j = 1; j < por; j++){
			*p++ = ' ';
			*p++ = kf[i + j] + '0';
		}
		//*p++ = '\r';
		*p++ = '\n';
	}
	//*p++ = '\r';
	*p++ = '\n';
	return ++cnt_bz < raz_bz;
}

bool init();
void vyvod();
void generator();
void checkpoint();

int main(){
	setlocale(LC_CTYPE, "rus");
	cout << "Генератор КФ ДЛК10\n\n";
	if(init()){
		clock_t t0 = clock();
		generator();
		vyvod();
		checkpoint();
		clock_t t1 = clock();
		cout << "Время работы: \t" << double(t1 - t0) / CLOCKS_PER_SEC << " сек\n\n";
	}
	return 1;
}

bool init(){
	ifstream fin("config.txt");
	if(!fin){cerr << "Нет файла config.txt\n"; return false;}
	if(!(fin >> lin >> raz_bz)){cerr << "Неверный формат config.txt\n"; return false;}
	if(lin < 1 || lin > ch_lin || raz_bz < 1){cerr << "Неверный формат config.txt\n"; return false;}
	fin.close();
	fin.open("start.txt");
	if(fin){
		char ch;
		for(int k = 0, i, j; k < raz && fin.get(ch);){
			if(!isdigit(ch)) continue;
			i = k / por;
			j = k++ % por;
			if(i == j || j == por - 1 - i) continue;
			start[raz_st++] = ch - '0';
		}
		if(!fin && !fin.eof()){cerr << "Неверный формат start.txt\n"; return false;}
	}
	lin--;
	baza.resize(raz_bz);
	return true;
}

/*
 * next: backtrack
 * first: enter
*/




void generator(){
	unsigned long steps_between = 0;
	int min_l = 81;
	static const int timeout = CLOCKS_PER_SEC << 3;
	clock_t t0 = clock(), t1, tb, te;
	if(!init_gen()) goto end;
	cout << "Старт:\n\n";
	cout<<"l: "<<l<<endl;
	kvadrat tempk;
	for(int i = 0; i < por; i++){
		tempk[i * (por + 1)] = i;
		tempk[ (i + 1) * (por - 1)] = diag[lin][i];
	}
	for(int i = 0, t; i < l; i++){
		t = 0;
		if(X[i] & 0x300) t += 8;
		if(X[i] & 0xf0)  t += 4;
		if(X[i] & 0xcc)  t += 2;
		if(X[i] & 0x2aa) t += 1;
		tempk[(rc[i] >> 4) * por + (rc[i] & 0xf)] = t;
	}
	for(int i = l; i < n; i++) tempk[(rc[i] >> 4) * por + (rc[i] & 0xf)] = por;
	for(int i = 0; i < raz; i++) cout << (tempk[i] == por ? '.' : char(tempk[i] + '0'))
		<< (i % por != por - 1 ? ' ' : '\n');
	cout << endl;
	tb = clock();
enter_level_l: 
	if(l >= n){
		cnt_sndlk++;
		//cout<<"steps_between:"<<steps_between<<endl;
		steps_between=0;
		if( is_kf(X) && !vizit(kanonizator::formy[0])){
		//is_kf(X);if(false /*!vizit(kanonizator::formy[0])*/){
			t1 = clock();
			cout << "Найдено КФ[" << (lin + 1) << "]: " << cnt_bz << " время поиска: "
				<< double(t1 - t0) / CLOCKS_PER_SEC << " сек\n";
			cout << "Проверено " << cnt_sndlk << " СНДЛК\n";
			return;
		}
		if(cnt_sndlk % 500000 == 0){
			te = clock();
			if(te - tb >= timeout){
				vyvod();
				checkpoint();
				cout << "СНДЛК: " << cnt_sndlk << " КФ: " << cnt_bz
					<< " minl: "<<min_l
					<< " время: " << (te - t0) / CLOCKS_PER_SEC << " сек\n";;
				tb = te;
				min_l = 81;
			}
		}
		goto backtrack;
	}
	S[l] = (row[rc[l] >> 4] | col[rc[l] & 0xf]) ^ all;
try_to_advance:
	if(S[l]){
		X[l] = (S[l] & (-S[l]));
		update_downdate(l++);
		//steps_between++;
		goto enter_level_l;
	}
backtrack:
	if(--l >= 0){
		//if(l<20) goto end;
		//min_l=l<min_l?l:min_l;
		//steps_between++;
		update_downdate(l);
		S[l] ^= X[l];
		goto try_to_advance;
	}
end:
	t1 = clock();
	cout << "Найдено КФ[" << (lin + 1) << "]: " << cnt_bz << " время поиска: "
		<< double(t1 - t0) / CLOCKS_PER_SEC << " сек\n";
	cout << "Линейка " << (lin + 1) << " завершена, проверено " << cnt_sndlk << " СНДЛК\n";
}

void vyvod(){
	ofstream fout("output.txt", ios::binary);
	fout.write((char*)baza.data(), sizeof(kvadrat_b) * cnt_bz);
}

void checkpoint(){
	kvadrat tempk;
	char text[raz_kvb];
	for(int i = 0; i < por; i++){
		tempk[i * (por + 1)] = i;
		tempk[ (i + 1) * (por - 1)] = diag[lin][i];
	}
	for(int i = 0, t; i < l; i++){
		t = 0;
		if(X[i] & 0x300) t += 8;
		if(X[i] & 0xf0)  t += 4;
		if(X[i] & 0xcc)  t += 2;
		if(X[i] & 0x2aa) t += 1;
		tempk[(rc[i] >> 4) * por + (rc[i] & 0xf)] = t;
	}
	char* p = text;
	for(int i = 0; i < raz; i += por){
		*p++ = tempk[i] + '0';
		for(int j = 1; j < por; j++){
			*p++ = ' ';
			*p++ = tempk[i + j] + '0';
		}
		*p++ = '\n';
	}
	*p++ = '\n';
	ofstream fout("checkpoint.txt", ios::binary);
	fout.write(text, raz_kvb);
}
