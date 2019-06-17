#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <ctime>
#include <unistd.h>
#include <cstdlib>
#include <sys/stat.h>

#include "odlkcommon/common.h"
#include "odlkcommon/namechdlk10.h"
#include "Stream.cpp"

#include "odlkcommon/namechdlk10.cpp"
#include "odlkcommon/kvio.cpp"

#include "wio.cpp"

int main(void) {
	NamerCHDLK10::init();
	Input inp = {
		.rule=51,
		.start= {
			0,2,3,4,5,6,7,8,9,1,
			3,1,0,6,7,4,5,9,2,8,
			1,0,2,7,6,8,9,3,4,5,
			2,5,1,3,8,9,4,0,6,7,
			5,3,7,9,4,0,8,2,1,6,
			6,4,8,0,9,5,2,1,7,3,
			7,8,9,5,0,1,6,4,3,2,
			4,9,6,8,1,2,3,7,5,0,
			9,7,5,1,2,3,0,6,8,4,
			8,6,4,2,3,7,1,5,0,9,
		},
		.min_level=45,
		.skip_below=0,
		.skip_fast=0,
		.skip_rule={},
		.lim_sn=0xFFFFFFFFFFFF,
		.lim_kf=0xFFFFFFFF,
	};
	CDynamicStream buf;
	inp.writeInput(buf);
	FILE* f = fopen("input.dat", "w");
	if(!f)
		throw std::runtime_error("fopen");
	auto rc = fwrite(buf.getbase(), 1, buf.length(), f);
	if( rc !=buf.length())
		throw std::runtime_error("fwrite");
	fclose(f);
	return 0;
}
