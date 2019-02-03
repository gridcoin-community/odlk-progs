#include "namechdlk10.h"
#include "psnip/builtin/builtin.h"
using namespace std;
namespace NamerCHDLK10 {

  typedef array<unsigned, 10> Name;
  typedef array<char, 25> NameStr;

  const unsigned char debase58[128] = {
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,0,1,2,3,4,5,6,7,8,255,255,255,255,255,255,
    255,9,10,11,12,13,14,15,16,255,17,18,19,20,21,255,
    22,23,24,25,26,27,28,29,30,31,32,255,255,255,255,255,
    255,33,34,35,36,37,38,39,40,41,42,43,255,44,45,46,
    47,48,49,50,51,52,53,54,55,56,57,255,255,255,255,255
  };
  const unsigned char base58[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

  const int diag[67][por] = {
    1,0,3,2,6,7,4,5,9,8,
    1,0,3,2,6,7,4,8,9,5,
    1,0,3,2,6,7,5,4,9,8,
    1,0,3,2,6,7,8,9,4,5,
    1,0,3,2,6,7,8,9,5,4,
    1,0,3,2,6,8,4,9,5,7,
    1,0,3,2,6,8,4,9,7,5,
    1,0,3,2,6,8,5,9,4,7,
    1,0,3,2,6,8,5,9,7,4,
    1,0,3,2,6,8,7,4,9,5,
    1,0,3,2,6,8,9,4,7,5,
    1,0,3,2,6,8,9,5,7,4,
    1,0,3,4,2,6,8,9,5,7,
    1,0,3,4,2,6,8,9,7,5,
    1,0,3,4,2,7,5,6,9,8,
    1,0,3,4,2,7,5,8,9,6,
    1,0,3,4,2,7,8,9,5,6,
    1,0,3,4,2,7,8,9,6,5,
    1,0,3,4,6,2,5,8,9,7,
    1,0,3,4,6,2,8,5,9,7,
    1,0,3,4,6,2,8,9,5,7,
    1,0,3,4,6,8,2,9,7,5,
    1,0,3,4,6,8,5,9,2,7,
    1,0,3,4,6,8,7,9,2,5,
    1,0,3,4,6,8,7,9,5,2,
    1,0,3,4,7,2,8,9,5,6,
    1,0,3,4,7,2,8,9,6,5,
    1,0,3,4,7,8,5,9,2,6,
    1,0,3,4,7,8,5,9,6,2,
    1,0,3,4,8,7,5,9,2,6,
    1,0,3,4,8,7,5,9,6,2,
    1,0,3,4,8,9,5,6,2,7,
    1,0,3,4,8,9,5,6,7,2,
    1,0,3,7,6,8,5,9,2,4,
    1,0,3,7,6,8,5,9,4,2,
    1,0,3,7,8,9,2,6,4,5,
    1,0,3,7,8,9,2,6,5,4,
    1,2,0,4,6,3,5,9,7,8,
    1,2,0,4,6,3,7,9,5,8,
    1,2,0,4,6,7,8,9,3,5,
    1,2,0,4,7,8,5,9,3,6,
    1,2,0,4,7,8,5,9,6,3,
    1,2,0,4,7,8,9,3,6,5,
    1,2,0,4,7,8,9,5,6,3,
    1,2,0,4,7,9,8,5,3,6,
    1,2,0,4,7,9,8,6,5,3,
    1,2,3,0,6,7,8,9,5,4,
    1,2,3,0,6,7,9,4,5,8,
    1,2,3,4,0,7,5,9,6,8,
    1,2,3,4,0,7,8,9,5,6,
    1,2,3,4,0,9,5,6,7,8,
    1,2,3,4,6,0,8,9,7,5,
    1,2,3,4,6,7,5,9,0,8,
    1,2,3,4,6,8,9,5,0,7,
    1,2,3,7,6,8,5,9,0,4,
    1,2,3,7,6,9,5,4,0,8,
   
    1,0,3,2,6,7,5,8,9,4,
    1,0,3,4,6,2,8,9,7,5,
    1,0,3,4,6,7,8,9,2,5,
    1,0,3,4,6,7,8,9,5,2,
    1,0,3,4,6,8,5,9,7,2,
    1,0,3,4,6,8,9,5,2,7,
    1,0,3,4,6,8,9,5,7,2,
    1,0,3,4,8,6,9,5,2,7,
    1,2,0,4,6,7,8,9,5,3,
    1,2,3,0,6,7,5,9,4,8,
    1,2,3,4,6,9,8,0,5,7
  };

  const int diag2[67][por] = {
    1,0,3,2,6,7,4,5,9,8,
    1,0,3,2,6,7,4,8,9,5,
    1,0,3,2,6,7,5,4,9,8,
    1,0,3,2,6,7,8,9,4,5,
    1,0,3,2,6,7,8,9,5,4,
    1,0,3,2,6,8,4,9,5,7,
    1,0,3,2,6,8,4,9,7,5,
    1,0,3,2,6,8,5,9,4,7,
    1,0,3,2,6,8,5,9,7,4,
    1,0,3,2,6,8,7,4,9,5,
    1,0,3,2,6,8,9,4,7,5,
    1,0,3,2,6,8,9,5,7,4,
    1,0,3,4,2,6,8,9,5,7,
    1,0,3,4,2,6,8,9,7,5,
    1,0,3,4,2,7,5,6,9,8,
    1,0,3,4,2,7,5,8,9,6,
    1,0,3,4,2,7,8,9,5,6,
    1,0,3,4,2,7,8,9,6,5,
    1,0,3,4,6,2,5,8,9,7,
    1,0,3,4,6,2,8,5,9,7,
    1,0,3,4,6,2,8,9,5,7,
    1,0,3,4,6,8,2,9,7,5,
    1,0,3,4,6,8,5,9,2,7,
    1,0,3,4,6,8,7,9,2,5,
    1,0,3,4,6,8,7,9,5,2,
    1,0,3,4,7,2,8,9,5,6,
    1,0,3,4,7,2,8,9,6,5,
    1,0,3,4,7,8,5,9,2,6,
    1,0,3,4,7,8,5,9,6,2,
    1,0,3,4,8,7,5,9,2,6,
    1,0,3,4,8,7,5,9,6,2,
    1,0,3,4,8,9,5,6,2,7,
    1,0,3,4,8,9,5,6,7,2,
    1,0,3,7,6,8,5,9,2,4,
    1,0,3,7,6,8,5,9,4,2,
    1,0,3,7,8,9,2,6,4,5,
    1,0,3,7,8,9,2,6,5,4,
    1,2,0,4,6,3,5,9,7,8,
    1,2,0,4,6,3,7,9,5,8,
    1,2,0,4,6,7,8,9,3,5,
    1,2,0,4,7,8,5,9,3,6,
    1,2,0,4,7,8,5,9,6,3,
    1,2,0,4,7,8,9,3,6,5,
    1,2,0,4,7,8,9,5,6,3,
    1,2,0,4,7,9,8,5,3,6,
    1,2,0,4,7,9,8,6,5,3,
    1,2,3,0,6,7,8,9,5,4,
    1,2,3,0,6,7,9,4,5,8,
    1,2,3,4,0,7,5,9,6,8,
    1,2,3,4,0,7,8,9,5,6,
    1,2,3,4,0,9,5,6,7,8,
    1,2,3,4,6,0,8,9,7,5,
    1,2,3,4,6,7,5,9,0,8,
    1,2,3,4,6,8,9,5,0,7,
    1,2,3,7,6,8,5,9,0,4,
    1,2,3,7,6,9,5,4,0,8,
   
    1,0,3,2,6,7,5,8,9,4,
    1,0,3,4,6,2,8,9,7,5,
    1,0,3,4,6,7,8,9,2,5,
    1,0,3,4,6,7,8,9,5,2,
    1,0,3,4,6,8,5,9,7,2,
    1,0,3,4,6,8,9,5,2,7,
    1,0,3,4,6,8,9,5,7,2,
    1,0,3,4,8,6,9,5,2,7,
    1,2,0,4,6,7,8,9,5,3,
    1,2,3,0,6,7,5,9,4,8,
    1,2,3,4,6,9,8,0,5,7
  };
  const int index[por - 1][por - 2] = {
    1,2,3,4,5,6,7,8,
    10,12,13,14,15,16,17,19,
    20,21,23,24,25,26,28,29,
    30,31,32,34,35,37,38,39,
    40,41,42,43,46,47,48,49,
    50,51,52,53,56,57,58,59,
    60,61,62,64,65,67,68,69,
    70,71,73,74,75,76,78,79,
    80,82,83,84,85,86,87,89
  };

  typedef array<unsigned char,por> morfizm;

  map<morfizm,int> lin_diag;

  inline int is_sndlk(const kvadrat& kv){
    if(!is_lk(kv)) return -1;
    int flag = 0;
    morfizm tempm;
    for(int i = 0, t; i < por; i++){
      if(flag & (t = 1 << (tempm[i] = kv[(i + 1) * (por - 1)])) || kv[i * (por + 1)] != i) return -1;
      flag |= t;
    }
    auto q = lin_diag.find(tempm);
    if(q == lin_diag.end()) return -1;
    return q->second;
  }

  inline int get_fnum(int perest[]){
    int ret = 0, ob_perest[por - 2];
    for(int i = 0; i < por - 2; i++) ob_perest[perest[i]] = i;
    for(int r = por - 3, m, s; r; r--){
      m = perest[r];
      s = ob_perest[r];
      ret = ret * (r + 1) + s;
      if(m == r) continue;
      perest[s] = m;
      ob_perest[m] = s;
    }
    return ret;
  }

  inline void unget_fnum(int fnum, int perest[]){
    for(int i = 0; i < por - 2; i++) perest[i] = i;
    for(int r = 1, s; r < por - 2; fnum /= ++r){
      s = fnum % (r + 1);
      if(s == r) continue;
      perest[s] ^= perest[r];
      perest[r] ^= perest[s];
      perest[s] ^= perest[r];
    }
  }



  void init()
  {
    morfizm tempm;
    for(int i = 0; i < 67; i++){
      for(int j = 0; j < por; j++) tempm[j] = diag[i][j];
      lin_diag[tempm] = i;
    }
  }

  bool getName(const kvadrat& kv, Name& nm)
  {
    int lin= is_sndlk(kv);
    if(lin<0)
      return false;
    nm[9]=lin;

    int perest[por - 1][por - 2];
    for(int i = 0, c, m, n; i < por - 1; i++){
      c = 0;
      m = kv[i * (por + 1)];
      n = kv[(i + 1) * (por - 1)];
      for(int j = 0, t; j < por; j++){
        if(j == i || j + i == por - 1) continue;
        t = kv[i * por + j];
        perest[i][c++] = t - (t > m) - (t > n);
      }
    }
    for(int i = 0; i < por - 1; i++) nm[i] = get_fnum(perest[i]);
    return true;
  }

  void encodeName58(const Name& nm, NameStr& imya)
  {
    long long bnum[3], templl;
    const unsigned &lin = nm[9];
    const array<unsigned,10> &fnum = nm;
    bnum[0] = ((lin * osn + fnum[0]) * osn + fnum[1]) * osn + fnum[2];
    bnum[1] = (fnum[3] * osn + fnum[4]) * osn + fnum[5];
    bnum[2] = (fnum[6] * osn + fnum[7]) * osn + fnum[8];
    //imya.resize(25);
    templl = bnum[0]; for(int i = 8; i >= 0; i--){imya[i] = base58[templl % 58]; templl /= 58;}
    templl = bnum[1]; for(int i = 16; i >= 9; i--){imya[i] = base58[templl % 58]; templl /= 58;}
    templl = bnum[2]; for(int i = 24; i >= 17; i--){imya[i] = base58[templl % 58]; templl /= 58;}
  }


  bool getName58(const kvadrat& kv, NameStr& name58)
  {
    Name tmp;
    if(!getName(kv, tmp))
      return false;
    encodeName58(tmp,name58);
    return true;
  }

  bool decodeName58(const NameStr& name58, Name& nm)
  {
    unsigned &lin = nm[9];
    array<unsigned,10> &fnum= nm;
    array<unsigned char,25> imya;
    if(name58.size()!=25)
      return false;
    for(unsigned i=0; i<25; ++i) {
      if(name58[i]&0x80)
        return false;
      if(debase58[name58[i]]==255)
        return false;
      imya[i]=debase58[name58[i]];
    }
    long long templl;
    templl = imya[0];
    for(int i = 1; i < 9; i++) templl = templl * 58 + imya[i];
    fnum[2] = templl % osn; templl /= osn;
    fnum[1] = templl % osn; templl /= osn;
    fnum[0] = templl % osn; lin = unsigned(templl / osn);
    if(lin >= 67) return false;
    templl = imya[9];
    for(int i = 10; i < 17; i++) templl = templl * 58 + imya[i];
    fnum[5] = templl % osn; templl /= osn;
    fnum[4] = templl % osn; fnum[3] = unsigned(templl / osn);
    if(fnum[3] >= osn) return false;
    templl = imya[17];
    for(int i = 18; i < 25; i++) templl = templl * 58 + imya[i];
    fnum[8] = templl % osn; templl /= osn;
    fnum[7] = templl % osn; fnum[6] = unsigned(templl / osn);
    if(fnum[6] >= osn) return false;
    return true;
  }

  bool fromName(const Name& fnum, kvadrat& kv)
  {
    const unsigned &lin = fnum[9];
    int perest[por - 1][por - 2];
    for(int i = 0; i < por - 1; i++) unget_fnum(fnum[i], perest[i]);
    kvadrat tempk;
    for(int i = 0, m, n; i < por - 1; i++){
      tempk[i * (por + 1)] = m = i;
      tempk[(i + 1) * (por - 1)] = n = diag[lin][i];
      if(m > n){m ^= n; n ^= m; m ^= n;}
      for(int j = 0, t; j < por - 2; j++){
        t = perest[i][j];
        tempk[(int)index[i][j]] = t + (t >= m) + (t >= n - 1);
      }
    }
    tempk[raz - por] = diag[lin][por - 1]; tempk[raz - 1] = por - 1;
    unsigned long flag = 0, r;
    for(int i = 0, t; i < raz; i += por){
      if(flag & (t = 1 << tempk[i])) return false;
      flag |= t;
    }
    flag = 0;
    for(int i = 0, t; i < raz; i += por){
      if(flag & (t = 1 << tempk[i + por - 1])) return false;
      flag |= t;
    }
    for(int i = 1; i < por - 1; i++){
      flag = 0;
      for(int j = 0, t; j < raz - por; j += por){
        if(flag & (t = 1 << tempk[i + j])) return false;
        flag |= t;
      }
      flag ^= 0x3ff;
      psnip_intrin_BitScanForward(&r, flag);
      tempk[raz - por + i] = (char)r;
    }
    flag = 0;
    for(int i = 0, t; i < por; i++){
      if(flag & (t = 1 << tempk[raz - por + i])) return false;
      flag |= t;
    }
    kv=tempk;
    return true;
  }

  bool fromName58(const NameStr& name58, kvadrat& kv)
  {
    Name tmp;
    if(!decodeName58(name58,tmp))
      return false;
    return fromName(tmp,kv);
  }

}
