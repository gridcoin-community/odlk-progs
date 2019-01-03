extern int pattern;
extern shablon shab;

class super_class{
	shablon init_shab;
	bool exist;

	void get_syst(int sv[][2], int ch_sv[], int syst[], int& ch_ur){
		for(int i = 0, v, tt; i < por_m; i++){
			if(ch_sv[i] < 2) continue;
			v = 0; tt = 0;
			for(int j = 0, t, i0, i1, jj; j < ch_sv[i]; j++){
				t = sv[i][j]; i0 = t >> 8; i1 = (t >> 4) & 0xf; jj = t & 0xf;
				if(i0 > 1) v ^= 1;
				if(i1 > 1) v ^= 1;
				if(jj) tt |= 1 << (jj - 1);
			}
			syst[ch_ur++] = (v << 4) | tt;
		}
	}

	bool solve_syst(int syst[], int& ch_ur, int isp){
		if(!ch_ur) return true;
		int ch = 0;
		for(int f = isp, t; f; f ^= t){
			t = f & -f;
			for(int i = ch; i < ch_ur; i++) if(syst[i] & t){
				std::swap(syst[ch], syst[i]);
				for(int j = i + 1; j < ch_ur; j++) if(syst[j] & t) syst[j] ^= syst[ch];
				ch++;
				goto next;
			}
			for(int i = 0; i < ch; i++) if(syst[i] & t) syst[i] ^= t | (t << 5);
			next:;
		}
		for(int i = ch; i < ch_ur; i++) if(syst[i]) return false;
		for(int i = (ch_ur = ch) - 1; i; i--) for(int j = i - 1; j >= 0; j--)
			if(syst[j] & syst[i] & 0xf) syst[j] ^= syst[i];
		return true;
	}

	void init(const int syst[], unsigned ch_ur){
		for(unsigned long k = 0, r, j; k < ch_ur; k++){
			if(syst[k] & 0x10){
				_BitScanForward(&r, syst[k] & 0xf);
				j = r + 1;
				std::swap(init_shab[por_m + j], init_shab[raz_m - por + j]);
			}
		}
	}
public:
	explicit super_class(const shablon& super): init_shab(super){
		int sv[por_m][2], ch_sv[por_m] = {};
		for(int j = 0, f; j < por_m; j++){
			f = 0;
			for(int i = 0, t, tt; i < por; i++){
				t = super[i * por_m + j];
				if(tt = (f >> (t << 2)) & 0xf) sv[t][ch_sv[t]++] = (--tt << 8) | (i << 4) | j;
				else f |= (i + 1) << (t << 2);
				if(i == 1) i = 7;
			}
		}
		int syst[por_m] = {}, ch_ur = 0, ispol = 0;
		get_syst(sv, ch_sv, syst, ch_ur);
		for(int i = 0; i < ch_ur; i++) ispol |= syst[i];
		ispol &= 0xf;
		if(exist = solve_syst(syst, ch_ur, ispol)) init(syst, ch_ur);
	}

	bool empty(){return !exist;}

	void get_init(shablon& sh){sh = init_shab;}
};

class super{
	shablon baz_sh;
	int iter, stl[por_m], cnt, ind, kol;

	void zapoln(int mass[], int c){
		std::sort(mass, mass + c);
		int st = 0;
		for(int i = 0; i < c; i++){
			st = (st << 3) | mass[i];
			ind ^= 1 << mass[i];
		}
		kol += c;
		if(mass[0] == 0) stl[0] = st;
		else stl[++cnt] = st;
	}

	void zapoln(int flag1, int flag2, const std::multimap<int, int>& otn){
		std::multimap<int, int>::const_iterator w;
		int mass[por_m], c;
		for(unsigned long f = flag1, r, t, tt; f; f &= f - 1){
			_BitScanForward(&r, f);
			c = 0;
			mass[c++] = r;
			for(t = otn.find(r)->second, tt = r; flag2 & (1 << t); tt = t, t = w->second){
				mass[c++] = t;
				w = otn.lower_bound(t);
				if(w->second == tt) w++;
				flag2 ^= 1 << t;
			}
			mass[c++] = t;
			f ^= 1 << t;
			zapoln(mass, c);
		}
		for(unsigned long f = flag2, r, t, tt; f; f &= f - 1){
			_BitScanForward(&r, f);
			c = 0;
			mass[c++] = r;
			for(t = otn.lower_bound(r)->second, tt = r; t != r; tt = t, t = w->second){
				mass[c++] = t;
				w = otn.lower_bound(t);
				if(w->second == tt) w++;
				f ^= 1 << t;
			}
			zapoln(mass, c);
		}
	}

	void zapoln(const int* sv){
		std::set<int> temps;
		for(int i = 0, t; i < por_m; i++) if((t = sv[i]) >> 3) temps.insert(t - 011);
		int flag1 = 0, flag2 = 0, pr, vt;
		std::multimap<int, int> otn;
		for(auto q = temps.begin(); q != temps.end(); q++){
			pr = (*q) >> 3; vt = (*q) & 7;
			otn.insert(std::make_pair(pr, vt)); otn.insert(std::make_pair(vt, pr));
			if(flag1 & (1 << pr)) flag2 |= 1 << pr; else flag1 |= 1 << pr;
			if(flag1 & (1 << vt)) flag2 |= 1 << vt; else flag1 |= 1 << vt;
		}
		zapoln(flag1 ^ flag2, flag2, otn);
	}
public:
	super(const shablon& sh, bool kn = false);
	bool next(shablon& sh);
};

class kan_shab{
	friend class super;

	static const int gray[];
	static const int ukaz_g[][2];
	static const int ukaz_gg[][2];
	static const int ukaz[][2];
	static const int ind_gg[];
	static const int per_f[][por];

	static int per[][(por << 1) + 1];

	static void norm0(shablon& sh);
	static void norm1(shablon& sh);
	static void norm_f_col(shablon& sh);
	static void trans1(const shablon& sh, shablon& frm);
	static void trans2(const shablon& sh, shablon& frm);
	static void init_formy0(shablon* formy, const shablon& sh);
	static void init_formy1(shablon* formy, const shablon& sh);
	static void init_formy0(std::vector<shablon>& formy, const shablon& sh);
	static void init_formy1(std::vector<shablon>& formy, const shablon& sh);
	static bool is_kanon0(const shablon& sh, const int ch_form);
	static bool is_kanon1(const shablon& sh, const int ch_form);
	static bool is_obrabotka0(const std::vector<shablon>& formy, const std::vector<int>& sp, const shablon& sh, const int* morf);
	static bool is_obrabotka1(const std::vector<shablon>& formy, const std::vector<int>& sp, const shablon& sh, const int* morf);
	static void kanon0(shablon& sh, const int ch_form);
	static void kanon1(shablon& sh, const int ch_form);
	static void obrabotka0(const std::vector<shablon>& formy, const std::vector<int>& sp, shablon& sh, const int* morf);
	static void obrabotka1(const std::vector<shablon>& formy, const std::vector<int>& sp, shablon& sh, const int* morf);
	static void get_dv(const shablon& sh, shablon& dvsh);

	static void utochn(shablon& sh){
		static const int peru[][por + 1] = {
			0,1,3,2,4,0,1,3,2,4,16,
			0,1,2,4,3,0,3,2,1,4,28,
			0,1,4,3,2,0,2,1,3,4,8
		};
		int x = sh[por_m], y = sh[raz_m - por];
		switch(x < y ? (x << 3)  | y : (y << 3) | x){
			case 013: trans(sh, peru[0]); break;
			case 02: case 024: trans(sh, peru[2]); break;
			case 03: case 023: case 034: trans(sh, peru[1]);
		}
	}

	static void trans(shablon& sh, const int* per_c){
		const int *per_v = per_c + por_m, flag = per_c[por];
		shablon tempsh;
		for(int i = 0; i < raz_m; i += por_m) for(int j = 0; j < por_m; j++) tempsh[i + j] = per_v[sh[i + per_c[j]]];
		for(unsigned long f = flag, r; f; f &= f - 1){
			_BitScanForward(&r, f);
			std::swap(tempsh[r], tempsh[raz_m - por_m + r]);
			std::swap(tempsh[por_m + r], tempsh[raz_m - por + r]);
		}
		sh = tempsh;
	}

	static int get_pattern(const shablon& sh){
		std::set<int> temps;
		int patt = 0;
		for(int t = 0; t < 2; t++){
			temps.clear();
			for(int i = 0, x, y; i < por_m; i++){
				x = sh[por_m * t + i];
				y = sh[raz_m - por_m * (1 + t) + i];
				temps.insert(x < y ? (x << 3) | y : (y << 3) | x);
			}
			patt = (patt << 1) | (temps.size() == por_m);
		}
		return patt;
	}

	kan_shab(){}
public:
	static bool is_super(const shablon& sh);
	static bool is_min_dv(const shablon& sh);

	static bool is_kanon(const shablon& sh){
		switch(pattern){
			case 0: return is_kanon0(sh, 8);
			case 1: return is_kanon0(sh, 4);
			case 2: return is_kanon1(sh, 20);
		}
		return true;
	}

	static int kanon(shablon& sh){
		int patt = get_pattern(sh);
		switch(patt){
			case 0: kanon0(sh, 8); break;
			case 1: kanon0(sh, 4); break;
			case 2:
				for(int i = 0; i < por_m; i++){
					std::swap(sh[i], sh[por_m + i]);
					std::swap(sh[raz_m - por_m + i], sh[raz_m - por + i]);
				}
				kanon0(sh, 4);
				break;
			case 3: kanon1(sh, 20);
		}
		return patt - (patt > 1);
	}

	static int get_super(shablon& sh);

	static int get_kan_sh_sem(shablon & sh){
		int patt = get_super(sh);
		shablon tempsh;
		get_dv(sh, tempsh);
		get_super(tempsh);
		if(tempsh < sh) sh = tempsh;
		return patt;
	}
};
