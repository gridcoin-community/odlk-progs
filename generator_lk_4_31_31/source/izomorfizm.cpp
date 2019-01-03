#include "main_const.h"
#include "izomorfizm.h"

std::map<std::string, int> nom_strukt;

void init_nom_strukt(){
	for(int i = 0; i < ch_kl; i++) nom_strukt.insert(std::make_pair(std::string(strukt[i]), i));
}

int get_nom_strukt(const morfizm& per){
	std::set<int> mnozh;
	for(int i = 0; i < por; i++) mnozh.insert(i);
	int first, temp, dl, count = 0;
	char cikl_strukt[por + 1];
	for(auto q = mnozh.begin(); !mnozh.empty(); q = mnozh.begin()){
		first = temp = *q;
		mnozh.erase(q);
		dl = 0;
		while((temp = per[temp]) != first){
			mnozh.erase(temp);
			dl++;
		}
		cikl_strukt[count++] = dl + '0';
	}
	std::sort(cikl_strukt, cikl_strukt + count);
	cikl_strukt[count] = '\0';
	return nom_strukt[std::string(cikl_strukt)] + 1;
}

int get_symm(const izomorfizm& avtom){
	int temp[3], par = avtom.parastrof;
	if(par) return 0;
	for(int i = 0; i < 3; i++) temp[i] = get_nom_strukt(avtom.izotop[i]);
	std::sort(temp, temp + 3);
	return (temp[0] << 16) | (temp[1] << 8) | temp[2];
}

int get_nom(morfizm& per){
	std::set<int> mnozh;
	morfizm tempm;
	for(int i = 0; i < por; i++) mnozh.insert(i);
	int first, temp, dl, count = 0, ind = 0, dlind[por];
	char cikl_strukt[por + 1];
	for(auto q = mnozh.begin(); !mnozh.empty(); q = mnozh.begin()){
		first = temp = *q;
		mnozh.erase(q);
		dl = 0;
		tempm[ind++] = temp; 
		while((temp = per[temp]) != first){
			mnozh.erase(temp);
			dl++;
			tempm[ind++] = temp;
		}
		cikl_strukt[count] = dl + '0';
		dlind[count++] = ((dl + 1) << 8) | (ind - dl - 1);
	}
	std::sort(dlind, dlind + count);
	for(int i = 0, id, kol, c = 0; i < count; i++){
		id = dlind[i] & 0xff;
		kol = dlind[i] >> 8;
		for(int j = 0; j < kol; j++) per[c++] = tempm[id + j];
	}
	std::sort(cikl_strukt, cikl_strukt + count);
	cikl_strukt[count] = '\0';
	return nom_strukt[std::string(cikl_strukt)] + 1;
}

izomorfizm get_kan(const izomorfizm& x){
	izomorfizm y = x;
	morfizm z[3], tempm;
	for(int i = 0; i < por; i++) tempm[y.izotop[2][i]] = i;
	y.izotop[2] = tempm;
	int w[3], v[3];
	for(int i = 0; i < 3; i++) w[i] = (get_nom(z[i] = y.izotop[i]) << 8) | i;
	std::sort(w, w + 3);
	for(int i = 0; i < 3; i++) v[w[i] & 0xff] = i;
	static const int par[6] = {2,5,4,3,1,0};
	y.parastrof = par[(v[2] << 1) + v[1] - (v[1] > v[2])];
	for(int i = 0; i < 3; i++) y.izotop[i] = z[w[i] & 0xff];
	for(int i = 0; i < por; i++) tempm[y.izotop[2][i]] = i;
	y.izotop[2] = tempm;
	return y;
}
