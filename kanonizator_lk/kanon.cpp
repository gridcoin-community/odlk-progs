#include "kanon.h"
#include "massivy.h"

#include "psnip/builtin/builtin.h"

using namespace std;

static inline void bit_set( unsigned long *a, unsigned long b )
{
       *a |= ((unsigned long) 1) << b;
}

static inline void bit_reset( unsigned long *a, unsigned long b )
{
       *a &= ~(((unsigned long) 1) << b);
}

void Kanonizator_lk::kanon(const kvadrat& lk, kvadrat& kf){
	int lin = init(lk), ind, kol;
	ind = ukaz[lin][0];
	kol = ukaz[lin][1];
	for(int i = 0; i < por; i++){
		kf[i] = i;
		kf[por + i] = line[lin][i];
		kf[i * por] = i;
	}
	kf[(por << 1) + 1] = por; 
	for(int i = 0; i < kol; i++) obrabotka(trans[ind + i], kf);
}

int Kanonizator_lk::get_type(morfizm& per){
	unsigned long ret = 0;
	unsigned long r;
	for(unsigned long i = 0, c, flag = 0x3ff; flag; psnip_intrin_BitScanForward(&i, flag)){
		bit_reset(&flag, i);
		c = 1;
		for(int j = per[i]; j != i; j = per[j]){
			bit_reset(&flag, j);
			c++;
		}
		if(c == 4) bit_set(&ret, 4 + psnip_intrin_bittest(&ret, 4));
		else bit_set(&ret, c);
	}
	psnip_intrin_BitScanReverse(&r, ret);
	bit_reset(&ret, r);
	switch(r){
		case 10: ret = 11; break;
		case 8: ret = 6; break;
		case 7: ret = 8; break;
		case 6: psnip_intrin_BitScanForward(&r, ret); ret = r == 2 ? 3 : 9; break;
		case 5: if(ret){psnip_intrin_BitScanReverse(&r, ret); ret = r == 3 ? 4 : 5;} else ret = 10; break;
		case 4: psnip_intrin_BitScanForward(&r, ret); ret = r == 2 ? 1 : 7; break;
		case 3: ret = 2; break;
		case 2: ret = 0;
	}
	return ret;
}

void Kanonizator_lk::get_invar(const kvadrat* sec_lk[], invariant& invar, sootv& nom){
	morfizm temp, kras;
	invariant_1 temp_1;
	array<pair<invariant_1, int>, por> temp_2;
	array<pair<invariant_2, int>, 3> temp_3;
	for(int k = 0; k < 3; k++){
		for(int i = 0; i < por; i++){
			for(int j = 0; j < por; j++) kras[(*sec_lk[k])[i * por + j]] = j;
			for(int j = 0, c = 0; j < por; j++){
				if(j == i) continue;
				for(int l = 0; l < por; l++) temp[l] = kras[(*sec_lk[k])[j * por + l]];
				temp_1[c++] = (get_type(temp) << 4) | j;
			}
			sort(temp_1.begin(), temp_1.end());
			for(int j = 0, t; j < por - 1; j++){
				temp_1[j] = (t = temp_1[j]) >> 4;
				nom.a[k][i][j] = t & 0xf;
			}
			temp_2[i] = make_pair(temp_1, i);
		}
		sort(temp_2.begin(), temp_2.end());
		for(int i = 0; i < por; i++){
			invar[k][i] = temp_2[i].first;
			nom.b[k][i] = temp_2[i].second;
		}
		temp_3[k] = make_pair(invar[k], k);
	}
	sort(temp_3.begin(), temp_3.end());
	for(int k = 0; k < 3; k++){
		invar[k] = temp_3[k].first;
		nom.c[k] = temp_3[k].second;
	}
}

void Kanonizator_lk::kratn(const invariant& invar, sootv& krat){
	invariant_2 temp_1 = invar[0];
	krat.c[0] = 1;
	int count = 0;
	for(int i = 1; i < 3; i++){
		if(temp_1 == invar[i]) krat.c[count]++;
		else{
			temp_1 = invar[i];
			krat.c[++count] = (i << 4) | 1;
		}
	}
	if(++count < 3) krat.c[count] = 0;
	invariant_1 temp_2;
	for(int k = 0; k < 3; k++){
		temp_2 = invar[k][0];
		krat.b[k][0] = 1;
		count = 0;
		for(int i = 1; i < por; i++){
			if(temp_2 == invar[k][i]) krat.b[k][count]++;
			else{
				temp_2 = invar[k][i];
				krat.b[k][++count] = (i << 4) | 1;
			}
		}
		if(++count < por) krat.b[k][count] = 0;
	}
	unsigned char temp_3;
	for(int k = 0; k < 3; k++) for(int i = 0; i < por; i++){
		temp_3 = invar[k][i][0];
		krat.a[k][i][0] = 1;
		count = 0;
		for(int j = 1; j < por - 1; j++){
			if(temp_3 == invar[k][i][j]) krat.a[k][i][count]++;
			else{
				temp_3 = invar[k][i][j];
				krat.a[k][i][++count] = (j << 4) | 1;
			}
		}
		if(++count < por - 1) krat.a[k][i][count] = 0;
	}
}

int Kanonizator_lk::analiz(const invariant& invar){
	sootv krat;
	kratn(invar, krat);
	int ves_s = 0xfffff, best_k, best_i, best_j, best_b;
	for(int k = 0, k_p, t, t1, ves_r, b_i, bb_j, bb_b; k < 3 && (t = krat.c[k]); k++){
		k_p = t >> 4;
		t1 = t & 0xf;
		ves_r = 0xfffff;
		for(int i = 0, i_p, tt, t2, ves_b, b_j, b_b; i < por && (tt = krat.b[k_p][i]); i++){
			i_p = tt >> 4;
			t2 = tt & 0xf;
			ves_b = 0xffff;
			for(int j = 0, j_p, ttt, t3, vv, bb; j < (por - 1) && (ttt = krat.a[k_p][i_p][j]); j++){
				j_p = ttt >> 4;
				t3 = ttt & 0xf;
				vv = ves[bb = invar[k_p][i_p][j_p]];
				if((t3 *= vv) < ves_b){
					ves_b = t3;
					b_j = ttt;
					b_b = bb;
				}
			}
			if((t2 *= ves_b) < ves_r){
				ves_r = t2;
				b_i = tt;
				bb_j = b_j;
				bb_b = b_b;
			}
		}
		if((t1 *= ves_r) < ves_s){
			ves_s = t1;
			best_k = t;
			best_i = b_i;
			best_j = bb_j;
			best_b = bb_b;
		}
	}
	return (best_k << 24) | (best_i << 16) | (best_j << 8) | best_b;
}

void Kanonizator_lk::get_cikly(const kvadrat& lk, int perv, int vtor, int lin, morfizm& cikly){
	morfizm kras, per, temp;
	for(int i = 0; i < por; i++) kras[lk[perv * por + i]] = i;
	for(int i = 0; i < por; i++) per[i] = kras[lk[vtor * por + i]];
	int count = ckl[lin];
	f_ind index = ind[lin];
	for(unsigned long i = 0, c, cc, flag = 0x3ff; flag; psnip_intrin_BitScanForward(&i, flag)){
		bit_reset(&flag, i);
		temp[(c = 0)++] = (unsigned char)i;
		for(int j = per[i]; j != i; j = per[j]){
			bit_reset(&flag, j);
			temp[c++] = j;
		}
		cc = index(c, count);
		for(int k = 0; k < (int)c; k++) cikly[cc + k] = temp[k];
	}
}

void Kanonizator_lk::postr_formu(const kvadrat& lk, const morfizm& per, int perv, int& count){
	kvadrat tempk;
	morfizm tempm;
	for(int i = 0; i < raz; i += por) for(int j = 0; j < por; j++) tempk[i + j] = lk[i + per[j]];
	for(int i = 0; i < por; i++) tempm[tempk[perv * por + i]] = i;
	for(int i = 0; i < raz; i++) tempk[i] = tempm[tempk[i]];
	for(int i = 0; i < por; i++) tempm[tempk[i * por]] = i;
	for(int i = 0; i < por; i++) for(int j = 0; j < por; j++) formy[count][i * por + j] = tempk[tempm[i] * por + j];
	count++;
}

void Kanonizator_lk::postr_sdvigi(const kvadrat& lk, int perv, int vtor, int lin, int& count){
	morfizm cikly, tempm;
	get_cikly(lk, perv, vtor, lin, cikly);
	int ind = ind_sdv[lin], kol = ch_sdv[lin];
	for(int i = 0; i < kol; i++){
		for(int j = 0; j < por; j++) tempm[j] = cikly[sdvig[ind + i][j]];
		postr_formu(lk, tempm, perv, count);
	}
}

void Kanonizator_lk::get_formy(const kvadrat* sec_lk[], const sootv& nom, int best){
	int sek, perv, vtor, lin, krat_s, krat_r, krat_b;
	sek = best >> 28; krat_s = (best >> 24) & 0xf;
	perv = (best >> 20) & 0xf; krat_r = (best >> 16) & 0xf;
	vtor = (best >> 12) & 0xf; krat_b = (best >> 8) & 0xf;
	lin = best & 0xff;
	ch_form = krat_s * krat_r * krat_b * (ch_sdv[lin] << 1);
	formy.resize(ch_form);
	for(int i = 0; i < 2; i++) spisok[i].resize(ch_form);
	kvadrat inv_lk[3];
	morfizm tempm;
	for(int k = 0; k < 3; k++) for(int i = 0; i < raz; i += por){
		for(int j = 0; j < por; j++) tempm[(*sec_lk[k])[i + j]] = j;
		for(int j = 0; j < por; j++) inv_lk[k][i + j] = tempm[j];
	}
	for(int k = 0, c = 0, sek_p; k < krat_s; k++){
		sek_p = nom.c[sek + k];
		for(int i = 0, perv_p; i < krat_r; i++){
			perv_p = nom.b[sek_p][perv + i];
			for(int j = 0, vtor_p; j < krat_b; j++){
				vtor_p = nom.a[sek_p][perv_p][vtor + j];
				postr_sdvigi(*sec_lk[sek_p], perv_p, vtor_p, lin, c);
				postr_sdvigi(inv_lk[sek_p], perv_p, vtor_p, lin, c);
			}
		}
	}
}

int Kanonizator_lk::init(const kvadrat& lk){
	kvadrat t_lk, ic_lk;
	for(int i = 0; i < por; i++) for(int j = 0; j < por; j++) t_lk[i * por + j] = lk[j * por + i];
	for(int j = 0; j < por; j++) for(int i = 0; i < por; i++) ic_lk[j + lk[j + i * por] * por] = i;
	const kvadrat* sec_lk[3] = {&lk, &t_lk, &ic_lk};
	invariant invar;
	sootv nom;
	get_invar(sec_lk, invar, nom);
	int best = analiz(invar);
	get_formy(sec_lk, nom, best);
	return best & 0xff;
}

void Kanonizator_lk::obrabotka(const int* perest, kvadrat& kf){
	int x, minz, index = 0, count[2];
	const int* ob_perest = perest + por;
	for(int i = 0; i < ch_form; i++) spisok[0][i] = i;
	count[0] = ch_form;
	for(int i = 2, ii; i < por - 1; i++){
		ii = perest[i] * por;
		for(int j = 1, ij; j < por - 1; j++){
			ij = ii + perest[j];
			minz = ob_perest[formy[spisok[index][0]][ij]];
			spisok[index ^ 1][0] = spisok[index][0];
			count[index ^ 1] = 1;
			for(int k = 1; k < count[index]; k++){
				if(minz > (x = ob_perest[formy[spisok[index][k]][ij]])){
					minz = x;
					spisok[index ^ 1][0] = spisok[index][k];
					count[index ^ 1] = 1;
				}
				else if(minz == x) spisok[index ^ 1][count[index ^ 1]++] = spisok[index][k];
			}
			if((x = kf[i * por + j]) < minz) return;
			index ^= 1;
			if(x > minz){
				count[index]--;
				for(int i = 2; i < por; i++) for(int j = 1; j < por; j++)
					kf[i * por + j] = ob_perest[formy[spisok[index][count[index]]][perest[i] * por + perest[j]]];
				if(!count[index]) return;
			}
		}
	}
}
