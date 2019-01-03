#include <array>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <string>
#include <algorithm>
#include <intrin.h>
#include <ostream>

const int por = 10;
const int por_m = por >> 1;
const int raz_m = por * por_m;
const int raz = por * por;

typedef std::array<unsigned char, raz_m> shablon;
typedef std::array<unsigned char, raz> kvadrat;
typedef std::array<unsigned char, por> morfizm;

inline std::ostream& operator<<(std::ostream& out, const shablon& sh){
	for(int i = 0; i < raz_m; i++) if(sh[i] == por_m){
		i += por_m - 1;
		continue;
	}
	else out << char(sh[i] + '0') << (i % por_m != por_m - 1 ? ' ' : '\n');
	out << '\n';
	return out;
}
