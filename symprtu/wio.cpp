struct TInput {
	uint64_t start;
	uint64_t end;
	unsigned upload;
	unsigned short mine_k; //min even k
	unsigned short mino_k; //min odd k
	unsigned short max_k;
	int twin_k; //shortest twin sequence to record
	int twin_cnt_k;
	unsigned short twin_min_k; //for symmetric twin prime tuples
	unsigned short twin_max_k;
	bool exit_early;
	bool out_last_primes;
	bool out_all_primes;
	vector<uint64_t> primes_in;

	void readInput(CStream&& s);
	void writeInput(CStream& s);
};

struct TOutputTuple {
	uint64_t start;
	short k;
	vector<int> ofs;
};

struct TOutput {
	uint64_t start;
	uint64_t chkpt; // last-k-th prime
	uint64_t last;  // last prime
	unsigned long nprime; //count of primes generated
	vector<uint64_t> primes;
	vector<TOutputTuple> tuples;
	vector<TOutputTuple> twins;
	vector<TOutputTuple> twin_tuples;
	std::map<unsigned,unsigned long> twin_cnt;
	enum Status {
		x_end =1,
		x_chkpt,
		x_time,
		x_cpu,
		x_abort
	} status;
	unsigned sieve_init_cs; //time to init

	void readOutput(CStream&& s);
	void writeOutput(CStream& s);
	void readOutput_OLD(CStream&& s);
};


void TInput::readInput(CStream&& s) {
	unsigned ident= s.r4();
	if(ident!=0x64DE70F6 && ident!=0x64DE70FA) s.fail();
	start= s.r8();
	end= s.r8();
	upload = s.r2();
	max_k = s.r1();
	mine_k = s.r1();
	unsigned flag=s.r1();
	exit_early= (flag >> 1) &1;
	out_last_primes= (flag >> 2) &1;
	out_all_primes= (flag >> 3) &1;
	unsigned len= s.r2();
	if(len) s.fail();
	if(ident==0x64DE70FA) {
		mino_k = s.r1();
		twin_k = s.r1();
		twin_cnt_k = s.r1();
		twin_min_k = s.r1();
		twin_max_k = s.r1();
	} else mino_k=mine_k+1;
}

void TInput::writeInput(CStream& s) {
	s.w4(0x64DE70FA);
	s.w8(start);
	s.w8(end);
	s.w2(upload);
	s.w1(max_k);
	s.w1(mine_k);
	unsigned flag = (exit_early<<1) | (out_last_primes<<2) |(out_all_primes<<3);
	s.w1(flag);
	if(!primes_in.empty()) s.fail();
	s.w2(0);
	s.w1(mino_k);
	s.w1(twin_k);
	s.w1(twin_cnt_k);
	s.w1(twin_min_k);
	s.w1(twin_max_k);
}


void TOutput::writeOutput(CStream& s) {
	s.w4(0x64DE70F9);
	s.w8(start);
	s.w8(chkpt);
	s.w8(last);
	s.w4(nprime);
	s.w4(primes.size());
	for( const auto& p : primes )
		s.w8(p);
	s.w4(tuples.size());
	for( const auto& t : tuples ) {
		s.w8(t.start);
		s.w1(t.k);
		for( const auto& o : t.ofs )
			s.w2(o);
	}
	s.w1(status);
	s.w4(sieve_init_cs);
	s.w4(twins.size());
	for( const auto& t : twins ) {
		s.w8(t.start);
		s.w2(t.ofs.size());
		for( const auto& o : t.ofs )
			s.w2(o);
	}
	s.w4(twin_tuples.size());
	for( const auto& t : twin_tuples ) {
		s.w8(t.start);
		s.w1(t.k);
		for( const auto& o : t.ofs )
			s.w2(o);
	}
	s.w4(0); //twin_cnt
	s.w4(0); //CRC here TODO
}

static void TOutput__readTuples(CStream& s, vector<TOutputTuple>& tuples, bool flag=false) {
	unsigned len= s.r4();
	tuples.resize(len);
	for(unsigned i=0; i<len; ++i) {
		tuples[i].start=s.r8();
		if(flag) {
			tuples[i].k=0;
			tuples[i].ofs.resize(s.r2());
		} else {
			unsigned k= tuples[i].k = s.r1();
			tuples[i].ofs.resize((k+1)/2);
		}
		for(unsigned j=0; j<tuples[i].ofs.size(); ++j)
			tuples[i].ofs[j]= s.r2();
	}
}

void TOutput::readOutput(CStream&& s) {
	unsigned ident= s.r4();
	if(ident!=0x64DE70F8 && ident!=0x64DE70F9) s.fail();
	start= s.r8();
	chkpt= s.r8();
	last= s.r8();
	nprime= s.r4();
	unsigned len = s.r4();
	primes.resize(len);
	for(unsigned i=0; i<len; ++i)
		primes[i]= s.r8();
	TOutput__readTuples(s, tuples);
	status= TOutput::Status(s.r1());
	sieve_init_cs= s.r4();
	if(ident==0x64DE70F9) {
		TOutput__readTuples(s, twins,1);
		TOutput__readTuples(s, twin_tuples);
	}
	//twin_cnt, CRC
}

void TOutput::readOutput_OLD(CStream&& s) {
	unsigned ident= s.r4();
	if(ident!=0x64DE70F7) s.fail();
	start= s.r8();
	chkpt= s.r8();
	last= s.r8();
	nprime= s.r4();
	unsigned len = s.r4();
	primes.resize(len);
	for(unsigned i=0; i<len; ++i)
		primes[i]= s.r8();
	TOutput__readTuples(s, tuples);
	status= TOutput::Status(s.r1());
	sieve_init_cs= s.r4();
}
