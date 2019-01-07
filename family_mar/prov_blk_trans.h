#include "prov_blk.h"

class trans_dlx{
	static const int ch_cols = 3 * por + 1;
	static const int ch_cols_simm = 4 * por + 1;
	static const int max_cols = raz + 1;
	static const int max_nodes = max_trans * por;
	static const int n = por >> 1;

	struct data{
		data* left;
		data* right;
		data* up;
		data* down;
		union{
			int column;
			int size;
		};
	};

	static data* O[por];
	static data heads[max_cols];
	static vector<data> nodes;

	static data* choose_column(){
		data* c = heads->right;
		int s = c->size, ss;
		if(!s) return 0;
		for(data* j = c->right; j != heads; j = j->right) if((ss = j->size) < s){
			c = j;
			if(!(s = ss)) return 0;
		}
		return c;
	}

	static void cover_column(data* p){
		p->right->left = p->left;
		p->left->right = p->right;
		for(data* i = p->down; i != p; i = i->down){
			for(data* j = i->right; j != i; j = j->right){
				j->down->up = j->up;
				j->up->down = j->down;
				(heads + (j->column & 0x7f))->size--;
			}
		}
	}

	static void uncover_column(data* p){
		for(data* i = p->up; i != p; i = i ->up){
			for(data* j = i->left; j != i; j = j->left){
				(heads + (j->column & 0x7f))->size++;
				j->down->up = j->up->down = j;
			}
		}
		p->right->left = p->left->right = p;
	}

	static bool is_simm(const kvadrat& kf, int lin){
		if(lin != 0 && lin != 5) return false;
		static const int diag[2][por] = {1,0,3,2,6,7,4,5,9,8,1,0,3,2,6,8,4,9,5,7};
		for(int i = 0; i < por; i++) for(int j = 0; j < por >> 1; j++){
			if(kf[i * por + j] != diag[lin == 5][kf[(i + 1) * por - 1 - j]]) return false;
		}
		return true;
	}

	static void postr(kvadrat& dlk, const kvadrat& lk, const data* O[]){
		int rows[por], cols[por];
		for(int i = 0, t; i < n; i++){
			t = O[i]->column >> 7;
			rows[i] = t >> 12;
			rows[por - 1 - i] = (t >> 8) & 0xf;
			cols[i] = (t >> 4) & 0xf;
			cols[por - 1 - i] = t & 0xf;
		}
		for(int i = 0; i < por; i++) for(int j = 0; j < por; j++) dlk[i * por + j] = lk[rows[i] * por + cols[j]];
	}

	static void init_trans(const kvadrat& lk);
	static bool init_simm(const kvadrat& lk, const transver& tr);
	static void poisk_simm(const kvadrat& lk, const vector<transver>& tr, int flag);
	static void poisk_simm_dlx(const kvadrat& lk, const vector<transver>& tr, int flag);
	static void init_mar();
	friend int init();

	trans_dlx(){}

public:

	static void search_trans(const kvadrat& lk);
	static void search_symm_trans(const kvadrat* srez[]);
	static bool is_mar();
};
