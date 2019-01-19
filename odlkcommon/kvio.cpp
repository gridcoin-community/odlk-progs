#include "odlkcommon/common.h"

bool in_kvadrat(std::istream& in, kvadrat& kvad)
{
  for(unsigned i= 0; i < raz; ++i)
  {
    int v;
    if(!( in>>v ) )
      return false;
    kvad[i] = v;
  }
  return true;
}

void out_kvadrat(std::ostream& out, const kvadrat& kv)
{
  using namespace std;
	static const int raz_kvb = 212;
	array<char,raz_kvb> tempk;
	char* p = tempk.data();
	for(int i = 0; i < raz; i += por){
		*p++ = kv[i] + '0';
		for(int j = 1; j < por; j++){
			*p++ = ' ';
			*p++ = kv[i + j] + '0';
		}
		*p++ = '\r';
		*p++ = '\n';
	}
	*p++ = '\r';
	*p = '\n';
	out.write((char*)tempk.data(), sizeof(tempk));
}
