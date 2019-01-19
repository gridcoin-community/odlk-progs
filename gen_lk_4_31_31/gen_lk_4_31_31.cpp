#include <iostream>
#include <fstream>
#include <sstream>
#include "odlkcommon/common.h"
#include "main_const.h"
#include "kan_shab_4_31_31.h"
#include "izomorfizm.h"
#include "kanon.h"

using namespace std;

const int w_44323 = 10;
const int x_44323[w_44323] = {
	012020134,
	013030124,
	014040123,
	023030214,
	023131204,
	024040213,
	024141203,
	034040312,
	034141302,
	034242301
};
const int w_43423 = 12;
const int x_43423[w_43423] = {
	02314120403,
	02314130402,
	02413120403,
	02414130302,
	02423130401,
	02423140301,
	03413120402,
	03414120302,
	03423120401,
	03423140201,
	03424120301,
	03424130201
};
const int first_col[][por] = {
	0,0,1,2,2,3,3,4,1,4,
	0,0,1,1,2,3,3,4,2,4,
	0,0,1,1,2,2,3,4,3,4,
	0,0,1,1,2,2,3,3,4,4,
	0,1,0,1,2,3,3,4,2,4,
	0,1,0,1,2,2,3,4,3,4,
	0,1,0,1,2,2,3,3,4,4,
	0,2,0,1,1,2,3,4,3,4,
	0,2,0,1,1,2,3,3,4,4,
	0,3,0,1,1,2,2,3,4,4
};
const int str_sec_row0[] = {
	012,021,0102,0120,0123,0132,0201,0210,0213,0231,0312,0321,
	01002,01020,01023,01032,01200,01203,01230,01302,01320,02001,02010,02013,
	02031,02100,02103,02130,02301,02310,03012,03021,03102,03120,03201,03210
};
const int str_sec_row1[] = {
	0123,0132,0213,0231,0312,0321,01023,01032,01203,01230,01302,01320,
	02013,02031,02103,02130,02301,02310,03012,03021,03102,03120,03201,03210
};
const int etalon = (4 << 16) | (31 << 8) | 31;
const izomorfizm etalon_simm = {0,9,8,2,3,4,5,6,7,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0};

struct blok{int koord[12], size, tip;};

set<kvadrat> baza;
int pattern;
shablon baz_sh, super_sh;
kvadrat baz_kv;
vector<blok> blocks;

bool sogl_44323(const shablon& sh){
	if(sh[por_m] == 0 && sh[raz_m - por + 1] == 0) return false;
	if(sh[por_m + 1] == 0 && sh[raz_m - por] == 0) return false;
	if(sh[por_m + 2] == 1 && sh[raz_m - por + 3] == 1) return false;
	if(sh[por_m + 3] == 1 && sh[raz_m - por + 2] == 1) return false;
	if(sh[por_m + 4] == 2 && sh[por_m + 3] == 2) return false;
	if(sh[raz_m - por + 3] == 2 && sh[raz_m - por + 4] == 2) return false;
	if(sh[raz_m - por] == 4 && sh[por_m + 1] == 4) return false;
	if(sh[raz_m - por + 1] == 4 && sh[por_m] == 4) return false;
	if(sh[raz_m - por + 2] == 3 && sh[por_m + 4] == 3) return false;
	if(sh[raz_m - por + 4] == 3 && sh[por_m + 2] == 3) return false;
	return true;
}

bool sogl_43423(const shablon& sh){
	if(sh[por_m] == 0 && sh[raz_m - por + 1] == 0) return false;
	if(sh[por_m + 1] == 0 && sh[raz_m - por] == 0) return false;
	if(sh[por_m + 2] == 1 && sh[raz_m - por + 3] == 1) return false;
	if(sh[por_m + 3] == 1 && sh[raz_m - por + 2] == 1) return false;
	if(sh[por_m + 4] == 2 && sh[por_m + 3] == 2) return false;
	if(sh[raz_m - por + 3] == 2 && sh[raz_m - por + 4] == 2) return false;
	if(sh[raz_m - por] == 4 && sh[por_m + 2] == 4) return false;
	if(sh[raz_m - por + 2] == 4 && sh[por_m] == 4) return false;
	if(sh[raz_m - por + 1] == 3 && sh[por_m + 4] == 3) return false;
	if(sh[raz_m - por + 4] == 3 && sh[por_m + 1] == 3) return false;
	return true;
}

int bin_poisk(const int* mass, int dl, int x){
	int l = 0, r = dl, c, t;
	while(l < r){
		c = (l + r) >> 1;
		if((t = mass[c]) == x) return c;
		if(t < x) l = c + 1;
		else r = c;
	}
	return -1;
}

bool unget_name(const string& name, shablon& sh, int patt, int flag){
	for(unsigned long f = flag, r; f; f &= f - 1){
		psnip_intrin_BitScanForward(&r, f);
		swap(sh[por_m + r], sh[raz_m - por + r]);
	}
	if(patt == 2){if(!sogl_43423(sh)) return false;}
	else if(!sogl_44323(sh)) return false;
	for(int i = 4, t, ii; i < por - 1; i++){
		t = str_sec_row1[name[i] - 'A'];
		ii = (i - 2) * por_m;
		for(int j = 1, tt, e = sh[ii]; j < por_m; j++) sh[ii + j] = (tt = (t >> 3 * (j - 1)) & 7 , tt >= e ? tt + 1 : tt);
	}
	for(unsigned long j = 1, r, f, d = raz_m - por - por_m; j < por_m; j++){
		f = 0;
		for(int i = 0, t; i < por; i++){
			if(i == por - 3) continue;
			t = 1 << sh[i * por_m + j];
			if(f & t) return false;
			if(f & (t << 5)) f |= t;
			else f |= t << 5;
		}
		psnip_intrin_BitScanForward(&r, ~f);
		sh[d + j] = (unsigned char)r;
	}
	for(int i = 0, d = raz_m - por - por_m, t, f = 0; i < por_m; i++){
		t = 1 << sh[d + i];
		if(f & t) return false;
		f |= t;
	}
	pattern = patt;
	return true;
}

bool unget_name(const string& name, shablon& sh, int patt, int flag, int det, int par, int row){
	static const int first_last[por] = {0,0,1,1,2,4,4,3,2,3};
	for(int i = 0; i < por_m; i++){
		sh[i] = first_last[i];
		sh[raz_m - por_m + i] = first_last[por_m + i];
	}
	if(patt == 2) swap(sh[raz_m - por_m + 1], sh[raz_m - por_m + 2]);
	for(int i = 1; i < por - 1; i++) sh[i * por_m] = first_col[det][i];
	det = (first_col[det][1] << 3) | first_col[det][por - 2];
	par = patt ? x_43423[par] : x_44323[par];
	row = patt ? str_sec_row1[row] : str_sec_row0[row];
	int pary[por_m], per[por_m];
	if(patt){
		for(int i = 0; i < por_m; i++) pary[i] = (par >> 6 * i) & 0x3f;
		if((per[0] = bin_poisk(pary, por_m, det)) < 0) return false;
		for(int i = 1, t, e = per[0]; i < por_m; i++) per[i] = (t = (row >> 3 * (i - 1)) & 7 , t >= e ? t + 1 : t);
		for(int i = 1, t; i < por_m; i++){
			sh[por_m + i] = (t = pary[per[i]]) >> 3;
			sh[raz_m - por + i] = t & 7;
		}
	}
	else{
		pary[0] = par & 0x3f;
		for(int i = 1; i < por_m; i++) pary[i] = (par >> 6 * (i - 1)) & 0x3f;
		if(pary[0] == det) per[0] = 0;
		else if(!(per[0] = 1 + bin_poisk(pary + 2, por_m - 2, det))) return false;
		int x = row ^ 03333; x = 04444 & (x - 01111) & ~x;
		if(!per[0] == !x) return false; 
		for(int i = 1; i < por_m; i++) per[i] = (row >> 3 * (i - 1)) & 7;
		if(per[0]) for(int i = 1; i < por_m; i++) if(per[i] >= per[0]) per[i]++;
		for(int i = 1, t; i < por_m; i++){
			sh[por_m + i] = (t = pary[per[i] + 1]) >> 3;
			sh[raz_m - por + i] = t & 7;
		}
	}
	return unget_name(name, sh, patt, flag);
}

bool unget_name(string& name, shablon& sh){
	static const int dets[] = {0,3,4,6};
	if(name.size() != por - 1) return false;
	for(int i = 0; i < por - 1; i++) name[i] = toupper(name[i]);
	int temp = name[0] - 'A';
	if(temp < 0 || temp >= 12 && temp < 16 || temp >= 26) return false;
	int patt = (temp >= 8) + (temp >= 16), nom_det, nom_row = 0, nom_par;
	nom_det = patt == 2 ? temp - 16 : dets[temp & 3];
	if(patt == 0 && temp >= 4) nom_row = 12;
	temp = name[1] - 'A';
	if(temp < 0 || temp >= 16) return false;
	int flag = temp << 1;
	temp = name[2] - 'A';
	if(temp < 0 || temp >= (patt ? 12 : 10)) return false;
	nom_par = temp;
	temp = name[3] - 'A';
	if(temp < 0 || temp >= 24) return false;
	if(patt == 0 && nom_row == 0 && temp >= 12) return false;
	nom_row += temp;
	for(int i = 4; i < por - 1; i++) if((temp = name[i] - 'A') < 0 || temp >= 24) return false;
	return unget_name(name, sh, patt, flag, nom_det, nom_par, nom_row);
}

bool is_correct(string& name){
	if(!unget_name(name, super_sh)) return false;
	if(!kan_shab::is_kanon(super_sh)) return false;
	if(!kan_shab::is_super(super_sh)) return false;
	if(!kan_shab::is_min_dv(super_sh)) return false;
	return true;
}

bool in_family(const izomorfizm& avtom){
	kvadrat tempk = baz_kv;
	static const izomorfizm et = ~get_kan(etalon_simm);
	(get_kan(avtom) * et)(tempk);
	shablon sh;
	for(int i = 0; i < por; i++) for(int j = 0, v; j < por_m; j++){
		v = tempk[i * por + j];
		sh[i * por_m + j] = v < por_m ? v : por - 1 - v;
	}
	int new_pattern = kan_shab::get_kan_sh_sem(sh);
	if(new_pattern < pattern) return false;
	if(new_pattern > pattern) return true;
	if(sh < super_sh) return false;
	return true;
}

void vizit(set<kvadrat>& super_kl){
	kvadrat kf;
	list<izomorfizm> avtom;
	Kanonizator_lk::kanon(baz_kv, avtom, &kf);
	if(!super_kl.insert(kf).second) return;
	for(auto w = avtom.begin(); w != avtom.end(); w++){
		if(*w == etalon_simm) continue;
		if(get_symm(*w) == etalon && !in_family(*w)) return;
	}
	baza.insert(kf);
}

void get_baz_kv(){
	for(auto q = blocks.begin(); q != blocks.end(); q++){
		for(int i = 0; i < q->size; i++) baz_kv[q->koord[i]] = i & 1 ? por - 1 - q->tip : q->tip;
		q->tip ^= por - 1 - q->tip;
	}
}

void get_blocks(){
	blocks.clear();
	int col_p[por][por_m] = {}, row_p[por][por_m] = {}, col[por][por], row[por][por];
	kvadrat tempk;
	for(int i = 0; i < por_m; i++){
		tempk[i] = baz_sh[i]; tempk[por - 1 - i] = baz_sh[raz_m - por_m + i];
		tempk[por + i] = baz_sh[por_m + i]; tempk[2 * por - 1 - i] = baz_sh[raz_m - por + i];
	}
	for(int i = 2; i < por - 2; i++) for(int j = 0; j < por_m; j++)
		tempk[i * por + j] = tempk[(i + 1) * por - 1 - j] = baz_sh[i * por_m + j];
	for(int i = 0; i < por; i++){
		tempk[raz - por + i] = tempk[por - 1 - i];
		tempk[raz - 2 * por + i] = tempk[2 * por - 1 - i];
	}
	for(int i = 0; i < por; i++) for(int j = 0, t; j < por; j++){
		t = tempk[i * por + j]; row_p[i][t] = (row_p[i][t] << 4) | j;
		t = tempk[j * por + i]; col_p[i][t] = (col_p[i][t] << 4) | j;
	}
	for(int i = 0; i < por; i++) for(int j = 0, p, v; j < por_m; j++){
		p = row_p[i][j] >> 4; v = row_p[i][j] & 0xf; row[i][p] = v; row[i][v] = p;
		p = col_p[i][j] >> 4; v = col_p[i][j] & 0xf; col[i][p] = v; col[i][v] = p;
	}
	set<int> temps;
	for(int i = 0; i < raz; i++) temps.insert(i);
	int nach, tek, x, y, c;
	blok tempb;
	for(auto q = temps.begin(); !temps.empty(); q = temps.begin()){
		nach = tek = *q; x = tek / por; y = tek % por; c = 0;
		while(true){
			tempb.koord[c++] = tek;
			temps.erase(tek);
			if(c & 1) y = row[x][y]; else x = col[y][x];
			tek = x * por + y;
			if(tek != nach) continue;
			x = (tek = tempb.koord[c - 1]) / por;
			if(x != 0 && x != 1 && x != por - 2 && x != por - 1) break;
			x = por - 1 - x;
			y = por - 1 - tek % por;
			tek = x * por + y;
			if(temps.find(tek) == temps.end()) break;
			nach = tek;
		}
		tempb.size = c; tempb.tip = tempk[nach];
		blocks.push_back(tempb);
	}
}

void rabota(){
	baz_sh = super_sh;
	super dt(baz_sh);
	set<kvadrat> super_kl;
	set<shablon> temps;
	do{
		if(!temps.insert(baz_sh).second) continue;
		get_blocks();
		get_baz_kv();
		vizit(super_kl);
		for(unsigned long i = 1, r, t, b; i < (1u << (blocks.size() - por_m)); i++){
			psnip_intrin_BitScanForward(&r, i);
			t = blocks[b = r + por_m].tip;
			for(int j = 0; j < blocks[b].size; j++) baz_kv[blocks[b].koord[j]] ^= t;
			vizit(super_kl);
		}
	}while(dt.next(baz_sh));
}

int main(int argc, char* argv[]){
	setlocale(LC_CTYPE, "rus");
	cout << "Generator of Latain Squares with (4,31,31) symmetry\n\n";
	if(argc < 2){cerr << "Missing command line argument\n\n"; return 1;}
	string name(argv[1]);
	if(!is_correct(name)){cerr << "Incorrect family name\n\n"; return 2;}
	for(int i = 0; i < ch_kl; i++) nom_strukt.insert(make_pair(string(strukt[i]), i));
	cout << "Family: " << name;
	rabota();
	if(baza.empty()){cout << " has no Latin Squares\n\n"; return 0;}
	else cout << " has " << baza.size() << " Latin Squares\n";
	ostringstream sout;
	sout << "family_4_31_31_" << name << ".txt";
	ofstream fout(sout.str(), ios::binary);
	for(auto q = baza.begin(); q != baza.end(); q++) out_kvadrat(fout, *q);
	cout << "written to file " << sout.str() << endl << endl;
}
