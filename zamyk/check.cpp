#include <iostream>
#include <fstream>
#include <list>
#include <set>
#include <ctime>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <sstream>
#include <thread>
#include <mutex>
#include "kanonizator_lk/kanon.h"
#include "kanonizator_dlk/kanonizator.h"
#include "family_mar/prov_blk_trans.h"
#include "ortogon/exact_cover.h"

using namespace std;

unsigned load_squares(list<kvadrat>& lout, istream& fin)
{
	const unsigned raz_buf = 0x1000;
	char bufer[raz_buf];
  unsigned j = 0;
  unsigned count=0;
  do
  {
    fin.read(bufer, raz_buf);
    kvadrat kv;
    for(unsigned i = 0; i < fin.gcount(); ++i)
    {
      char& ch = bufer[i];
      if(ch >= '0' && ch <= '9')
      {
        kv[j++] = ch - '0';
        if(j == raz){
          lout.push_back(kv);
          j = 0;
          count++;
        }
      }
    }
  } while(fin.gcount());
  if(j!=0)
  {
    cerr << "Warning: Input file possibly corrupt"<<endl;
  }
  return count;
}

const char help_text[] =
"Program ...\n"
"check.exe input\n"
;

set<kvadrat> database;

void check_file(const string& input_name)
{
  ifstream fin(input_name, ios::binary);
  list<kvadrat> input;
  unsigned lcnt=load_squares(input,fin);
  unsigned luniq=0, ldup=0, lnolk=0, lnodlk=0;
  cout<<"Processing "<<input_name<<" : "<<lcnt<<endl;
  for(const kvadrat& kv : input) {
    if(!is_lk(kv)){
      lnolk++;
      cout<<"is_lk = false :"<<endl;
      out_kvadrat(cout,kv);
      continue;
    }
    if(!is_dlk(kv)){
      lnodlk++;
      cout<<"is_dlk = false :"<<endl;
      out_kvadrat(cout,kv);
    }
    kvadrat kf;
    kvadrat kv_copy{kv};
		int lin= Kanonizator_dlk::kanon(kv_copy, kf);
    auto ir= database.insert(kf);
    if(!ir.second) {
      ldup++;
      cout<<"duplicate (orig) :"<<endl;
      out_kvadrat(cout,kv);
      if(kf!=kv){
        cout<<"kf (lin="<<lin<<") of the duplicate:"<<endl;
        out_kvadrat(cout,kf);
      }
    } else {
      luniq++;
    }
  }
  cout<<input_name<<" -: in: "<<lcnt
    <<", not lk: "<<lnolk<<", not dlk: "<<lnodlk
    <<", dup: "<<ldup
    <<", new uniqe kf dlk: "<<luniq<<endl;
  cout<<"db unique kf dlk: "<<database.size()<<endl;
}


int main(int argc, char** argv)
{
  database.clear();
  for(unsigned i=1; i<argc; ++i)
  {
    check_file(argv[i]);
  }
}
