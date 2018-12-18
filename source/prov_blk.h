#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <list>
#include <set>
#include <string>
#include <ctime>

using namespace std;

const int por = 10;
const int raz = por * por;
const int ch_srez = 3;
const int max_trans = 5504;
const int timeout = 5 * CLOCKS_PER_SEC;

typedef array<unsigned char, raz> kvadrat;
typedef array<unsigned char, por> transver;

extern list<transver> d_trans;
extern vector<transver> trans[ch_srez];
extern list<pair<kvadrat, pair<transver, transver>>> kf_trans[ch_srez];
extern set<kvadrat> baza_kf, baza_mar;
extern int cnt_trans;