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

unsigned load_squares(set<kvadrat>& sout, istream& fin)
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
          sout.insert(kv);
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
"ortogoncnt.exe -wco input\n"
" -w : write sorted and unique cf odls back to input file\n"
" -c : count cf odls grouped by number of their co-squares\n"
" -o : write out_ortogon.txt and out_kf_N.txt files\n"
" input : database of fancy diagonal latin squares\n"
;

class Ortocnt
{
  public:
  set<kvadrat> mar, mates;
  map<kvadrat, list<kvadrat>> ortomap;
  map<unsigned, set<kvadrat>> ortocnt;
  string input_name = "input.txt";
  unsigned progress_of, progress_n, progress_nlast;
  unsigned max_threads, nb_threads;
  time_t progress_last;
  bool f_writeback, f_ortogen, f_out;
  mutex cs_main;

  void parse_args(int argc, char** argv)
  {
		int c ;
    f_writeback= f_ortogen= f_out= false;
		while( ( c = getopt (argc, argv, "wco") ) != -1 ) 
		{
			switch(c)
			{
				case 'w':
					f_writeback = true;
					break;
				case 'c':
					f_ortogen= true;
					break;
				case 'o':
					f_out = true;
					break;
				default:
					cerr << "Unknown Option" << endl << help_text;
					exit(2);
			}
		}

    if(argc - optind != 1) {
      cerr<<"Expect 1 command line argument: input file name"<<endl << help_text;
      exit(2);
    }
    input_name=argv[optind++];

    max_threads = std::thread::hardware_concurrency();
    progress_last=time(0);
  }

  void init()
  {
    ifstream fin(input_name, ios::binary);
    unsigned lcnt= load_squares(mar,fin);
    cout << "Squares in input: "<<lcnt<<endl;
    cout << "Unique: "<<mar.size()<<endl;
    cout << "Duplicate: "<<(lcnt-mar.size())<<endl;
  }

  void writeback()
  {
    if(!f_writeback)
      return;
    string tmpname=input_name+".tmp";
    {
      ofstream fout(tmpname, ios::binary);
      for(auto q=mar.begin(); q!=mar.end(); ++q)
        out_kvadrat(fout, *q);
      fout.flush();
      if(!fout.good()) {
        cerr<<"Wrtiting "<<tmpname<<" error!"<<endl;
        exit(4);
      }
    }

    if(0!=rename(tmpname.c_str(),input_name.c_str())) {
      cerr<<"Wrtiting "<<input_name<<" error!"<<endl;
      exit(4);
    }
    cerr<<input_name<<": sorted and duplicates removed"<<endl;
  }

  void batch_parallel(
    set<kvadrat>& in,
    const std::function< void( set<kvadrat>::iterator, set<kvadrat>::iterator, bool) >& fun)
  {
    set<kvadrat>::iterator it = in.begin();
		nb_threads = max_threads;
    nb_threads = std::min( (size_t)nb_threads, in.size() / 4 );
		nb_threads = std::max(nb_threads , 1U);
		unsigned batch_size = in.size() / nb_threads;
		unsigned batch_reminder = in.size() % nb_threads;
    progress_of = batch_size+batch_reminder;
    progress_n = progress_nlast = 0;
    std::vector< std::thread > my_threads(nb_threads-1);

		for(unsigned thi = 0; thi < nb_threads-1; ++thi)
		{
      auto ib=it;
      std::advance(it, batch_size);
			my_threads[thi] = std::thread( fun, ib, it, false );
		}

		fun(it, in.end(), true );

		for(unsigned thi = 0; thi < nb_threads-1; ++thi)
		{
			my_threads[thi].join();
    }
  }

  inline void progress()
  {
    progress_n++;
    if(progress_n-progress_nlast < 400) return;
    time_t now= time(0);
    if(now-progress_last < 30) return;
    float pct= ((float)progress_n / (float)progress_of)*100;
    float rate= (progress_n - progress_nlast) / static_cast<float>(now-progress_last);
    ostringstream ss;
    ss.setf(std::ios_base::fixed, std::ios_base::floatfield);
    ss<<"Progress: ";
    ss.precision(1);
    ss.width(5);
    ss<<pct<<"% ";
    ss.precision(0);
    ss.width(4);
    ss<<rate<<"/s * "<<nb_threads<<" threads"<<endl;
    cerr<<ss.str();
    progress_last=now;
    progress_nlast=progress_n;
  }

  void work()
  {
    if(!f_ortogen)
      return;
    batch_parallel( mar,
      [this](set<kvadrat>::iterator q, set<kvadrat>::iterator it_end, bool fpro ){
        Exact_cover exact_cover{};
        Kanonizator_lk kanonizator{};
        for(; q != it_end; ++q) {
          list<kvadrat> m;
          if(!is_dlk(*q)){
            cerr<<"Warning: non-DLS in Mar set!"<<endl;
            continue;
          }
          exact_cover.search_mate(*q, m);
          {
            lock_guard<mutex> lock(cs_main);
            ortomap.emplace(*q,m);
          }
          for(auto w = m.begin(); w != m.end(); ++w) {
            kvadrat kf;
            kanonizator.kanon( *w, kf );
            lock_guard<mutex> lock(cs_main);
            mates.insert(kf);
          }
          if(fpro) progress();
        }
      }
    );

    for(auto q = ortomap.begin(); q != ortomap.end(); ++q) {
      ortocnt [q->second.size()] . insert ( q->first );
    }
  }

  void counts()
  {
    if(!f_ortogen)
      return;
    cout<<"Fancy CF: "<<mar.size()<<endl;
    cout<<"Found Fancy CF: "<<endl;
    unsigned scount=0;
    for(auto q=ortocnt.begin(); q!=ortocnt.end(); ++q){
      cout<<"count["<<q->first<<"]: "<<q->second.size()<<endl;
      if(q->second.size()) scount+=q->second.size();
    }
    cout<<"All: "<<scount<<endl;
    cout<<"Found CF co-squares: "<<mates.size()<<endl;
  }

  void output()
  {
    if(!f_out)
      return;
    {
      // ortogon
      ofstream fout("out_ortogon.txt", ios::binary);
      for(auto q=ortomap.begin(); q!=ortomap.end(); ++q){
        fout<<"DLK("<<q->second.size()<<"):\r\n";
        out_kvadrat(fout, q->first);
        unsigned ix=1;
        for(auto w=q->second.begin(); w!=q->second.end(); ++w, ++ix){
          fout<<"mate#"<<ix<<"\r\n";
          out_kvadrat(fout,*w);
        }
      }
      cerr<<"Written file: "<<"out_ortogon.txt"<<endl;
    }
    for(auto q=ortocnt.begin(); q!=ortocnt.end(); ++q) {      
      // CF ODLK by group size
      ostringstream ss;
      ss << "out_kf_" << q->first << ".txt";
      ofstream fout(ss.str(), ios::binary);
      for(auto w=q->second.begin(); w!=q->second.end(); ++w){
        out_kvadrat(fout, *w);
      }
      cerr<<"Written file: "<<ss.str()<<endl;
    }
    {
      // kf mates
      ofstream fout("out_kf_mates.txt", ios::binary);
      for(auto q=mates.begin(); q!=mates.end(); ++q){
        out_kvadrat(fout,*q);
      }
      cerr<<"Written file: "<<"out_kf_mates.txt"<<endl;
    }
  }
};

int main(int argc, char** argv)
{
  Ortocnt o;
  o.parse_args(argc,argv);
  o.init();
  o.writeback();
  o.work();
  o.counts();
  o.output();
}
