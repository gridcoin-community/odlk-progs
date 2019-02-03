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
#include <cassert>
#include "kanonizator_lk/kanon.h"
#include "kanonizator_dlk/kanonizator.h"
#include "family_mar/prov_blk_trans.h"
#include "ortogon/exact_cover.h"
#include "odlkcommon/namechdlk10.h"

/* Program to operate on database of DLK10

  UC:
  * add new entries to database and output unique
      dlkdbop.exe -dn db.bin input.txt
  * convert database to format 2 or 3
      dlkdbop.exe -ce db.bin db.txt

  Input:
  * text CF DLK -d
  * text CF LK -k
  * name58 (default)

  Output:
  * text CF DLK -e
  * name58 -n

  Options
  * read only -r

  DB Format:
  * db is in binary (default)
  * db is in name58 format -h
  * db is in cf dlk format -j

  Conversion:
  * convert db to: cf-dlk (-cd), name58 (-cn)
*/

using namespace std;

const char help_text[] =
"Program ...\n"
"dlkdbop.exe -[dkenrhj] db.bin input.txt\n"
"dlkdbop.exe -c[enhj] db.bin db.txt\n"
" -d : input file is in CF DLS format\n"
" -k : input file is in non-canonicial text format\n"
" -e : output as CF DLS\n"
" -n : output as name58 format\n"
" -r : read-only operation\n"
" -h : db is in name58 format\n"
" -j : db is in CF DLS format\n"
" -c : convert database to text format\n"
;

bool f_convert, f_write;
short f_in, f_out, f_db;
enum TFormat {
  FMT_LK=1,
  FMT_DLK=2,
  FMT_N58=3,
  FMT_BIN=4,
};
string name_db, name_in;

class CDbReader
{
  public:
  ifstream fin;
  short format;
  unsigned long line;
  
  void open(string name, short iformat)
  {
    format=iformat;
    fin= ifstream{ name, ios::binary };
    line=1;
  }

  bool fromNameBin(unsigned char *nb, kvadrat& kv)
  {
    NamerCHDLK10::Name nm;
    using NamerCHDLK10::osn;
    unsigned long long a;
    /* L-0 3-1 1-2 1-2 1-2 1-2 1-1 1-8*/
    //1-8
    a =        nb[17];
    a = a<<8 | nb[16];
    a = a<<8 | nb[15];
    a = a<<8 | nb[14];
    a = a<<8 | nb[13];
    a = a<<8 | nb[12];
    a = a<<8 | nb[11];
    a = a<<8 | nb[10];
    nm[8] = a % osn;
    //1-1
    a = (a/osn)<<8 | nb[9];
    nm[7] = a % osn;
    //1-2 * 4
    a = (a/osn)<<8 | nb[8];
    a =  a     <<8 | nb[7];
    nm[6] = a % osn;
    a = (a/osn)<<8 | nb[6];
    a =  a     <<8 | nb[5];
    nm[5] = a % osn;
    a = (a/osn)<<8 | nb[4];
    a =  a     <<8 | nb[3];
    nm[4] = a % osn;
    a = (a/osn)<<8 | nb[2];
    a =  a     <<8 | nb[1];
    nm[3] = a % osn;
    //3-1
    a = (a/osn)<<8 | nb[0];
    nm[2] = a % osn;
    a =  a/osn; nm[1] = a % osn;
    a =  a/osn; nm[0] = a % osn;
    // L-0
    nm[9] = a/osn;
    //decode
    return NamerCHDLK10::fromName(nm,kv);
  }

  bool read(kvadrat& kv)
  {
    using NamerCHDLK10::debase58;
    if(format!=FMT_BIN)
    {
      int kvi=0;
      NamerCHDLK10::NameStr name58;
      enum {
        snothing,
        stext,
        sname,
        sreml,sremb,
      } state;
      state=snothing;
      while(1)
      {
        char tmp;
        unsigned char ch;
        if(!fin.get(tmp)) {
          if(fin.eof()) ch=255;
          else {
            cerr<<"I/O Error reading file"<<endl;
            exit(3);
          }
        }
        else ch=tmp;

        if(ch=='\n') ++line;

        if(state==sreml && ch=='\n' ) state=snothing;
        else if(state==sremb && ch==']' ) state=snothing;
        else if(state==snothing && (ch=='#' || ch==';')) state=sreml;
        else if(state==snothing && ch=='[') state=sremb;
        else if(state<=stext && (format==FMT_LK||format==FMT_DLK) && ch >= '0' && ch <= '9' )
        {
          state=stext;
          kv[kvi++] = ch - '0';
          if(kvi==raz) {
            state=snothing;
            kvi=0;
            if(is_lk(kv)) {
              if(format==FMT_LK) {
                kvadrat tmp=kv;
                Kanonizator_dlk::kanon(tmp,kv);
              }
              return true;
            }
          }
        }
        else if(format==FMT_N58 && (state<=sname) && !(ch&0x80) && debase58[ch]!=255 )
        {
          state=sname;
          name58[kvi++]= ch;
          if(kvi==25) {
            state=snothing; kvi=0;
            NamerCHDLK10::fromName58(name58,kv);
            return true;
          }
        }
        else if(state<=stext && ch>0 && ch<255) {} //ignore
        else if(state==snothing && ch==255) return false; //EOF
        else
        {
          cerr<<"Invalid character '"<<int(ch)<<"' on line "<<line<<endl;
          exit(3);
        }
      }
    }
    else /* binary */
    {
      char nameB[18];
      NamerCHDLK10::Name name;
      fin.read(nameB,18);
      if(fin.gcount()!=18) {
        if(fin.gcount()==0 && fin.eof())
          return false;
        cerr<<"Corrupted Binary Database"<<endl;
        exit(3);
      }
      if(!fromNameBin((unsigned char*)nameB,kv)) {
        cerr<<"Corrupted Binary Database"<<endl;
        exit(3);
      }
      return true;
    }
  }
};

set<kvadrat> workset;

void save_work()
{

  ofstream fout { name_in+".new", ios::binary };
  if(f_out==FMT_DLK){
    for(auto q=workset.begin(); q!=workset.end(); ++q)
      out_kvadrat(fout, *q);
  } else if(f_out==FMT_N58) {
    for(auto q=workset.begin(); q!=workset.end(); ++q) {
      NamerCHDLK10::NameStr name;
      if(!NamerCHDLK10::getName58(*q,name)){
        cerr<<"Failed to get name for square!"<<endl;
        exit(5);
      }
      fout<<string(name.begin(),name.end())<<endl;
      //todo: avoid this string copy
    }
  }
  else assert(0);

  if(!fout.good()) {
    cerr<<"Wrtiting "<<name_in<<".new error!"<<endl;
    exit(4);
  }
  if(0!=rename(name_in.c_str(),(name_in+".old").c_str())) {
    /*cerr<<"Rename of "<<name_in<<" to "<<name_in<<".old failed!"<<endl;
    exit(4); Ignore the error. */
  }
  if(0!=rename((name_in+".new").c_str(),name_in.c_str())) {
    cerr<<"Rename of "<<name_in<<".new to "<<name_in<<" failed!"<<endl;
    exit(4);
  }
}

bool getNameBin(const kvadrat& kv, unsigned char *nb)
{
  NamerCHDLK10::Name nm;
  if(!NamerCHDLK10::getName(kv,nm))
    return false;
  using NamerCHDLK10::osn;
  unsigned long long a;
  /* L-0 3-1 1-2 1-2 1-2 1-2 1-1 1-8*/
  a = nm[9];
  // 3-1
  a = a*osn + nm[0];
  a = a*osn + nm[1];
  a = a*osn + nm[2];
  nb[0] = a & 0xFF; a>>=8;
  // 1-2 * 4
  a = a*osn + nm[3];
  nb[1] = a & 0xFF; a>>=8;
  nb[2] = a & 0xFF; a>>=8;
  a = a*osn + nm[4];
  nb[3] = a & 0xFF; a>>=8;
  nb[4] = a & 0xFF; a>>=8;
  a = a*osn + nm[5];
  nb[5] = a & 0xFF; a>>=8;
  nb[6] = a & 0xFF; a>>=8;
  a = a*osn + nm[6];
  nb[7] = a & 0xFF; a>>=8;
  nb[8] = a & 0xFF; a>>=8;
  // 1-1
  a = a*osn + nm[7];
  nb[9] = a & 0xFF; a>>=8;
  // 1-8
  a = a*osn + nm[8];
  nb[10] = a & 0xFF; a>>=8;
  nb[11] = a & 0xFF; a>>=8;
  nb[12] = a & 0xFF; a>>=8;
  nb[13] = a & 0xFF; a>>=8;
  nb[14] = a & 0xFF; a>>=8;
  nb[15] = a & 0xFF; a>>=8;
  nb[16] = a & 0xFF; a>>=8;
  nb[17] = a & 0xFF; a>>=8;
  return true;
}



void append_db()
{
  ofstream fout { name_db+".new", ios::binary };

  {
    ifstream fin { name_db, ios::binary };
    fin >> fout.rdbuf();
    if(fin.bad() || fout.fail()) {
      cerr<<"Copying database to new file failed!"<<endl;
      exit(4);
    }
  }
  
  if(f_db==FMT_DLK) {
    for(auto q=workset.begin(); q!=workset.end(); ++q)
      out_kvadrat(fout, *q);
  } else if(f_db==FMT_N58) {
    for(auto q=workset.begin(); q!=workset.end(); ++q) {
      NamerCHDLK10::NameStr name;
      if(!NamerCHDLK10::getName58(*q,name)){
        cerr<<"Failed to get name for square!"<<endl;
        exit(5);
      }
      fout<<string(name.begin(),name.end())<<endl;
      //todo: avoid this string copy
    }
  }
  else if(f_db==FMT_BIN) {
    for(auto q=workset.begin(); q!=workset.end(); ++q) {
      unsigned char bin[18];
      if(!getNameBin(*q,bin)){
        cerr<<"Failed to get name for square!"<<endl;
        exit(5);
      }
      fout.write((char*)bin,18);
    }
  }
  else assert(0);

  if(!fout.good()) {
    cerr<<"Wrtiting "<<name_db<<".new error!"<<endl;
    exit(4);
  }
  if(0!=rename(name_db.c_str(),(name_db+".old").c_str())) {
    cerr<<"Rename of "<<name_db<<" to "<<name_db<<".old failed!"<<endl;
    exit(4);
  }
  if(0!=rename((name_db+".new").c_str(),name_db.c_str())) {
    cerr<<"Rename of "<<name_db<<".new to "<<name_db<<" failed!"<<endl;
    exit(4);
  }
}

void op_convert()
{
  kvadrat kv;
  CDbReader db;
  workset.clear();
  db.open(name_db, f_db );
  while( db.read(kv) ) {
    workset.insert(kv);
  }
  save_work();
}

void op_uniq()
{
  kvadrat kv;
  CDbReader db;
  CDbReader inf;
  workset.clear();
  db.open(name_db, f_db );
  inf.open(name_in, f_in );
  unsigned long cntdup = 0;
  unsigned long cntin = 0;
  unsigned long cntdb = 0;
  while( inf.read(kv) ){
    workset.insert(kv);
    cntin++;
  }
  cout<<"Input set: "<<workset.size()<<endl;
  cout<<"local dup: "<<(cntin-workset.size())<<endl;
  while( db.read(kv) ) {
    cntdup += workset.erase(kv);
    cntdb++;
  }
  cout<<"db size: "<<cntdb<<endl;
  cout<<"global dup: "<<(cntdup)<<endl;
  cout<<"global new: "<<(workset.size())<<endl;
  if(f_write)
  {
    save_work();
    append_db();
  }
}

int main(int argc, char** argv)
{
  NamerCHDLK10::init();
  /* entropy analysis: 25 base58 chars, or 18 bytes */
  {
    f_convert=false;
    f_write=true;
    f_in=FMT_N58;
    f_out=0;
    f_db=FMT_BIN;
    name_db=name_in="";
		int c ;
		while( ( c = getopt (argc, argv, "dkenrhjc") ) != -1 )
		{
			switch(c)
			{
				case 'r': f_write= false; break;
				case 'c':	f_convert= true; break;

				case 'd': f_in=  FMT_DLK; break;
				case 'k':	f_in=  FMT_LK;	break;
				case 'e':	f_out= FMT_DLK;	break;
				case 'n':	f_out= FMT_N58;	break;
				case 'h':	f_db=  FMT_N58;	break;
				case 'j':	f_db=  FMT_DLK;	break;

				default:
					cerr << "Unknown Option" << endl << help_text;
					exit(2);
			}
		}

    if(!f_out){
      if(f_in==FMT_N58)
        f_out=FMT_N58;
        else f_out= FMT_DLK;
    }

    if(f_convert) {
      if(argc - optind != 2) {
        cerr<<"Expect 2 command line argument: database file name and input file name"<<endl<<help_text;
        exit(2);
      }
      name_db= argv[optind++];
      name_in= argv[optind++];
    } else {
      if(argc - optind != 2) {
        cerr<<"Expect 2 command line argument: input file name and output file name"<<endl<<help_text;
        exit(2);
      }
      name_db= argv[optind++];
      name_in= argv[optind++];
    }
  }

  if(f_convert){
    op_convert();
    return 0;
  } else {
    op_uniq();
  }
}
