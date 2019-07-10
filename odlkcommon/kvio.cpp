#include "odlkcommon/common.h"
#include "odlkcommon/namechdlk10.h"

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

bool CSquareReader::fromNameBin(unsigned char* nb, kvadrat& kv)
{
  NamerCHDLK10::Name nm;
	NamerCHDLK10::NameBin nb2; std::copy(nb,nb+18,nb2.begin());
	NamerCHDLK10::decodeNameBin(nb2, nm);
  //decode
  return NamerCHDLK10::fromName(nm,kv);
}

bool CSquareReader::read(kvadrat& kv)
{
  using NamerCHDLK10::debase58;
  if(format!=BINARY)
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
	  std::cerr<<"I/O Error reading file"<<std::endl;
	  error=1; return false;
	}
      }
      else ch=tmp;

      if(ch=='\n') ++line;

      if(state==sreml && ch=='\n' ) state=snothing;
      else if(state==sremb && ch==']' ) state=snothing;
      else if(state==snothing && (ch=='#' || ch==';')) state=sreml;
      else if(state==snothing && ch=='[') state=sremb;
      else if(state<=stext && format==TEXT && ch >= '0' && ch <= '9' )
      {
	state=stext;
	kv[kvi++] = ch - '0';
	if(kvi==raz) {
	  state=snothing;
	  kvi=0;
	  if(is_lk(kv)) {
	    /*if(format==FMT_LK) {
	      kvadrat tmp=kv;
	      Kanonizator_dlk::kanon(tmp,kv);
	    }*/
	    return true;
	  }
	}
      }
      else if(format==NAME58 && (state<=sname) && !(ch&0x80) && debase58[ch]!=255 )
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
      else if(state==sreml || state==sremb) {} //ignore
      else if(state==snothing && ch==255) return false; //EOF
      else
      {
	std::cerr<<"Invalid character '"<<int(ch)<<"' on line "<<line<<std::endl;
	error=2; return false;
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
      std::cerr<<"Corrupted Binary Database"<<std::endl;
      error=3; return false;
    }
    if(!fromNameBin((unsigned char*)nameB,kv)) {
      std::cerr<<"Corrupted Binary Database"<<std::endl;
      error=4; return false;
    }
    return true;
  }
}


static bool getNameBin(const kvadrat& kv, unsigned char *nb)
{
  NamerCHDLK10::Name nm;
  NamerCHDLK10::NameBin nb2;
  if(!NamerCHDLK10::getName(kv,nm))
    return false;
	NamerCHDLK10::encodeNameBin(nm, nb2);
	std::copy(nb2.begin(), nb2.end(), nb); // Angry at c++
  return true;
}


void write_square(std::ostream& out, const kvadrat& kv, const CSquareReader::format_t format)
{
  if(format==CSquareReader::TEXT) {
    out_kvadrat(out, kv);
  } else
  if(format==CSquareReader::NAME58) {
    NamerCHDLK10::NameStr name;
    if(!NamerCHDLK10::getName58(kv,name)){
      cerr<<"Failed to get sndlk name for square!"<<endl;
      out.setstate(std::ios::failbit);
    }
		out.write(name.data(),name.size());
    out<<endl;
  } else
  if(format==CSquareReader::BINARY) {
    unsigned char bin[18];
    if(!getNameBin(kv,bin)){
      cerr<<"Failed to get sndlk name for square!"<<endl;
      out.setstate(std::ios::failbit);
    }
    out.write((char*)bin,18);
  }
}

void write_squares(std::ostream& out, const std::set< kvadrat>& kvs, const CSquareReader::format_t format)
{
  for(auto q= kvs.cbegin(); q!=kvs.cend() && out.good(); ++q)
    write_square(out,*q,format);
}
void write_squares(std::ostream& out, const std::list<kvadrat>& kvs, const CSquareReader::format_t format)
{
  for(auto q= kvs.cbegin(); q!=kvs.cend() && out.good(); ++q)
    write_square(out,*q,format);
}
