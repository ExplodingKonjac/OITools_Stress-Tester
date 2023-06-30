#include <bits/stdc++.h>
using namespace std;
// #define OPENIOBUF

namespace FastIO
{

class FastIOBase
{
 protected:
#ifdef OPENIOBUF
	static const int BUFSIZE=1<<16;
	char buf[BUFSIZE+1];
	int buf_p=0;
#endif
	FILE *target;
	FastIOBase(FILE *f): target(f){}
	~FastIOBase()=default;
 public:
#ifdef OPENIOBUF
	virtual void flush()=0;
#endif
};

class FastOutput final: public FastIOBase
{
#ifdef OPENIOBUF
 public:
	void flush()
	{ fwrite(buf,1,buf_p,target),buf_p=0; }
#endif
 private:
	void __putc(char x)
	{
#ifdef OPENIOBUF
		if(buf[buf_p++]=x,buf_p==BUFSIZE) flush();
#else
		putc(x,target);
#endif
	}
	template<typename T>
	void __write(T x)
	{
		char stk[64],*top=stk;
		if(x<0) return __putc('-'),__write(-x);
		do *(top++)=x%10,x/=10; while(x);
		for(;top!=stk;__putc(*(--top)+'0'));
	}
 public:
	FastOutput(FILE *f=stdout): FastIOBase(f){}
#ifdef OPENIOBUF
	~FastOutput(){ flush(); }
#endif
	FastOutput &operator <<(char x)
	{ return __putc(x),*this; }
	FastOutput &operator <<(const char *s)
	{ for(;*s;__putc(*(s++)));return *this; }
	FastOutput &operator <<(const string &s)
	{ return (*this)<<s.c_str(); }
	template<typename T>
	enable_if_t<is_integral<T>::value,FastOutput&> operator <<(const T &x)
	{ return __write(x),*this; }
	template<typename ...T>
	void writesp(const T &...x)
	{ initializer_list<int>{(this->operator<<(x),__putc(' '),0)...}; }
	template<typename ...T>
	void writeln(const T &...x)
	{ initializer_list<int>{(this->operator<<(x),__putc('\n'),0)...}; }
	template<typename Iter>
	void writesp(Iter begin,Iter end)
	{ while(begin!=end) (*this)<<*(begin++)<<' '; }
	template<typename Iter>
	void writeln(Iter begin,Iter end)
	{ while(begin!=end) (*this)<<*(begin++)<<'\n'; }
}qout;

class FastInput final: public FastIOBase
{
#ifdef OPENIOBUF
 public:
	void flush()
	{ buf[fread(buf,1,BUFSIZE,target)]=EOF,buf_p=0; }
#endif
 private:
	bool __eof;
	char __getc()
	{
		if(__eof) return EOF;
#ifdef OPENIOBUF
		if(buf_p==BUFSIZE) flush();
		char ch=buf[buf_p++];
#else
		char ch=getc(target);
#endif
		return ~ch?ch:(__eof=true,EOF);
	}
	void __ungetc(char c)
	{
		__eof=false;
#ifdef OPENIOBUF
		buf_p--;
#else
		ungetc(c,target);
#endif
	}
 public:
	FastInput(FILE *f=stdin): FastIOBase(f),__eof(false)
#ifdef OPENIOBUF
	{ buf_p=BUFSIZE; }
	bool eof()const { return buf[buf_p]==EOF; }
#else
	{}
	bool eof()const { return feof(target); }
#endif
	char peek() { return __getc(); }
	explicit operator bool()const { return !__eof; }
	FastInput &operator >>(char &x)
	{ while(isspace(x=__getc()));return *this; }
	template<typename T>
	enable_if_t<is_integral<T>::value,FastInput&> operator >>(T &x)
	{
		char ch,sym=0;x=0;
		while(isspace(ch=__getc()));
		if(__eof) return *this;
		if(ch=='-') sym=1,ch=__getc();
		for(x=0;isdigit(ch);x=(x<<1)+(x<<3)+(ch^48),ch=__getc());
		return __ungetc(ch),sym?x=-x:x,*this;
	}
	FastInput &operator >>(char *s)
	{
		while(isspace(*s=__getc()));
		if(__eof) return *this;
		for(;!isspace(*s) && !__eof;*(++s)=__getc());
		return __ungetc(*s),*s='\0',*this;
	}
	FastInput &operator >>(string &s)
	{
		char str_buf[(1<<8)+1]={0},*p=str_buf;
		char *const buf_end=str_buf+(1<<8);
		while(isspace(*p=__getc()));
		if(__eof) return *this;
		for(s.clear(),p++;;p=str_buf)
		{
			for(;p!=buf_end && !isspace(*p=__getc()) && !__eof;p++);
			if(p!=buf_end) break;
			s.append(str_buf);
		}
		__ungetc(*p),*p='\0',s.append(str_buf);
		return *this;
	}
	template<typename ...T>
	void read(T &...x)
	{ initializer_list<int>{(this->operator>>(x),0)...}; }
	template<typename Iter>
	void read(Iter begin,Iter end)
	{ while(begin!=end) (*this)>>*(begin++); }
}qin;

} // namespace FastIO
using FastIO::qin,FastIO::qout;

using LL=long long;
using LD=long double;
using UI=unsigned int;
using ULL=unsigned long long;
using I128=__int128_t;
using U128=__uint128_t;

namespace Random
{

const auto seed=chrono::steady_clock::now().time_since_epoch().count();
mt19937 mt_rnd(seed);

template<typename IntT>
inline enable_if_t<is_integral<IntT>::value,IntT> randi(const IntT &l,const IntT &r) //[l,r]
{ return uniform_int_distribution<IntT>(l,r)(mt_rnd); }

template<typename FloatT>
inline enable_if_t<is_floating_point<FloatT>::value,FloatT> randf(const FloatT &l,const FloatT &r) //[l,r]
{ return uniform_real_distribution<FloatT>(l,r)(mt_rnd); }

template<typename Iter,typename IntT>
inline enable_if_t<is_integral<IntT>::value> randFilli(Iter first,Iter last,IntT l,IntT r) //[first,last)
{ for(uniform_int_distribution<IntT> rng(l,r);first!=last;*(first++)=rng(mt_rnd)); }

template<typename Iter,typename FloatT>
inline enable_if_t<is_floating_point<FloatT>::value> randFillf(Iter first,Iter last,FloatT l,FloatT r) //[first,last)
{ for(uniform_real_distribution<FloatT> rng(l,r);first!=last;*(first++)=rng(mt_rnd)); }

template<typename FloatT,typename ResT=bool>
inline const ResT &probability(FloatT p,const ResT &a=true,const ResT &b=false)
{ return bernoulli_distribution(p)(mt_rnd)?a:b; }

template<typename Iter>
inline Iter choose(Iter first,Iter last) //[first,last)
{ return first+mt_rnd()%distance(first,last); }

template<typename Iter>
inline void shuffle(Iter first,Iter last) //[first,last)
{ for(;first!=last;++first)swap(*first,*(first+mt_rnd()%distance(first,last))); }

template<typename IntT>
inline enable_if_t<is_integral<IntT>::value,pair<IntT,IntT>> subinterval(IntT l,IntT r)
{
	IntT resl=randi(l-1,r),resr=randi(l,r);
	if(resl<l) return make_pair(resr,resr);
	return minmax(resl,resr);
}

template<typename FloatT>
inline enable_if_t<is_floating_point<FloatT>::value,pair<FloatT,FloatT>> subinterval(FloatT l,FloatT r)
{
	FloatT resl=randf(l,r),resr=randf(l,r);
	return minmax(l,r);
}

template<typename InIter,typename OutIter>
inline OutIter sample(InIter first,InIter last,int n,OutIter out) //[first,last)
{
	auto m=distance(first,last);
	for((m<n&&(n=m));n;++first,m--)
		if(mt_rnd()%m<n) *out++=*first,n--;
	return out;
}

template<typename IntT,typename OutIter>
inline enable_if_t<is_integral<IntT>::value,OutIter> rangeSample(IntT l,IntT r,int n,OutIter out) //[l,r]
{
	unordered_set<IntT> s;
	IntT m=r-l+1,lim=m*0.6,x;
	for(n=min((IntT)n,m);n && m>lim;n--,m--)
	{
		while(s.find(x=randi(l,r))!=s.end());
		*out++=x,s.insert(x);
	}
	if(!n) return out;
	for(IntT i=l;i<=r;i++)
	{
		if(s.find(i)!=s.end()) continue;
		if(mt_rnd()%(m--)<n) *out++=i,n--;
	}
	return out;
}

vector<pair<int,int>> randTree(int n)
{
	vector<pair<int,int>> res;
	if(n<2) return res;
	vector<int> prf(n-2),deg(n+1,1);
	int p=0,u;
	randFilli(prf.begin(),prf.end(),1,n);
	for(auto &i: prf) deg[i]++;
	while(deg[u=++p]!=1);
	for(auto &v: prf)
	{
		res.emplace_back(u,v);
		if((--deg[v])==1 && v<p) u=v;
		else while(deg[u=++p]!=1);
	}
	res.emplace_back(n,u);
	return res;
}

vector<pair<int,int>> randTree(int n,int span)
{
	vector<pair<int,int>> res;
	if(n<2) return res;
	vector<int> fa(n+1),p(n);
	iota(p.begin(),p.end(),1),shuffle(p.begin(),p.end());
	for(int i=2;i<=n;i++)
	{
		int FloatT=abs(span),x;
		if(span>0) for(x=0;FloatT--;x=max(x,randi(1,i-1)));
		else if(span<0) for(x=n+1;FloatT--;x=min(x,randi(1,i-1)));
		else x=randi(1,i-1);
		fa[i]=x;
	}
	for(int i=2;i<=n;i++) res.emplace_back(p[i-1],p[fa[i]-1]);
	shuffle(res.begin(),res.end());
	return res;
}

} // namespace Random
using namespace Random;

int T=1,n=5,m=5,V=5;

int main()
{
	// freopen("input.in","w",stdout);
	qout<<n<<' '<<m<<'\n';
	auto t=randTree(n);
	for(auto &[u,v]: t)
		qout<<u<<' '<<v<<'\n';
	for(int i=1;i<=n;i++)
		qout<<randi('a','c');
	qout<<'\n';
	for(int i=1;i<=m;i++)
		qout<<randi('a','c');
	return 0;
}