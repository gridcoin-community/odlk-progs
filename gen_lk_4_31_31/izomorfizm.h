const int ch_kl = 42;
static const char* strukt[ch_kl] = {
	"0000000000",
	"000000001",
	"00000002",
	"00000011",
	"0000003",
	"0000012",
	"000004",
	"0000111",
	"000013",
	"000022",
	"00005",
	"000112",
	"00014",
	"00023",
	"0006",
	"001111",
	"00113",
	"00122",
	"0015",
	"0024",
	"0033",
	"007",
	"01112",
	"0114",
	"0123",
	"016",
	"0222",
	"025",
	"034",
	"08",
	"11111",
	"1113",
	"1122",
	"115",
	"124",
	"133",
	"17",
	"223",
	"26",
	"35",
	"44",
	"9"
};

struct izomorfizm{
	int parastrof;
	morfizm izotop[3];

	izomorfizm operator~()const{
		izomorfizm tempm;
		morfizm tempmm[3];
		for(int i = 0; i < por; i++){
			tempmm[0][izotop[0][i]] = i;
			tempmm[1][izotop[1][i]] = i;
			tempmm[2][izotop[2][i]] = i;
		}
		const morfizm* el[6] = {&izotop[0],&izotop[1],&tempmm[2],&tempmm[0],&tempmm[1],&izotop[2]};
		static const int rasp[6] = {0x342,0x432,0x540,0x351,0x531,0x450};
		for(int i = 0, t = rasp[parastrof]; i < 3; i++) tempm.izotop[i] = *el[(t >> ((2 - i) << 2)) & 0xf];
		tempm.parastrof = parastrof;
		if(parastrof > 3) tempm.parastrof ^= 1;
		return tempm;
	}

	izomorfizm operator*(const izomorfizm& x)const{
		izomorfizm tempm;
		morfizm tempmm[3];
		for(int i = 0; i < por; i++){
			tempmm[0][izotop[0][i]] = i;
			tempmm[1][izotop[1][i]] = i;
			tempmm[2][izotop[2][i]] = i;
		}
		const morfizm* el[6] = {&izotop[0],&izotop[1],&tempmm[2],&tempmm[0],&tempmm[1],&izotop[2]};
		static const int rasp[6] = {0x15,0x105,0x213,0x24,0x123,0x204};
		for(int i = 0, t = rasp[x.parastrof]; i < 3; i++) tempm.izotop[i] = *el[(t >> ((2 - i) << 2)) & 0xf];
		morfizm tempmr;
		for(int i = 0; i < por; i++) tempmr[i] = tempm.izotop[0][x.izotop[0][i]];
		tempm.izotop[0] = tempmr;
		for(int i = 0; i < por; i++) tempmr[i] = tempm.izotop[1][x.izotop[1][i]];
		tempm.izotop[1] = tempmr;
		for(int i = 0; i < por; i++) tempmr[i] = x.izotop[2][tempm.izotop[2][i]];
		tempm.izotop[2] = tempmr;
		static const int tabl[6][6] = {
			0,1,2,3,4,5,
			1,0,5,4,3,2,
			2,4,0,5,1,3,
			3,5,4,0,2,1,
			4,2,3,1,5,0,
			5,3,1,2,0,4
		};
		tempm.parastrof = tabl[parastrof][x.parastrof];
		return tempm;
	}

	bool operator==(const izomorfizm& x)const{
		if(parastrof != x.parastrof) return false;
		for(int i = 0; i < 3; i++) if(izotop[i] != x.izotop[i]) return false;
		return true;
	}

	void operator()(kvadrat& kv){
		kvadrat tempk;
		if(!parastrof || parastrof == 1) tempk = kv;
		else if(parastrof == 2 || parastrof == 4)
			for(int j = 0; j < por; j++) for(int i = 0; i < por; i++) tempk[j + kv[j + i * por] * por] = i;
		else for(int i = 0; i < por; i++) for(int j = 0; j < por; j++) tempk[i * por + kv[i * por + j]] = j;
		if(parastrof == 1 || parastrof == 4 || parastrof == 5)
			for(int i = 0; i < por - 1; i++) for(int j = i + 1; j < por; j++) std::swap(tempk[i * por + j], tempk[j * por + i]);
		for(int i = 0; i < por; i++) for(int j = 0; j < por; j++)
			kv[i * por + j] = izotop[2][tempk[izotop[0][i] * por + izotop[1][j]]];
	}
};

extern std::map<std::string, int> nom_strukt;

void init_nom_strukt();
int get_nom_strukt(const morfizm& per);
int get_symm(const izomorfizm& avtom);
int get_nom(morfizm& per);
izomorfizm get_kan(const izomorfizm& x);
