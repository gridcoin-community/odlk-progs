#include "exact_cover.h"

bool Exact_cover::is_dlk(const kvadrat& kvad){
	for(int i = 0, flag; i < raz; i += por){
		flag = 0;
		for(int j = 0, t; j < por; j++){
			if(flag & (t = 1 << kvad[i + j])) return false;
			flag |= t;
		}
	}
	for(int i = 0, flag; i < por; i++){
		flag = 0;
		for(int j = 0, t; j < raz; j += por){
			if(flag & (t = 1 << kvad[i + j])) return false;
			flag |= t;
		}
	}
	for(int j = 0, flag = 0, t; j < raz + por; j += por + 1){
		if(flag & (t = 1 << kvad[j])) return false;
		flag |= t;
	}
	for(int j = por - 1, flag = 0, t; j <= raz - por; j += por - 1){
		if(flag & (t = 1 << kvad[j])) return false;
		flag |= t;
	}
	return true;
}

void Exact_cover::search(std::list<kvadrat>& templ){
	data* c;
	data* r;
	int l = 0;
enter_level_l: 
	if(l >= por){
		save_solution(templ);
		goto backtrack;
	}
	c = choose_column();
	r = c->down;
try_to_advance:
	if(r != c){
		cover_column(c);
		for(data* j = r->right; j != r; j = j->right) cover_column(heads + (j->column & 0x7f));
		O[l++] = r;
		goto enter_level_l;
	}
backtrack:
	if(--l >= 0){
		r = O[l];
		c = heads + (r->column & 0x7f);
		for(data* j = r->left; j != r; j = j->left) uncover_column(heads + (j->column & 0x7f));
		uncover_column(c);
		r = r->down;
		goto try_to_advance;
	}
}

void Exact_cover::init(int ch_trans){
	for(int i = 0; i <= raz; i++){
		heads[i].right = heads + i + 1;
		heads[i].left = heads + i - 1;
		heads[i].down = heads[i].up = heads + i;
		heads[i].size = 0;
	}
	heads[0].left += max_cols;
	heads[raz].right = heads;
	for(int i = 0, count = 0; i < ch_trans; count += por, i++){
		for(int j = 0, k = 0, t; j < por; k += por, j++){
			nodes[count + j].column = (i << 7) | (t = baza_trans[i][j] + k + 1);
			nodes[count + j].right = nodes + count + j + 1;
			nodes[count + j].left = nodes + count + j - 1;
			nodes[count + j].down = heads + t;
			nodes[count + j].up = heads[t].up;
			heads[t].up = heads[t].up->down = nodes + count + j;
			heads[t].size++;
		}
		nodes[count].left += por;
		nodes[count + por - 1].right -= por;
	}
}

void Exact_cover::search_mate(const kvadrat& dlk, std::list<kvadrat>& mates){
	int ch_trans = search_trans(dlk);
	if(ch_trans < por) return;
	if(ch_trans > max_trans){
		std::cerr << "Число трансверсалей " << ch_trans << " превышает максимум "
			<< max_trans << std::endl;
		return;
	}
	init(ch_trans);
	search(mates);
}

int Exact_cover::search_trans(const kvadrat& dlk){
	init_trans(dlk);
	transver tempt;
	int count = 0;
	data* c;
	data* r;
	int l = 0;
enter_level_l: 
	if(l >= por){
		for(int i = 0, t; i < por; i++){
			t = O[i]->column >> 7;
			tempt[t >> 4] = t & 0xf;
		}
		baza_trans[count++] = tempt;
		goto backtrack;
	}
	c = choose_column();
	r = c->down;
try_to_advance:
	if(r != c){
		cover_column(c);
		for(data* j = r->right; j != r; j = j->right) cover_column(heads + (j->column & 0x7f));
		O[l++] = r;
		goto enter_level_l;
	}
backtrack:
	if(--l >= 0){
		r = O[l];
		c = heads + (r->column & 0x7f);
		for(data* j = r->left; j != r; j = j->left) uncover_column(heads + (j->column & 0x7f));
		uncover_column(c);
		r = r->down;
		goto try_to_advance;
	}
	return count;
}

void Exact_cover::init_trans(const kvadrat& dlk){
	for(int i = 0; i <= 32; i++){
		heads[i].right = heads + i + 1;
		heads[i].left = heads + i - 1;
		heads[i].down = heads[i].up = heads + i;
		heads[i].size = 0;
	}
	heads[0].left = heads + 32;
	heads[32].right = heads;
	int temp[4];
	for(int i = 0, count = 0, r, c; i < raz; i++){
		temp[0] = (r = i / por) + 1;
		temp[1] = (c = i % por) + por + 1;
		temp[2] = dlk[i] + (por << 1) + 1;
		temp[3] = r == c ? 31 : r == por - 1 - c ? 32 : 0;
		for(int j = 0, t; j < 3; j++){
			nodes[count + j].column = (r << 11) | (c << 7) | (t = temp[j]);
			nodes[count + j].right = nodes + count + j + 1;
			nodes[count + j].left = nodes + count + j - 1;
			nodes[count + j].down = heads + t;
			nodes[count + j].up = heads[t].up;
			heads[t].up = heads[t].up->down = nodes + count + j;
			heads[t].size++;
		}
		if(int t = temp[3]){
			nodes[count + 3].column = (r << 11) | (c << 7) | t;
			nodes[count + 3].right = nodes + count;
			nodes[count + 3].left = nodes + count + 2;
			nodes[count + 3].down = heads + t;
			nodes[count + 3].up = heads[t].up;
			heads[t].up = heads[t].up->down = nodes + count + 3;
			heads[t].size++;
			nodes[count].left += 4;
			count += 4;
		}
		else{
			nodes[count].left += 3;
			nodes[count + 2].right -= 3;
			count += 3;
		}
	}
}
