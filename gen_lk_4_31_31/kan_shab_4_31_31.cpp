#include "main_const.h"
#include "kan_shab_4_31_31.h"

const int kan_shab::gray[] = {
	0,0,
	0,1,0,1,
	0,1,0,2,0,1,0,2,
	0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,3
};
const int kan_shab::ukaz_g[][2] = {0,2,2,4,6,8,14,16};
const int kan_shab::ukaz_gg[][2] = {0,2,2,2,4,2,6,3,9,1,10,1,11,2,13,1,14,2,16,2};
const int kan_shab::ukaz[][2] = {0,2,2,2,4,2,6,6,12,2,14,1,15,2,17,1,18,2,20,2};
const int kan_shab::ind_gg[] = {3,5,2,5,2,4,2,4,6,5,4,4,6,3,3,6,3,5};
const int kan_shab::per_f[por][por] = {
	0,1,2,3,4,0,1,2,3,4,
	0,2,1,4,3,4,3,2,1,0,
	1,0,4,3,2,0,2,1,4,3,
	1,4,0,2,3,4,2,3,0,1,
	2,0,3,4,1,3,4,1,2,0,
	2,3,0,1,4,1,0,3,2,4,
	3,2,4,1,0,2,0,4,1,3,
	3,4,2,0,1,2,4,0,3,1,
	4,1,3,2,0,3,1,4,0,2,
	4,3,1,0,2,1,3,0,4,2
};

int kan_shab::per[][(por << 1) + 1] = {
	0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,0,1,2,3,4,0,
	0,1,2,5,6,3,4,7,8,9,0,1,3,2,4,0,1,3,2,4,16,

	10,10,10,10,10,10,10,10,10,10,0,1,2,3,4,0,1,2,3,4,0,
	10,10,10,10,10,10,10,10,10,10,0,1,2,4,3,0,3,2,1,4,28,

	10,10,10,10,10,10,10,10,10,10,0,1,2,3,4,0,1,2,3,4,0,
	10,10,10,10,10,10,10,10,10,10,0,1,4,3,2,0,2,1,3,4,8,

	0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,0,1,2,3,4,0,
	0,1,2,3,6,7,4,5,8,9,0,1,3,2,4,0,1,3,2,4,16,
	0,1,6,7,2,3,4,5,8,9,0,1,4,2,3,0,2,3,1,4,12,
	0,1,6,7,4,5,2,3,8,9,0,1,2,4,3,0,3,2,1,4,28,
	0,1,4,5,6,7,2,3,8,9,0,1,3,4,2,0,3,1,2,4,20,
	0,1,4,5,2,3,6,7,8,9,0,1,4,3,2,0,2,1,3,4,8,

	0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,0,1,2,3,4,0,
	0,8,2,4,3,5,6,7,1,9,0,1,4,3,2,0,2,1,3,4,8,

	10,10,10,10,10,10,10,10,10,10,0,1,2,3,4,0,1,2,3,4,0,

	0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,0,1,2,3,4,0,
	0,1,2,3,6,7,4,5,8,9,0,1,3,2,4,0,1,3,2,4,16,

	10,10,10,10,10,10,10,10,10,10,0,1,2,3,4,0,1,2,3,4,0,

	10,10,10,10,10,10,10,10,10,10,0,1,2,3,4,0,1,2,3,4,0,
	10,10,10,10,10,10,10,10,10,10,0,1,2,4,3,0,3,2,1,4,28,

	10,10,10,10,10,10,10,10,10,10,0,1,2,3,4,0,1,2,3,4,0,
	10,10,10,10,10,10,10,10,10,10,0,1,4,3,2,0,2,1,3,4,8
};

void kan_shab::norm0(shablon& sh){
	static const int d = raz_m - por_m;
	int per_c[por_m], per_v[por_m], flag1 = 0x1f, flag2 = 0x1f;
	std::set<int> temps;
	std::map<int, int> nom;
	for(int i = 0, x, y, t, ii; i < por_m; i++){
		x = sh[i]; y = sh[d + i];
		if(!temps.insert(t = x < y ? (x << 4) | y : (y << 4) | x).second){
			per_c[0] = ii = nom[t]; per_c[1] = i;
			flag1 ^= (1 << ii) | (1 << i);
			per_v[x] = 0; per_v[y] = 4;
			flag2 ^= (1 << x) | (1 << y);
			break;
		}
		nom[t] = i;
	}
	for(unsigned long f = flag1, r, c = 2; f; f &= f - 1){
		_BitScanForward(&r, f);
		per_c[c++] = r;
	}
	shablon tempsh;
	for(int i = 0; i < raz_m; i += por_m) for(int j = 0; j < por_m; j++) tempsh[i + j] = sh[i + per_c[j]];
	unsigned long cv1, cv2, cv3;
	flag2 ^= (1 << (cv1 = tempsh[2])) | (1 << (cv2 = tempsh[d + 2]));
	per_v[cv1] = 1; per_v[cv2] = 3;
	_BitScanForward(&cv3, flag2);
	per_v[cv3] = 2;
	for(int i = 0; i < raz_m; i++) tempsh[i] = per_v[tempsh[i]];
	for(int i = 0; i < por_m; i++) if(tempsh[i] > tempsh[d + i]){
		std::swap(tempsh[i], tempsh[d + i]);
		std::swap(tempsh[por_m + i], tempsh[d - por_m + i]);
	}
	if(tempsh[3] != 1) for(int i = 0; i < raz_m; i += por_m) std::swap(tempsh[i + 3], tempsh[i + 4]);
	utochn(tempsh);
	norm_f_col(tempsh);
	sh = tempsh;
}

void kan_shab::norm1(shablon& sh){
	static const int d = raz_m - por_m, per[] = {0,4,1,2,3};
	int per_c[por_m], per_v[por_m], per_x[por_m + 1];
	std::map<int, int> nom;
	std::multimap<int, int> otobr;
	for(int i = 0, x, y; i < por_m; i++){
		x = sh[i]; y = sh[d + i];
		nom[x < y ? (x << 4) | y : (y << 4) | x] = i;
		otobr.insert(std::make_pair(x, y));
		otobr.insert(std::make_pair(y, x));
	}
	std::multimap<int, int>::iterator q;
	per_x[0] = sh[0];
	for(int i = 1, x = sh[0], y = sh[d], t; i <= por_m; i++){
		per_x[i] = y;
		if((t = (q = otobr.lower_bound(y))->second) != x){x = y; y = t;}
		else{x = y; y = (++q)->second;}
	}
	shablon tempsh;
	for(int i = 0, t, x, y; i < por_m; i++){
		per_v[per_x[i]] = t = per[i];
		x = per_x[t]; y = per_x[t + 1];
		per_c[i] = nom[x < y ? (x << 4) | y : (y << 4) | x];
	}
	for(int i = 0; i < raz_m ; i += por_m) for(int j = 0; j < por_m; j++) tempsh[i + j] = per_v[sh[i + per_c[j]]];
	for(int i = 0; i < por_m; i++) if(tempsh[i] > tempsh[d + i]){
		std::swap(tempsh[i], tempsh[d + i]);
		std::swap(tempsh[por_m + i], tempsh[d - por_m + i]);
	}
	norm_f_col(tempsh);
	sh = tempsh;
}

void kan_shab::norm_f_col(shablon& sh){
	int flag = ((1 << 0) | (1 << 4)) << 5, x, y;
	int temp = 1 << (x = sh[por_m]);
	flag |= flag & (temp << 5) ? temp : temp << 5;
	temp = 1 << (y = sh[raz_m - por]);
	flag |= flag & (temp << 5) ? temp : temp << 5;
	int pary[por_m] = {}, per_row[por] = {0,1,10,10,10,10,10,10,8,9};
	if(x > y) std::swap(per_row[1], per_row[8]);
	for(int i = 2, t; i < por - 2; i++){pary[t = sh[i * por_m]] <<= 4; pary[t] |= i;}
	for(int i = 0, c = 2, t; i < por_m; i++){
		t = 1 << i;
		if(flag & t) continue;
		if(flag & (t << 5)) per_row[c++] = pary[i];
		else{
			per_row[c++] = pary[i] >> 4;
			per_row[c++] = pary[i] & 0xf;
		}
	}
	shablon tempsh;
	for(int i = 0; i < por; i++) for(int j = 0; j < por_m; j++) tempsh[i * por_m + j] = sh[per_row[i] * por_m + j];
	sh = tempsh;
}

void kan_shab::trans1(const shablon& sh, shablon& frm){
	frm = sh;
	for(int i = 1; i < por - 1; i++) for(int j = 0; j < por_m; j++) if(!(frm[i * por_m + j] & 3)) frm[i * por_m + j] ^= 4;
	for(int i = 0; i < 2; i++) std::swap(frm[por_m + i], frm[raz_m - por + i]);
	utochn(frm);
	norm_f_col(frm);
}

void kan_shab::trans2(const shablon& sh, shablon& frm){
	frm = sh;
	for(int i = por_m; i < raz_m - por_m; i += por_m) std::swap(frm[i], frm[i + 1]);
	utochn(frm);
	norm_f_col(frm);
}

void kan_shab::init_formy0(shablon* formy, const shablon& sh){
	formy[0] = sh;
	trans1(sh, formy[1]);
	trans2(sh, formy[2]);
	trans1(formy[2], formy[3]);
}

void kan_shab::init_formy0(std::vector<shablon>& formy, const shablon& sh){
	init_formy0(formy.data(), sh);
	if(formy.size() == 8){
		shablon tempsh = sh;
		for(int i = 0; i < por_m; i++){
			std::swap(tempsh[i], tempsh[por_m + i]);
			std::swap(tempsh[raz_m - por_m + i], tempsh[raz_m - por + i]);
		}
		norm0(tempsh);
		init_formy0(formy.data() + 4, tempsh);
	}
}

void kan_shab::init_formy1(shablon* formy, const shablon& sh){
	shablon tempsh;
	const int *per_c, *per_v;
	static const int d = raz_m - por_m;
	for(int k = 0; k < por; k++){
		per_c = per_f[k];
		per_v = per_c + por_m;
		for(int i = 0; i < raz_m; i += por_m) for(int j = 0; j < por_m; j++)
			tempsh[i + j] = per_v[sh[i + per_c[j]]];
		for(int i = 0; i < por_m; i++) if(tempsh[i] > tempsh[d + i]){
			std::swap(tempsh[i], tempsh[d + i]);
			std::swap(tempsh[por_m + i], tempsh[d - por_m + i]);
		}
		norm_f_col(tempsh);
		formy[k] = tempsh;
	}
}

void kan_shab::init_formy1(std::vector<shablon>& formy, const shablon& sh){
	init_formy1(formy.data(), sh);
	if(formy.size() == (por << 1)){
		shablon tempsh = sh;
		for(int i = 0; i < por_m; i++){
			std::swap(tempsh[i], tempsh[por_m + i]);
			std::swap(tempsh[raz_m - por_m + i], tempsh[raz_m - por + i]);
		}
		norm1(tempsh);
		init_formy1(formy.data() + por, tempsh);
	}
}

bool kan_shab::is_kanon0(const shablon& sh, const int ch_form){
	std::vector<shablon> formy(ch_form);
	init_formy0(formy, sh);
	std::vector<int> spisok;
	int xx = sh[por_m], yy = sh[raz_m - por];
	int x = (xx << 3) | yy;
	for(int k = 0, y; k < ch_form; k++){
		y = (formy[k][por_m] << 3) | formy[k][raz_m - por];
		if(y < x) return false;
		if(x == y) spisok.push_back(k);
	}
	x = (xx << 1) + yy - (xx == 0 || xx == 3);
	int ind = ukaz[x][0], kol = ukaz[x][1], baz_gg = ukaz_gg[x][0], kol_gg = ukaz_gg[x][1];
	int ind_g = ukaz_g[kol_gg - 1][0], kol_g = ukaz_g[kol_gg - 1][1];
	for(int i = 0; i < kol; i++){
		for(int j = 0, t; j < kol_g; j++){
			if(!is_obrabotka0(formy, spisok, sh, per[ind + i])) return false;
			t = ind_gg[baz_gg + gray[ind_g + j]];
			std::swap(per[ind + i][t], per[ind + i][t + 1]);
		}
	}
	return true;
}

bool kan_shab::is_kanon1(const shablon& sh, const int ch_form){
	std::vector<shablon> formy(ch_form);
	init_formy1(formy, sh);
	std::vector<int> spisok;
	int xx = sh[por_m], yy = sh[raz_m - por];
	int x = (xx << 3) | yy;
	for(int k = 0, y; k < ch_form; k++){
		y = (formy[k][por_m] << 3) | formy[k][raz_m - por];
		if(y < x) return false;
		if(x == y) spisok.push_back(k);
	}
	x = (xx << 1) + yy - (xx == 0 || xx == 3);
	int baz_gg = ukaz_gg[x][0], kol_gg = ukaz_gg[x][1];
	int ind_g = ukaz_g[kol_gg - 1][0], kol_g = ukaz_g[kol_gg - 1][1];
	int perest[por] = {0,1,2,3,4,5,6,7,8,9};
	for(int j = 0, t; j < kol_g; j++){
		if(!is_obrabotka1(formy, spisok, sh, perest)) return false;
		t = ind_gg[baz_gg + gray[ind_g + j]];
		std::swap(perest[t], perest[t + 1]);
	}
	return true;
}

bool kan_shab::is_obrabotka0(const std::vector<shablon>& formy, const std::vector<int>& sp, const shablon& sh, const int* per_r){
	const int *per_c = per_r + por, *per_v = per_r + por + por_m, flag = per_r[por << 1];
	shablon tempsh;
	for(int k = 0; k < (int)sp.size(); k++){
		for(int i = 0; i < por; i++) for(int j = 0; j < por_m; j++)
			tempsh[i * por_m + j] = per_v[formy[sp[k]][per_r[i] * por_m + per_c[j]]];
		for(unsigned long f = flag, r; f; f &= f - 1){
			_BitScanForward(&r, f);
			std::swap(tempsh[r], tempsh[raz_m - por_m + r]);
			std::swap(tempsh[por_m + r], tempsh[raz_m - por + r]);
		}
		if(tempsh < sh) return false;
	}
	return true;
}

bool kan_shab::is_obrabotka1(const std::vector<shablon>& formy, const std::vector<int>& sp, const shablon& sh, const int* per_r){
	shablon tempsh;
	for(int k = 0; k < (int)sp.size(); k++){
		for(int i = 0; i < por; i++) for(int j = 0; j < por_m; j++)
			tempsh[i * por_m + j] = formy[sp[k]][per_r[i] * por_m + j];
		if(tempsh < sh) return false;
	}
	return true;
}

void kan_shab::kanon0(shablon& sh, const int ch_form){
	std::vector<shablon> formy(ch_form);
	norm0(sh);
	init_formy0(formy, sh);
	std::vector<int> spisok;
	int x = 055;
	for(int k = 0, y; k < ch_form; k++){
		y = (formy[k][por_m] << 3) | formy[k][raz_m - por];
		if(y < x){
			x = y;
			spisok.clear();
			spisok.push_back(k);
		}
		else if(x == y) spisok.push_back(k);
	}
	int xx = x >> 3, yy = x & 7;
	x = (xx << 1) + yy - (xx == 0 || xx == 3);
	int ind = ukaz[x][0], kol = ukaz[x][1];
	int baz_gg = ukaz_gg[x][0], kol_gg = ukaz_gg[x][1];
	int ind_g = ukaz_g[kol_gg - 1][0], kol_g = ukaz_g[kol_gg - 1][1];
	sh[por_m] = por;
	for(int i = 0; i < kol; i++){
		for(int j = 0, t; j < kol_g; j++){
			obrabotka0(formy, spisok, sh, per[ind + i]);
			t = ind_gg[baz_gg + gray[ind_g + j]];
			std::swap(per[ind + i][t], per[ind + i][t + 1]);
		}
	}
}

void kan_shab::obrabotka0(const std::vector<shablon>& formy, const std::vector<int>& sp, shablon& kf, const int* morf){
	const int *per_r = morf, *per_c = morf + por, *per_v = morf + por + por_m, flag = morf[por << 1];
	shablon tempsh;
	for(int k = 0; k < (int)sp.size(); k++){
		for(int i = 0; i < por; i++) for(int j = 0; j < por_m; j++)
			tempsh[i * por_m + j] = per_v[formy[sp[k]][per_r[i] * por_m + per_c[j]]];
		for(unsigned long f = flag, r; f; f &= f - 1){
			_BitScanForward(&r, f);
			std::swap(tempsh[r], tempsh[raz_m - por_m + r]);
			std::swap(tempsh[por_m + r], tempsh[raz_m - por + r]);
		}
		if(tempsh < kf) kf = tempsh;
	}
}

void kan_shab::kanon1(shablon& sh, const int ch_form){
	std::vector<shablon> formy(ch_form);
	norm1(sh);
	init_formy1(formy, sh);
	std::vector<int> spisok;
	int x = 055;
	for(int k = 0, y; k < ch_form; k++){
		y = (formy[k][por_m] << 3) | formy[k][raz_m - por];
		if(y < x){
			x = y;
			spisok.clear();
			spisok.push_back(k);
		}
		else if(x == y) spisok.push_back(k);
	}
	int xx = x >> 3, yy = x & 7;
	x = (xx << 1) + yy - (xx == 0 || xx == 3);
	int baz_gg = ukaz_gg[x][0], kol_gg = ukaz_gg[x][1];
	int ind_g = ukaz_g[kol_gg - 1][0], kol_g = ukaz_g[kol_gg - 1][1];
	int perest[por] = {0,1,2,3,4,5,6,7,8,9};
	sh[por_m] = por;
	for(int j = 0, t; j < kol_g; j++){
		obrabotka1(formy, spisok, sh, perest);
		t = ind_gg[baz_gg + gray[ind_g + j]];
		std::swap(perest[t], perest[t + 1]);
	}
}

void kan_shab::obrabotka1(const std::vector<shablon>& formy, const std::vector<int>& sp, shablon& kf, const int* per_r){
	shablon tempsh;
	for(int k = 0; k < (int)sp.size(); k++){
		for(int i = 0; i < por; i++) for(int j = 0; j < por_m; j++)
			tempsh[i * por_m + j] = formy[sp[k]][per_r[i] * por_m + j];
		if(tempsh < kf) kf = tempsh;
	}
}

bool kan_shab::is_super(const shablon& sh){
	super dt(sh);
	shablon tempsh;
	while(dt.next(tempsh)) if(tempsh < sh) return false;
	return true;
}

bool kan_shab::is_min_dv(const shablon& sh){
	shablon tempsh;
	get_dv(sh, tempsh);
	get_super(tempsh);
	return tempsh >= sh;
}

void kan_shab::get_dv(const shablon& sh, shablon& dvsh){
	int first_last[por] = {0,2,3,2,0,1,3,4,4,1};
	if(sh[raz_m - por_m + 1] == 3) std::swap(first_last[3], first_last[9]);
	for(int i = 0; i < por_m; i++){
		dvsh[i] = first_last[i];
		dvsh[raz_m - por_m + i] = first_last[por_m + i];
	}
	int temp[por_m] = {};
	for(int i = 0; i < por_m; i++) for(int k = 0, t = sh[por_m + i]; k < 2; t = sh[raz_m - por + i], k++)
		temp[t] = (temp[t] << 3) | i;
	for(int i = 0; i < por_m; i++){
		dvsh[por_m + i] = temp[i] >> 3;
		dvsh[raz_m - por + i] = temp[i] & 7;
	}
	for(int i = 2; i < por - 2; i++){
		for(int j = 0; j < por_m; j++) temp[sh[i * por_m + j]] = j;
		for(int j = 0; j < por_m; j++) dvsh[i * por_m + j] = temp[j];
	}
	super_class sup(dvsh);
	sup.get_init(dvsh);
}

int kan_shab::get_super(shablon& sh){
	int patt = kanon(sh);
	super dt(sh);
	shablon tempsh;
	while(dt.next(tempsh)) if(tempsh < sh) sh = tempsh;
	return patt;
}

super::super(const shablon& sh, bool kn):iter(0),cnt(0),ind(0x1f),kol(0),baz_sh(sh){
	if(kn) kan_shab::kanon(baz_sh);
	int temp[por_m] = {};
	for(int i = 0; i < por_m; i++){
		for(int k = 0, t = baz_sh[por_m + i]; k < 2; t = baz_sh[raz_m - por + i], k++){
			if(t == baz_sh[i] || t == baz_sh[raz_m - por_m + i]){
				if(temp[t]) temp[t] = (temp[t] << 3) | (i + 1);
				else temp[t] = i + 1;
			}
		}
	}
	stl[0] = 0;
	zapoln(temp);
	for(unsigned long f = ind & 0x1e, r, c = cnt; f; f &= f - 1){
		_BitScanForward(&r, f);
		stl[++c] = r;
	}
	if(cnt += por_m - kol - (stl[0] == 0)){
		ind = kan_shab::ukaz_g[cnt - 1][0];
		kol = kan_shab::ukaz_g[cnt - 1][1];
	}
	else ind = kol = 0;
}

bool super::next(shablon& sh){
	if(!kol) return false;
	for(int r = stl[kan_shab::gray[ind + iter] + 1]; r; r >>= 3){
		std::swap(baz_sh[por_m + (r & 7)], baz_sh[raz_m - por + (r & 7)]);
	}
	sh = baz_sh;
	if(++iter == kol){iter = 0; return false;}
	kan_shab::kanon(sh);
	return true;
}
