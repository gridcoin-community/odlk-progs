#include "prov_blk_trans.h"

class kanonizator{
	typedef std::array<unsigned char,por> morfizm;

	static const int raz_hash_tabl = 933188;
	static const int ch_klass = 67;
	static const int ch_klass2 = 56;
	static const int raz_trans_tabl = 114;
	static const int trans_tabl[raz_trans_tabl][por << 1];
	static const int ukaz[ch_klass];
	static const int dop_trans[ch_klass2][por];

	static int hash_tabl[raz_hash_tabl];

	friend bool init();
	static unsigned long hash_f(const morfizm& a);
	static void perebor(kvadrat& kf, int klass);
	static void obrabotka(const int* perest, const int* ob_perest, const kvadrat formy[], int chform, kvadrat& kf);

	static void dop_init(kvadrat& kf, kvadrat& dlk, int klass){
		kvadrat tempk = kf;
		for(int i = 0; i < por; i++) for(int j = 0; j < (por >> 1); j++)
			std::swap(tempk[i * por + j], tempk[i * por + por - 1 - j]);
		for(int i = 0; i < por; i++) for(int j = 0; j < por; j++)
			dlk[i * por + j] = tempk[dop_trans[klass][i] * por + dop_trans[klass][j]];
		morfizm perest;
		for(int i = 0, j = 0; i < por; j += por + 1, i++) perest[dlk[j]] = i;
		for(int i = 0; i < raz; i++) dlk[i] = perest[dlk[i]];
		if(dlk < kf) dlk.swap(kf);
	}
	
	static void normaliz(kvadrat& dlk){
		morfizm perest;
		unsigned flag = 1;
		for(int i = 0, j = 0, t; i < por; j += por + 1, i++){
			perest[t = dlk[j]] = i;
			flag &= t == i;
		}
		if(!flag) for(int i = 0; i < raz; i++) dlk[i] = perest[dlk[i]];
	}

	static void preobraz(kvadrat& dlk, kvadrat & kf, int izo){
		morfizm perest;
		if(izo & 0x200000) for(int i = 0; i < por - 1; i++) for(int j = i + 1; j < por; j++)
			std::swap(dlk[i * por + j], dlk[j * por + i]);
		if(izo & 0x100000) for(int i = 0; i < por; i++) for(int j = 0; j < (por >> 1); j++)
			std::swap(dlk[i * por + j], dlk[i * por + por - 1 - j]);
		for(int i = 0, t; i < (por >> 1); i++){
			t = (izo >> (i << 2)) & 0xf;
			perest[i] = t;
			perest[por - 1 - i] = por - 1 - t;
		}
		for(int i = 0; i < por; i++) for(int j = 0; j < por; j++) kf[i * por + j] = dlk[perest[i] * por + perest[j]];
		normaliz(kf);
	}

	kanonizator(){}

public:

	static int kanon(kvadrat& dlk, kvadrat& kf);
};

