#include "kanon.h"

using namespace std;

const char* input = "input.txt";
const char* output = "output.txt";

list<kvadrat> baza_lk;
set<kvadrat> baza_kf;

bool pars_arg(int argc, char* argv[]){
	bool yes = true, first = true, last = true;
	for(int i = 1; i < argc; i++){
		if(argv[i][0] == '/'){if(yes && argv[i][1] == 'm') yes = false;}
		else{
			if(first){input = argv[i]; first = false;}
			else if(last){output = argv[i]; last = false;}
			else if(!yes) break;
		}
	}
	return yes;
}

void zapis(char ch){
	static int count;
	static kvadrat kv;
	if(ch < '0' || ch > '9') return;
	kv[count++] = ch - '0';
	if(count == raz){
		if(is_lk(kv)) baza_lk.push_back(kv);
		count = 0;
	}
}

int init(){
	ifstream fin(input, ios::binary);
	if(!fin){cerr << "Нет файла " << input << endl; return 1;}
	const int raz_buf = 0x1000;
	char bufer[raz_buf];
	while(fin.read(bufer, raz_buf)) for(int i = 0; i < raz_buf; i++) zapis(bufer[i]);
	if(fin.eof()) for(int i = 0; i < fin.gcount(); i++) zapis(bufer[i]);
	size_t count = baza_lk.size();
	if(!count){cerr << "Нет ЛК в файле " << input << endl; return 2;}
	cout << "Введено ЛК   : " << count << endl;
	return 0;
}

void rabota(){
	kvadrat tempk;
	clock_t tb = clock(), te;
	int count = 0;
	Kanonizator_lk kanonizator{};
	for(auto q = baza_lk.begin(); q != baza_lk.end(); q++){
		kanonizator.kanon(*q, tempk);
		count++;
		if((te = clock()) - tb > 5 * CLOCKS_PER_SEC){
			cout << count << endl;
			tb = te;
		}
		baza_kf.insert(tempk);
	}
}

void vyvod(){
	ofstream fout(output, ios::binary);
	for(auto q = baza_kf.begin(); q != baza_kf.end(); q++) out_kvadrat(fout, *q);
	cout << "Найдено КФ ЛК: " << baza_kf.size() << endl;
}

int main(int argc, char* argv[]){
	clock_t t0 = clock();
	setlocale(LC_CTYPE, "rus");
	cout << "Канонизатор ЛК10\n\n";
	bool yes = pars_arg(argc, argv);
	int ret;
	if(!(ret = init())){
		rabota();
		vyvod();
		cout << "Время работы : " << double(clock() - t0) / CLOCKS_PER_SEC << " сек\n\n";
	}
	if(yes){
		cout << "Для выхода нажмите любую клавишу . . . ";
		system("pause > nul");
		cout << endl;
	}
	return ret;
}
