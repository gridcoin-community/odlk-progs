struct Input {
	int rule;
	kvadrat start;
	int min_level;
	bool skip;
	bool skip_below;
	bool skip_fast;
	typedef std::bitset<67> ruleset;
	ruleset skip_rule;
	unsigned long long lim_sn;
	unsigned long lim_kf;
	void readInput(CStream& s);
	void writeInput(CStream& s);
};

struct State : Input {
	kvadrat next;
	kvadrat last_kf;
	unsigned long long nsn;
	unsigned long nkf;
	unsigned long long ndaugh;
	unsigned long nkf_skip_below, nkf_skip_rule;
	signed long max_trans;
	unsigned long long ntrans;
	unsigned interval_rsm; // count of resumptions in this interval
	time_t interval_t; // ?
	bool ended; // the min_level was reached - do not resume
	vector<NamerCHDLK10::NameBin> odlk;
	long userid;
	void readInput(CStream& s);
	void readState(CStream& s);
	void writeState(CStream& s);
};

void readNameBin(CStream& s, kvadrat& kv) {
	NamerCHDLK10::NameBin nb;
	NamerCHDLK10::Name nm;
	s.read(nb.data(),nb.size());
	NamerCHDLK10::decodeNameBin(nb, nm);
	if(!NamerCHDLK10::fromName(nm,kv))
		s.fail();
}

void writeNameBin(CStream& s, kvadrat& kv)
{
	//bool getNameBin(const kvadrat& kv, unsigned char *nb)
  NamerCHDLK10::Name nm;
	NamerCHDLK10::NameBin bin;
  if(!NamerCHDLK10::getName(kv,nm))
    s.fail();
	NamerCHDLK10::encodeNameBin(nm,bin);
	s.write(bin.data(),bin.size());
  return;
}

void Input::readInput(CStream& s) {
	using std::bitset;
	unsigned ident= s.r4();
	if(ident!=0x34603BA1) s.fail();
	rule= s.r1();
	readNameBin(s, start);
	min_level= s.r1();
	unsigned flag=s.r1();
	skip_below= !!(flag&1);
	skip_fast= !!(flag&2);
	if(flag&8) s.fail();
	skip_rule = ruleset(flag>>4)<<64;
	unsigned x= s.r4(); skip_rule |= ruleset(x)<<32;
	x= s.r4(); skip_rule |= ruleset(x);
	lim_sn= s.r6();
	lim_kf= s.r4();
	skip = skip_below || skip_rule.any();	
}

void Input::writeInput(CStream& s) {
	s.w4(0x34603BA1);
	s.w1(rule);
	writeNameBin(s, start);
	s.w1(min_level);
	unsigned flag = (skip_below) | (skip_fast<<1);
	flag |= ((skip_rule>>64)&ruleset(4)).to_ulong();
	s.w1(flag);
	s.w4(((skip_rule>>32)&ruleset(0xFFFFFFFF)).to_ulong());
	s.w4((skip_rule&ruleset(0xFFFFFFFF)).to_ulong());
	s.w6(lim_sn);
	s.w4(lim_kf);
}


void State::writeState(CStream& s) {
	writeInput(s);
	s.w6(nsn);
	s.w4(nkf);
	s.w6(ndaugh);
	s.w4(nkf_skip_below);
	s.w4(nkf_skip_rule);
	s.w2(max_trans);
	s.w6(ntrans);
	s.w2(interval_rsm);
	s.w6(interval_t);
	s.w1(!!ended);
	s.w4(userid);
	if(!ended)
		writeNameBin(s, next);
	if(nkf)
		writeNameBin(s, last_kf);
	s.w4(odlk.size());
	for( const NamerCHDLK10::NameBin& bin : odlk) {
		s.write(&bin, sizeof(NamerCHDLK10::NameBin));
	}
}

void State::readInput(CStream& s) {
	Input::readInput(s);

	nsn= ndaugh= ntrans= 0;
	nkf= nkf_skip_below= nkf_skip_rule= 0;
	max_trans= -1;
	interval_rsm= 0;
	interval_t= 0;//?
	ended= 0;
	last_kf={0};
	odlk.clear();
	next = start;
}

void State::readState(CStream& s) {
	readInput(s);
	nsn= s.r6();
	nkf= s.r4();
	ndaugh= s.r6();
	nkf_skip_below= s.r4();
	nkf_skip_rule= s.r4();
	max_trans= s.r2();
	ntrans= s.r6();
	interval_rsm= s.r2();
	interval_t= s.r6();
	ended= !!s.r1();
	userid= s.r4();
	if(!ended)
		readNameBin(s, next);
	if(nkf)
		readNameBin(s, last_kf);
	unsigned cnt= s.r4();
	odlk.resize(cnt);
	for(unsigned i=0; i<cnt; ++i) {
		s.read(&odlk[i], sizeof(NamerCHDLK10::NameBin));
	}
}
