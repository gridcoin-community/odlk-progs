#pragma once
#include <vector>
#include <array>
#include <list>
#include <set>
#include <iostream>


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
