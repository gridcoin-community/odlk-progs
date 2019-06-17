#pragma once

typedef unsigned char byte;

class EStreamOutOufBounds
	: public std::exception
{ const char * what () const noexcept {return "Stream access Out Ouf Bounds";} };

class CStream
{
	protected:
	byte *base;
	byte *cur;
	byte *wend;
	virtual void outofbounds(size_t ilen, bool wr, byte*& ret);

	public:
	virtual void fail();

	CStream()
		:base(0), wend(0)
	{ setpos(0); }
	CStream(byte *ibase,	size_t isize)
		:base(ibase), wend(ibase+isize)
	{ setpos(0); }

	/*byte* disown()
	{
		own= false;
		return base;
	}*/

	void setpos(size_t pos) {
		cur= base + pos;
	}

	void skip(unsigned displacement) {
		cur= cur + displacement;
	}

	size_t pos() const {
		return cur - base;
	}

	size_t length() const {
		return wend - base;
	}
	
	const byte* getbase() const {
		return base;
	}
	
	byte* getbase() {
		return base;
	}

	size_t left() {
		return wend - cur;
	}

	void read(void *dest, size_t len) {
		byte* x=getdata(len,0);
		std::copy(x,x+len,static_cast<byte*>(dest));
	}

	void write(const void *src, size_t len) {
		byte* x=getdata(len,1);
		std::copy(static_cast<const byte*>(src),static_cast<const byte*>(src)+len,x);
	}

	inline byte* getdata(size_t len, bool wr) {
		byte* ret= cur;
		cur+=len;
		if(cur > wend)
			outofbounds(len, wr, ret);
		return ret;
	}

	unsigned r1() {
		return getdata(1,0)[0];
	}

	void w1(unsigned v) {
		getdata(1,1)[0]= v;
	}

	unsigned r2() {
		byte*p=getdata(2,0);
		return (p[0])
			| (p[1]<<8);
	}
	
	void w2(unsigned v) {
		byte*p=getdata(2,1);
		p[0] = v, p[1]=v>>8;
	}
	
	unsigned r4() {
		byte*p=getdata(4,0);
		return (p[0])
		 | (p[1]<<8)
		 | (p[2]<<16)
		 | (p[3]<<24);
	}

	void w4(unsigned v) {
		byte*p=getdata(4,1);
		p[0]=v,
		p[1]=v>>8,
		p[2]=v>>16,
		p[3]=v>>24;
	}
	
	unsigned long long r6() {
		byte* p=getdata(6,0);
		return (unsigned long long)(p[0])
		 | ((unsigned long long)p[1]<<8)
		 | ((unsigned long long)p[2]<<16)
		 | ((unsigned long long)p[3]<<24)
		 | ((unsigned long long)p[4]<<32)
		 | ((unsigned long long)p[5]<<40);
	}	

	void w6(unsigned long long v) {
		byte* p=getdata(6,1);
		p[0]=v,
		p[1]=v>>8,
		p[2]=v>>16,
		p[3]=v>>24,
		p[4]=v>>32,
		p[5]=v>>40;
	}

	std::string ReadShortString() {
		size_t len = r1();
		return std::string ((char*)getdata(len,0), len);
	}

	std::string ReadStringAll() {
		size_t len = length() - pos();
		return std::string ((char*)getdata(len,0), len);
	}

	void WriteShortString(std::string s) {
		w1(s.length());
		std::copy(s.begin(),s.end(),(char*)getdata(s.length(),1));
	}
	
	void WriteStringAll(std::string s) {
		std::copy(s.begin(),s.end(),(char*)getdata(s.length(),1));
	}

};

void CStream::outofbounds(size_t ilen, bool wr, byte*& ret)
{
	fail();
}

void CStream::fail()
{
	throw EStreamOutOufBounds();
}

class CDynamicStream
	: public CStream
{
	protected:
	size_t headspace;
	byte* storage;
	void outofbounds(size_t ilen, bool wr, byte*& ret) override
	{
		if(!wr)
			fail();
		//find new size
		size_t curpos=cur-base;
		size_t newsize=curpos+headspace;
		//use different resize strategy than default implementation
		//when small size grow faster, to avoid thouse 2/4/8/16... byte allocations
		newsize= newsize * 1.5 + 128;
		newsize -= newsize % 64;
		//realloc
		byte* ptr2= (byte*) realloc(storage,newsize);
		storage= ptr2;
		//set new base/wend
		base= ptr2 + headspace;
		wend= base + newsize - headspace;
		//reset position
		//cur= base + curpos - ilen;
		cur= base + curpos;
		ret= cur - ilen;
	}
	public:
	CDynamicStream()
		: CStream(), headspace(0), storage(0)
	{}
	/* a hint of future write to at least this size */
	void reserve(size_t res)
	{
		if(res>pos()) {
			byte* ptr2= (byte*) realloc(storage,headspace+res);
			storage= ptr2;
			base= ptr2 + headspace;
			wend= base + res;
		}
	}
};

#ifdef TEST
	TEST("CBuffer::Basic") {
		CStream s((byte*)"abcdefgh", 8);
		ASSERT( s.pos() == 0 );
		ASSERT( s.length() == 8 );
		ASSERT( s.getdata(2,0)[0] == 'a' );
		ASSERT( s.pos() == 2 );
		ASSERT( s.getdata(1,0)[0] == 'c' );
		char r[3]= {0,};
		s.read(r,2);
		ASSERT( r[0] == 'd' && r[1] == 'e' );
		ASSERT( s.pos() == 5 );
	};
	TEST("CBuffer::DeserWord") {
		CStream s((byte*)"\1\1\2\x30\x12\x00\xFF""abcdef",13);
		ASSERT(s.r1() == 1);
		ASSERT(s.r2() == 513);
		ASSERT(s.r4() == 0xFF001230);
		ASSERT(s.r6() == 0x666564636261);
	}

	TEST("CBuffer::Seek")  {
		CStream s((byte*)"abcdefgh",8);
		ASSERT( s.pos() == 0 );
		ASSERT( s.length() == 8 );
		ASSERT( s.left() == 8 );
		s.skip(1);
		ASSERT( s.pos() == 1 );
		ASSERT( s.length() == 8 );
		ASSERT( s.left() == 7 );
		s.setpos(3);
		ASSERT( s.pos() == 3 );
		ASSERT( s.length() == 8 );
		ASSERT( s.left() == 5 );
		s.setpos(0);
		ASSERT( s.pos() == 0 );
		ASSERT( s.length() == 8 );
		ASSERT( s.left() == 8 );
	}

	TEST("CBuffer::Bounds")  {
		CStream s((byte*)"abcdefgh",8);
		try{
			s.getdata(9,0);
			ASSERT(false);
		} catch(EStreamOutOufBounds& e) {ASSERT(1);}
	}

	TEST("CBuffer::SerWord")  {
		byte data[13];
		CStream s(data, sizeof data);
		ASSERT(s.pos() == 0);
		s.w1(7);
		ASSERT( data[0]==7 );
		ASSERT(s.pos() == 1);
		s.w2(258);
		ASSERT(s.pos() == 3);
		ASSERT( data[1]==2 && data[2]==1 );
		s.w4(123456789);
		ASSERT(s.pos() == 7);
		ASSERT( memcmp((char*)data+3, "\x15\xCD\x5B\x07",4)==0 );
		s.w6(0x424344454647ULL);
		ASSERT(s.pos() == 13);
		ASSERT( memcmp((char*)data+7, "GFEDCB",6)==0 );
	}
#endif
