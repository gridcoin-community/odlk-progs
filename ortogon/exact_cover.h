#include <iostream>
#include <array>
#include <map>
#include <list>
#include "odlkcommon/common.h"

class Exact_cover{
	typedef std::array<unsigned char, por> transver;

	static const int max_trans = 10000;
	static const int max_cols = raz + 1;
	static const int max_nodes = max_trans * por;
	transver baza_trans[max_trans];

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

	data* O[por];
	data heads[max_cols];
	data nodes[max_nodes];

	void save_solution(std::list<kvadrat>& templ){
		kvadrat mate;
		for(int i = 0; i < por; i++){
			for(int j = 0, k = 0; j < por; k += por, j++){
				mate[k + baza_trans[O[i]->column >> 7][j]] = i;
			}
		}
		templ.push_back(mate);
	}

	data* choose_column(){
		data* c = heads->right;
		int s = c->size;
		if(!s) return c;
		for(data* j = c->right; j != heads; j = j->right) if(j->size < s){
			c = j;
			if(!(s = j->size)) return c;
		}
		return c;
	}

	void cover_column(data* p){
		p->right->left = p->left;
		p->left->right = p->right;
		for(data* i = p->down; i != p; i = i->down){
			for(data* j = i->right; j != i; j = j->right){
				j->down->up = j->up;
				j->up->down = j->down;
				heads[j->column & 0x7f].size--;
			}
		}
	}

	void uncover_column(data* p){
		for(data* i = p->up; i != p; i = i ->up){
			for(data* j = i->left; j != i; j = j->left){
				heads[j->column & 0x7f].size++;
				j->down->up = j->up->down = j;
			}
		}
		p->right->left = p->left->right = p;
	}

	void init(int ch_trans);
	void search(std::list<kvadrat>& templ);
	void init_trans(const kvadrat& dlk);
	int search_trans(const kvadrat& dlk);

public:

	static bool is_dlk(const kvadrat& kvad);
	void search_mate(const kvadrat& dlk, std::list<kvadrat>& mates);
};
