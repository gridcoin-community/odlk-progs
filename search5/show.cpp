#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <ctime>
#include <unistd.h>
#include <cstdlib>
#include <iomanip>
#include <sys/stat.h>

#include "odlkcommon/common.h"
#include "odlkcommon/namechdlk10.h"
#include "Stream.cpp"

#include "odlkcommon/namechdlk10.cpp"
#include "odlkcommon/kvio.cpp"
#include "wio.cpp"

using namespace std;

void readFile(const char* fn, CDynamicStream& buf) {
		FILE* f = fopen(fn, "r");
		if(!f) {
			//if(errno==ENOENT) throw EFileNotFound();
			throw std::runtime_error("fopen");
		}
		struct stat stat_buf;
		if(fstat(fileno(f), &stat_buf)<0)
			throw std::runtime_error("fstat");
		buf.setpos(0);
		buf.reserve(stat_buf.st_size);
		if( fread(buf.getbase(), 1, stat_buf.st_size, f) !=stat_buf.st_size)
			throw std::runtime_error("fread");
		buf.setpos(0);
		fclose(f);
}

int main(int argc, char** argv) {
	NamerCHDLK10::init();
	CDynamicStream buf;
	State inp;
	if(argc != 2) {
		cerr<<"Expect 1 command line argument: input file name"<<endl;
		exit(2);
	}
	//std::string input_name {argv[1]};
	try {
		readFile(argv[1],buf);
		inp.readState(buf);
	} catch(EStreamOutOufBounds& e) {
		cerr<<"Invalid format of input file"<<endl;
		exit(1);
	} catch(std::exception& e) {
		cerr<<"Cant open/read input file, "<<e.what()<<endl;
		exit(1);
	}
	cout<<"#rule "<<inp.rule<<endl;
		int rule;
	cout<<"#start "; write_square(cout, inp.start, CSquareReader::NAME58);
	cout<<"#min_level "<<inp.min_level<<endl;
	cout<<"#skip_below "<<inp.skip_below<<endl;
	cout<<"#skip_fast "<<inp.skip_fast<<endl;
	cout<<"#skip_rule "<<inp.skip_rule.to_string()<<endl;
	cout<<"#lim_sn "<<inp.lim_sn<<endl;
	cout<<"#lim_kf "<<inp.lim_kf<<endl;

	cout<<"#n_sn "<<inp.nsn<<endl;
	cout<<"#n_kf "<<inp.nkf<<endl;
	cout<<"#n_daugh "<<inp.ndaugh<<endl;
	cout<<"#n_daugh_avg "<<(double(inp.ndaugh)/double(inp.nkf))<<endl;
	cout<<"#ended "<<inp.ended<<endl;
	if(!inp.ended)
		cout<<"#next "; write_square(cout, inp.next, CSquareReader::NAME58);
	if(inp.nkf)
		cout<<"#last_kf "; write_square(cout, inp.last_kf, CSquareReader::NAME58);
	cout<<"#n_kf_skip_below "<<inp.nkf_skip_below<<endl;
	cout<<"#n_kf_skip_below "<<inp.nkf_skip_below<<endl;
	cout<<"#max_trans "<<inp.max_trans<<endl;
	cout<<"#n_trans "<<inp.ntrans<<endl;
	cout<<"#n_trans_avg "<<(double(inp.ntrans)/double(inp.nkf))<<endl;
	cout<<"#interval_rsm "<<inp.interval_rsm<<endl;
	cout<<"#userid "<<inp.userid<<endl;
	cout<<"#n_odlk "<<inp.odlk.size()<<endl;
	for(const NamerCHDLK10::NameBin& odlk_n : inp.odlk) {
		kvadrat odlk;
		int rule, k;
		NamerCHDLK10::Name nm;
		NamerCHDLK10::NameStr nms;
		NamerCHDLK10::decodeNameBin(odlk_n, nm);
		NamerCHDLK10::encodeName58(nm,nms);
		rule=nm[9]+1;
		k=0;
		/*if(NamerCHDLK10::fromName(nm,odlk)) {
			cout<<"# rule="<<rule<<" k="<<k<<endl;
			write_square(cout,odlk,CSquareReader::NAME58);
		} else cout<<"#odlk with invalid name??";*/
		cout .write(nms.data(),nms.size()) <<" # rule="
		<<setfill('0') <<setw(2) <<rule
		/*<<" k=" <<setw(2) <<k*/
		<<endl;
	}
	return 0;
}
