#include "kanonizator.h"
#include "massivy.h"

int kanonizator::hash_tabl[];

unsigned long kanonizator::hash_f(const kanonizator::morfizm& a){
	unsigned long besp = 0, temp, temp2, mod;
	for(int i = 0, t; i < por; i++){
		besp |= ((t = a[i]) & 7) << ((i << 1) + i);
		switch(t){
			case 0: besp |= 0x40000000; break;
			case 1: besp |= 0x80000000; break;
			case 8: besp &= 0xbfffffff; break;
			case 9: besp &= 0x7fffffff; break;
		}
	}
	if(!((temp = hash_tabl[(besp + (besp >> 13)) & 0x7ffff]) & 0x80000000))
		return temp;
	if(!((temp2 = hash_tabl[temp &= 0xfffff]) & 0x40000000))
		return hash_tabl[++temp + ((besp & temp2 & 0xfff) != 0)];
	mod = temp2 & 0x1f;
	temp2 = (temp2 >> 5) & 0x1f;
	return hash_tabl[++temp + ((besp ^ (besp >> temp2)) & mod)];
}

int kanonizator::kanon(kvadrat& dlk, kvadrat& kf){
	normaliz(dlk);
	morfizm perest;
	for(int i = 0, j = por - 1; i < por; j += por - 1, i++) perest[i] = dlk[j];
	int izo = hash_f(perest);
	preobraz(dlk, kf, izo);
	int lin = izo >> 22;
	perebor(kf, lin);
	return lin;
}

void kanonizator::perebor(kvadrat& kf, int klass){
	kvadrat formy[2];
	int chform = 2, temp = ukaz[klass];
	if(klass < ch_klass2) dop_init(kf, formy[1], klass);
	else chform = 1;
	if(!temp) return;
	formy[0] = kf;
	int kol = temp & 0x1f, ind = temp >> 5;
	for(int i = 0; i < kol; i++) obrabotka(&trans_tabl[ind + i][0], &trans_tabl[ind + i][por], formy, chform, kf);
}

void kanonizator::obrabotka(const int* perest, const int* ob_perest, const kvadrat formy[], int chform, kvadrat& kf){
	int x, minz, index = 0, count[2], spisok[2][2];
	for(int i = 0; i < chform; i++) spisok[0][i] = i;
	count[0] = chform;
	for(int i = 0; i < por - 1; i++) for(int j = 0; j < por - 1; j++){
		if(j == i || j == por - 1 - i) continue;
		minz = ob_perest[formy[spisok[index][0]][perest[i] * por + perest[j]]];
		spisok[index ^ 1][0] = spisok[index][0];
		count[index ^ 1] = 1;
		for(int k = 1; k < count[index]; k++){
			if(minz > (x = ob_perest[formy[spisok[index][k]][perest[i] * por + perest[j]]])){
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
			for(int i = 0; i < por; i++) for(int j = 0; j < por; j++){
				if(j == i || j == por - 1 - i) continue;
				kf[i * por + j] = ob_perest[formy[spisok[index][count[index]]][perest[i] * por + perest[j]]];
			}
			if(!count[index]) return;
		}
	}
}