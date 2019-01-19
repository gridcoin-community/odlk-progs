#include <iostream>
#include <fstream>
#include <list>
#include <set>
#include <vector>
#include <array>
#include <ctime>
#include <algorithm>
#include "odlkcommon/common.h"

using std::array;
using std::vector;
using std::list;
using std::set;
class Kanonizator_lk {
	typedef array<unsigned char, por> morfizm;
	typedef array<unsigned char, por - 1> invariant_1;
	typedef array<invariant_1, por> invariant_2;
	typedef array<invariant_2, 3> invariant;
	typedef int (*f_ind)(int, int&);

	struct sootv{
		invariant a;
		morfizm b[3];
		int c[3];
	};

	static const int ch_lin = 12;
	static const int ch_trans = 550;
	static const int ch_sdvig = 63;
	static const int line[ch_lin][por];
	static const int ves[ch_lin];
	static const int ch_sdv[ch_lin];
	static const int ind_sdv[ch_lin];
	static const int sdvig[ch_sdvig][por];
	static const int ckl[ch_lin];
	static const f_ind ind[ch_lin];
	static const int ukaz[ch_lin][2];
	static const int trans[ch_trans][por << 1];

	vector<kvadrat> formy;
	vector<int> spisok[2];
	int ch_form;

	int init(const kvadrat& lk);
	void get_invar(const kvadrat* sec_lk[], invariant& invar, sootv& nom);
	int get_type(morfizm& per);
	int analiz(const invariant& invar);
	void get_formy(const kvadrat* sec_lk[], const sootv& nom, int best);
	void get_cikly(const kvadrat& lk, int perv, int vtor, int lin, morfizm& cikly);
	void postr_sdvigi(const kvadrat& lk, int perv, int vtor, int lin, int& count);
	void postr_formu(const kvadrat& lk, const morfizm& per, int perv, int& count);
	void kratn(const invariant& invar, sootv& krat);
	void obrabotka(const int* perest, kvadrat& kf);

	static int ind_1(int dl, int& count){
		int ret = count;
		count += 2;
		return ret;
	}

	static int ind_2(int dl, int& count){
		int ret;
		if(dl == 2){ret = count & 0xf; count += 2;}
		else ret = count >> 4;
		return ret;
	}

	static int ind_3(int dl, int& count){
		int ret;
		if(dl == 2){ret = count & 0xf; count += 2;}
		else{ret = count >> 4; count += 0x30;}
		return ret;
	}

	static int ind_5(int dl, int& count){
		if(dl == 2) return 0;
		if(dl == 3) return 2;
		return 5;
	}

	static int ind_6(int dl, int& count){
		int ret;
		if(dl == 2) ret = 0;
		else{ret = count >> 4; count += 0x40;}
		return ret;
	}

	static int ind_7(int dl, int& count){
		if(dl == 2) return 0;
		return 2;
	}

	static int ind_8(int dl, int& count){
		int ret;
		if(dl == 3){ret = count & 0xf; count += 3;}
		else ret = count >> 4;
		return ret;
	}

	static int ind_9(int dl, int& count){
		if(dl == 3) return 0;
		return 3;
	}

	static int ind_10(int dl, int& count){
		if(dl == 4) return 0;
		return 4;
	}

	static int ind_11(int dl, int& count){
		int ret = count;
		count += 5;
		return ret;
	}

	static int ind_12(int dl, int& count){
		return 0;
	}

public:
	void kanon(const kvadrat& lk, kvadrat& kf);
};
