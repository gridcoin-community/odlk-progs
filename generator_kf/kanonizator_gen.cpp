#include "kanonizator_gen.h"
#include "massivy_gen.h"

int kanonizator::chform;
kvadrat kanonizator::formy[];

bool kanonizator::operator()(const int X[]){
	for(int i = 0; i < por; i++){
		formy[0][i * (por + 1)] = i;
		formy[0][ (i + 1) * (por - 1)] = diag[lin][i];
	}
	for(int i = 0, t; i < n; i++){
		t = 0;
		if(X[i] & 0x300) t += 8;
		if(X[i] & 0xf0)  t += 4;
		if(X[i] & 0xcc)  t += 2;
		if(X[i] & 0x2aa) t += 1;
		formy[0][(rc[i] >> 4) * por + (rc[i] & 0xf)] = t;
	}
	if(lin < ch_klass2){
		chform = 2;
		simmetr();
		if(formy[1] < formy[0]) return false;
	}
	else chform = 1;
	int kol = ukaz[lin] & 0x1f, ind = ukaz[lin] >> 5;
	for(int i = 0; i < kol; i++)
		if(!obrabotka(&trans_tabl[ind + i][0], &trans_tabl[ind + i][por])) return false;
	return true;
}

bool kanonizator::obrabotka(const int* perest, const int* ob_perest){
	int x, minz, index = 0, count[2], spisok[2][max_chform];
	for(int i = 0; i < chform; i++) spisok[0][i] = i;
	count[0] = chform;
	for(int i = 0; i < por - 1; i++) for(int j = 0; j < por - 1; j++){
		if(j == i || j == por - 1 - i) continue;
		minz = formy[0][i * por + j];
		count[index ^ 1] = 0;
		for(int k = 0; k < count[index]; k++){
			if(minz > (x = ob_perest[formy[spisok[index][k]][perest[i] * por + perest[j]]])){
				return false;
			}
			else if(minz == x) spisok[index ^ 1][count[index ^ 1]++] = spisok[index][k];
		}
		if(!count[index ^= 1]) return true;
	}
	return true;
}