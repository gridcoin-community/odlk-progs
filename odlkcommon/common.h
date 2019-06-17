#pragma once
#include <vector>
#include <array>
#include <list>
#include <set>
#include <iostream>
#include <fstream>


const int por = 10;
const int raz = por * por;

typedef std::array<unsigned char, raz> kvadrat;

inline bool is_lk(const kvadrat& kvad){
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
	return true;
}

bool in_kvadrat(std::istream& in, kvadrat& kvad);
void out_kvadrat(std::ostream& out, const kvadrat& kv);


class CSquareReader
{
  public:
  std::ifstream fin;
  enum format_t {TEXT, BINARY, NAME58};
  enum format_t format;
  unsigned long line;
  int error;
  
  void open(std::string name, format_t iformat)
  {
    format=iformat;
    fin= std::ifstream{ name, std::ios::binary };
    line=1;
    error=0;
  }

  static bool fromNameBin(unsigned char *nb, kvadrat& kv);
  bool read(kvadrat& kv);
};

void write_square(std::ostream& out, const kvadrat& kv, const CSquareReader::format_t format);
void write_squares(std::ostream& out, const std::set< kvadrat>& kvs, const CSquareReader::format_t format);
void write_squares(std::ostream& out, const std::list<kvadrat>& kvs, const CSquareReader::format_t format);
