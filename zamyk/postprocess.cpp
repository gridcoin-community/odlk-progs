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

/* Program to post process results.
 * Finds additional CF ODLKs from the ones already in input file.
 * Appends the CF ODLKs to output file.
 * No uniqueness check is performed. For that, use ortogoncnt program.
 * This should really have been part of the boinc app.
*/

void load_squares(set<kvadrat>& sout, istream& fin)
{
	const unsigned raz_buf = 0x1000;
	char bufer[raz_buf];
  unsigned j = 0;
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
          if(is_lk(kv)) {
            sout.insert(kv);
          }
          j = 0;
        }
      }
    }
  } while(fin.gcount());
  if(j!=0)
  {
    cerr << "Warning: Input file possibly corrupt"<<endl;
  }
}

class Postprocess
{
  public:
  set<kvadrat> input, kan, mar, sum, mates;
  unsigned max_threads, nb_threads;
  string input_name = "input.txt";
  string output_name = "out_klpmd.txt";
  unsigned progress_of, progress_n, progress_nlast;
  time_t progress_last;
  unsigned stepnum;
  mutex cs_main;

  void parse_args(int argc, char** argv)
  {
    if(argc!=3) {
      cerr<<"Expect 2 command line arguments:"<<endl<<"input file name"<<endl<<"output file name"<<endl;
      exit(3);
    }
    input_name=argv[1];
    output_name=argv[2];
    max_threads = std::thread::hardware_concurrency();
    progress_last=time(0);
  }

  void init()
  {
    ifstream fin(input_name, ios::binary);
    load_squares(input,fin);
    
    cerr <<"Read "<<input.size()<<" squares from input"<<endl;
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
    if(now-progress_last < 60) return;
    float pct= ((float)progress_n / (float)progress_of)*100;
    float rate= (progress_n - progress_nlast) / static_cast<float>(now-progress_last);
    ostringstream ss;
    ss.setf(std::ios_base::fixed, std::ios_base::floatfield);
    ss<<"Progress: step "<<stepnum<<" ";
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

  void kanon(set<kvadrat>& out, set<kvadrat>& in)
  {
    Kanonizator_lk kanonizator{};
    kvadrat kf;
    for(auto q = in.begin(); q != in.end(); ++q){
      kanonizator.kanon(*q, kf);
      out.insert(kf);
    }
  }

  class Mariazhne : public Trans_DLx {
    public:
    void search_mar(list<kvadrat>& out, kvadrat lk) {
      search_trans(lk);
      if(cnt_trans <= 1) return;
      kvadrat tempk[Trans_DLx::ch_srez - 1];
      const kvadrat* srez[Trans_DLx::ch_srez] = {&lk, &tempk[0], &tempk[1]};
      for(int i = 0; i < raz; i += por) for(int j = 0; j < por; j++) tempk[0][i + lk[i + j]] = j;
      for(int j = 0; j < por; j++) for(int i = 0; i < por; i++) tempk[1][lk[i * por + j] * por + j] = i;
      search_symm_trans(srez);
      for(int i = 0; i < Trans_DLx::ch_srez; i++){
        for(auto q = kf_trans[i].begin(); q != kf_trans[i].end(); q++){
          find_d_trans(q->second, trans[i]);
          if(is_mar()){
            out.push_back(q->first);
          }
        }
      }
    }
  };

  static void set_difference(set<kvadrat>& out, set<kvadrat>& a, set<kvadrat>& b)
  {
    std::set_difference(a.begin(),a.end(),b.begin(),b.end(),std::inserter(out, out.end()));
  }

  void work()
  {
    stepnum=0;
    while(!input.empty())
    {
      cerr<<"Iteration "<<(stepnum/2)<<" input set: "<<input.size()<<endl;
      kanon(kan, input);
      input.clear();
      cerr<<"cf "<<kan.size()<<endl;

      batch_parallel( kan,
        [this](set<kvadrat>::iterator it, set<kvadrat>::iterator it_end, bool fpro ){
          list<kvadrat> pout;
          Mariazhne mari{};
          mari.f_simm= true; //disable filter
          for( ; it != it_end; ++it) {
            mari.search_mar( pout, *it );
            if(fpro) progress();
          }
          lock_guard<mutex> lock(cs_main);
          mar.insert(pout.begin(),pout.end());
          sum.insert(pout.begin(),pout.end());
        }
      );

      cerr<<"mar "<<mar.size()<<endl;
      cerr<<"sum "<<sum.size()<<endl;
      kan.clear();
      stepnum++;

      batch_parallel( mar,
        [this](set<kvadrat>::iterator q, set<kvadrat>::iterator it_end, bool fpro ){
          // give only the co-squares produced by otrogon
          list<kvadrat> lm;
          list<kvadrat> lmkf;
          Exact_cover exact_cover{};
          Kanonizator_lk kanonizator{};
          for(; q != it_end; ++q) {
            lm.clear();
            if(!is_dlk(*q)){
              cerr<<"Warning: non-DLS in Mar set!"<<endl;
              continue;
            }
            exact_cover.search_mate(*q, lm);
            //if(mates.empty()) cerr<<"Warning: square has no mates!"<<endl;
            for(auto w = lm.begin(); w != lm.end(); ++w) {
              kvadrat kf;
              kanonizator.kanon(*w, kf);
              lmkf.push_back(kf);
            }
          }
          lock_guard<mutex> lock(cs_main);
          mates.insert(lmkf.begin(),lmkf.end());
        }
      );

      cerr<<"cf mates "<<mates.size()<<endl;
      mar.clear();

      set_difference(input,mates,sum);
      sum.insert(input.begin(),input.end());
      cerr<<"sum "<<sum.size()<<endl;

      stepnum++;
    }

    cerr<<"Second stage"<<endl;
    kanon(kan, sum);
    cerr<<"cf "<<kan.size()<<endl;
    batch_parallel( kan,
      [this](set<kvadrat>::iterator it, set<kvadrat>::iterator it_end, bool fpro ){
        list<kvadrat> pout;
        Mariazhne mari{};
        mari.f_simm= true; //disable filter
        for( ; it != it_end; ++it) {
          mari.search_mar( pout, *it );
          if(fpro) progress();
        }
        lock_guard<mutex> lock(cs_main);
        mar.insert(pout.begin(),pout.end());
      }
    );
    // mar -> KF ODLK
  }

  void output()
  {
    cout<<"New Fancy CF:"<<mar.size()<<endl;
    ofstream fout(output_name, ios::binary | ios::app);
    for(auto q=mar.begin(); q!=mar.end(); ++q)
      out_kvadrat(fout, *q);
    cerr<<"Results appended to: "<<output_name<<endl;
  }
};

int main(int argc, char** argv)
{
  Postprocess o;
  o.parse_args(argc,argv);
  o.init();
  o.work();
  o.output();
}
