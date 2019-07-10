#pragma once
#include <array>
#include <map>
#include "odlkcommon/common.h"

namespace NamerCHDLK10 {

  typedef std::array<unsigned, 10> Name;
  typedef std::array<char, 25> NameStr;
  typedef std::array<unsigned char, 18> NameBin;

  extern const unsigned char debase58[128];
  extern const unsigned char base58[];

  static const long long osn = 40320; // factorial(8)

  void init();

  bool getName(const kvadrat& kv, Name& nm);
  void encodeName58(const Name& nm, NameStr& imya);
	void encodeNameBin(const Name& nm, NameBin& bin);
  bool getName58(const kvadrat& kv, NameStr& name58);

  bool decodeName58(const NameStr& name58, Name& nm);
	void decodeNameBin(const NameBin& bin, Name& nm);
  bool fromName(const Name& fnum, kvadrat& kv);
  bool fromName58(const NameStr& name58, kvadrat& kv);

}
