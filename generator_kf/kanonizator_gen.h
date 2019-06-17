#include <array>

const int por = 10;
const int raz = por * por;
const int n = por * (por - 2);
const unsigned all = (1 << por) - 1;
const int ch_lin = 67;
const int raz_kvb = 201;
const int rc[n] = {
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
	0x10,0x12,0x13,0x14,0x15,0x16,0x17,0x19,
	0x20,0x21,0x23,0x24,0x25,0x26,0x28,0x29,
	0x30,0x31,0x32,0x34,0x35,0x37,0x38,0x39,
	0x40,0x41,0x42,0x43,0x46,0x47,0x48,0x49,
	0x50,0x51,0x52,0x53,0x56,0x57,0x58,0x59,
	0x60,0x61,0x62,0x64,0x65,0x67,0x68,0x69,
	0x70,0x71,0x73,0x74,0x75,0x76,0x78,0x79,
	0x80,0x82,0x83,0x84,0x85,0x86,0x87,0x89,
	0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98
};
const int diag[ch_lin][por] = {
	1,0,3,2,6,7,4,5,9,8,
	1,0,3,2,6,7,4,8,9,5,
	1,0,3,2,6,7,5,4,9,8,
	1,0,3,2,6,7,8,9,4,5,
	1,0,3,2,6,7,8,9,5,4,
	1,0,3,2,6,8,4,9,5,7,
	1,0,3,2,6,8,4,9,7,5,
	1,0,3,2,6,8,5,9,4,7,
	1,0,3,2,6,8,5,9,7,4,
	1,0,3,2,6,8,7,4,9,5,
	1,0,3,2,6,8,9,4,7,5,
	1,0,3,2,6,8,9,5,7,4,
	1,0,3,4,2,6,8,9,5,7,
	1,0,3,4,2,6,8,9,7,5,
	1,0,3,4,2,7,5,6,9,8,
	1,0,3,4,2,7,5,8,9,6,
	1,0,3,4,2,7,8,9,5,6,
	1,0,3,4,2,7,8,9,6,5,
	1,0,3,4,6,2,5,8,9,7,
	1,0,3,4,6,2,8,5,9,7,
	1,0,3,4,6,2,8,9,5,7,
	1,0,3,4,6,8,2,9,7,5,
	1,0,3,4,6,8,5,9,2,7,
	1,0,3,4,6,8,7,9,2,5,
	1,0,3,4,6,8,7,9,5,2,
	1,0,3,4,7,2,8,9,5,6,
	1,0,3,4,7,2,8,9,6,5,
	1,0,3,4,7,8,5,9,2,6,
	1,0,3,4,7,8,5,9,6,2,
	1,0,3,4,8,7,5,9,2,6,
	1,0,3,4,8,7,5,9,6,2,
	1,0,3,4,8,9,5,6,2,7,
	1,0,3,4,8,9,5,6,7,2,
	1,0,3,7,6,8,5,9,2,4,
	1,0,3,7,6,8,5,9,4,2,
	1,0,3,7,8,9,2,6,4,5,
	1,0,3,7,8,9,2,6,5,4,
	1,2,0,4,6,3,5,9,7,8,
	1,2,0,4,6,3,7,9,5,8,
	1,2,0,4,6,7,8,9,3,5,
	1,2,0,4,7,8,5,9,3,6,
	1,2,0,4,7,8,5,9,6,3,
	1,2,0,4,7,8,9,3,6,5,
	1,2,0,4,7,8,9,5,6,3,
	1,2,0,4,7,9,8,5,3,6,
	1,2,0,4,7,9,8,6,5,3,
	1,2,3,0,6,7,8,9,5,4,
	1,2,3,0,6,7,9,4,5,8,
	1,2,3,4,0,7,5,9,6,8,
	1,2,3,4,0,7,8,9,5,6,
	1,2,3,4,0,9,5,6,7,8,
	1,2,3,4,6,0,8,9,7,5,
	1,2,3,4,6,7,5,9,0,8,
	1,2,3,4,6,8,9,5,0,7,
	1,2,3,7,6,8,5,9,0,4,
	1,2,3,7,6,9,5,4,0,8,
	1,0,3,2,6,7,5,8,9,4,
	1,0,3,4,6,2,8,9,7,5,
	1,0,3,4,6,7,8,9,2,5,
	1,0,3,4,6,7,8,9,5,2,
	1,0,3,4,6,8,5,9,7,2,
	1,0,3,4,6,8,9,5,2,7,
	1,0,3,4,6,8,9,5,7,2,
	1,0,3,4,8,6,9,5,2,7,
	1,2,0,4,6,7,8,9,5,3,
	1,2,3,0,6,7,5,9,4,8,
	1,2,3,4,6,9,8,0,5,7
};

typedef std::array<unsigned char,raz> kvadrat;
typedef std::array<char,raz_kvb> kvadrat_b;
typedef unsigned long long ull;

extern int lin;

class kanonizator{
	typedef std::array<unsigned char,por> morfizm;

	static const int ch_klass2 = 56;
	static const int raz_trans_tabl = 114;
	static const int max_chform = 2;
	static const int trans_tabl[raz_trans_tabl][por << 1];
	static const int ukaz[ch_lin];
	static const int dop_trans[ch_klass2][por];

	static kvadrat formy[max_chform];
	static int chform;

	friend void generator();
	bool obrabotka(const int* perest, const int* ob_perest);

	void simmetr(){
		kvadrat tempk = formy[0];
		for(int i = 0; i < por; i++) for(int j = 0; j < (por >> 1); j++)
			std::swap(tempk[i * por + j], tempk[i * por + por - 1 - j]);
		for(int i = 0; i < por; i++) for(int j = 0; j < por; j++)
			formy[1][i * por + j] = tempk[dop_trans[lin][i] * por + dop_trans[lin][j]];
		morfizm perest;
		for(int i = 0, j = 0; i < por; j += por + 1, i++) perest[formy[1][j]] = i;
		for(int i = 0; i < raz; i++) formy[1][i] = perest[formy[1][i]];
	}

public:

	bool operator()(const int X[]);
};
