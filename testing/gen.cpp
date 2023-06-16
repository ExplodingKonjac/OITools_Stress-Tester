#include <bits/stdc++.h>
using namespace std;
//#define OPENIOBUF

namespace FastIO
{

class FastIOBase
{
 protected:
#ifdef OPENIOBUF
	static const int BUFSIZE=1<<22;
	char buf[BUFSIZE+1];
	int buf_p=0;
#endif
	FILE *target;
 public:
#ifdef OPENIOBUF
	virtual void flush()=0;
#endif
	FastIOBase(FILE *f): target(f){}
	~FastIOBase()=default;
};

class FastOutput: public FastIOBase
{
#ifdef OPENIOBUF
 public:
	inline void flush()
	{ fwrite(buf,1,buf_p,target),buf_p=0; }
#endif
 protected:
	inline void __putc(char x)
	{
#ifdef OPENIOBUF
		if(buf[buf_p++]=x,buf_p==BUFSIZE)flush();
#else
		putc(x,target);
#endif
	}
	template<typename T>
	inline void __write(T x)
	{
		static char stk[64],*top;top=stk;
		if(x<0) return __putc('-'),__write(-x);
		do *(top++)=x%10,x/=10; while(x);
		for(;top!=stk;__putc(*(--top)+'0'));
	}
 public:
	FastOutput(FILE *f=stdout): FastIOBase(f){}
#ifdef OPENIOBUF
	inline void setTarget(FILE *f) { this->flush(),target=f; }
	~FastOutput(){ flush(); }
#else
	inline void setTarget(FILE *f) { target=f; }
#endif
	template<typename ...T>
	inline void writesp(const T &...x)
	{ initializer_list<int>{(this->operator<<(x),__putc(' '),0)...}; }
	template<typename ...T>
	inline void writeln(const T &...x)
	{ initializer_list<int>{(this->operator<<(x),__putc('\n'),0)...}; }
	inline FastOutput &operator <<(char x)
	{ return __putc(x),*this; }
	inline FastOutput &operator <<(const char *s)
	{ for(;*s;__putc(*(s++)));return *this; }
	inline FastOutput &operator <<(const string &s)
	{ return (*this)<<s.c_str(); }
	template<typename T,typename=typename enable_if<is_integral<T>::value>::type>
	inline FastOutput &operator <<(const T &x)
	{ return __write(x),*this; }
}qout;

class FastInput: public FastIOBase
{
#ifdef OPENIOBUF
 public:
	inline void flush()
	{ buf[fread(buf,1,BUFSIZE,target)]='\0',buf_p=0; }
#endif
 protected:
	inline char __getc()
	{
#ifdef OPENIOBUF
		if(buf_p==BUFSIZE) flush();
		return buf[buf_p++];
#else
		return getc(target);
#endif
	}
 public:
#ifdef OPENIOBUF
	FastInput(FILE *f=stdin): FastIOBase(f){ buf_p=BUFSIZE; }
	inline void setTarget(FILE *f) { this->flush(),target=f; }
#else
	FastInput(FILE *f=stdin): FastIOBase(f){}
	inline void setTarget(FILE *f) { target=f; }
#endif
	inline char getchar() { return __getc(); }
	template<typename ...T>
	inline void read(T &...x)
	{ initializer_list<int>{(this->operator>>(x),0)...}; }
	inline FastInput &operator >>(char &x)
	{ while(isspace(x=__getc()));return *this; }
	template<typename T,typename=typename enable_if<is_integral<T>::value>::type>
	inline FastInput &operator >>(T &x)
	{
		static char ch,sym;x=sym=0;
		while(isspace(ch=__getc()));
		if(ch=='-') sym=1,ch=__getc();
		for(;isdigit(ch);x=(x<<1)+(x<<3)+(ch^48),ch=__getc());
		return sym?x=-x:x,*this;
	}
	inline FastInput &operator >>(char *s)
	{
		while(isspace(*s=__getc()));
		for(;!isspace(*s) && *s && ~*s;*(++s)=__getc());
		return *s='\0',*this;
	}
	inline FastInput &operator >>(string &s)
	{
		char str_buf[(1<<8)+1],*p=str_buf;
		char *const buf_end=str_buf+(1<<8);
		while(isspace(*p=__getc()));
		for(s.clear(),p++;;p=str_buf)
		{
			for(;p!=buf_end && !isspace(*p=__getc()) && *p && ~*p;p++);
			*p='\0',s.append(str_buf);
			if(p!=buf_end) break;
		}
		return *this;
	}
}qin;

} // namespace FastIO
using FastIO::qin,FastIO::qout;

using LL=long long;
using LD=long double;
using UI=unsigned int;
using ULL=unsigned long long;

namespace Random
{

const auto seed=chrono::steady_clock::now().time_since_epoch().count();
mt19937 mt_rnd(seed);

template<typename T,typename=enable_if_t<is_integral<T>::value>>
inline T randi(const T &l,const T &r) //[l,r]
{ return uniform_int_distribution<T>(l,r)(mt_rnd); }

template<typename T,typename=enable_if_t<is_floating_point<T>::value>>
inline T randf(const T &l,const T &r) //[l,r]
{ return uniform_real_distribution<T>(l,r)(mt_rnd); }

template<typename Iter,typename IntT,typename=enable_if_t<is_integral<IntT>::value>>
inline void randFilli(Iter first,Iter last,IntT l,IntT r) //[first,last)
{ for(;first!=last;*(first++)=randi(l,r)); }

template<typename Iter,typename FloatT,typename=enable_if_t<is_floating_point<FloatT>::value>>
inline void randFillf(Iter first,Iter last,FloatT l,FloatT r) //[first,last)
{ for(;first!=last;*(first++)=randf(l,r)); }

template<typename FloatT,typename ResT=bool>
inline const ResT &probability(FloatT p,const ResT &a=true,const ResT &b=false)
{ return bernoulli_distribution(p)(mt_rnd)?a:b; }

template<typename Iter>
inline Iter choose(Iter first,Iter last) //[first,last)
{ return first+mt_rnd()%distance(first,last); }

template<typename Iter>
inline void shuffle(Iter first,Iter last) //[first,last)
{ for(;first!=last;++first)swap(*first,*(first+mt_rnd()%distance(first,last))); }

template<typename IntT,typename=enable_if_t<is_integral<IntT>::value>>
inline pair<IntT,IntT> subinterval(IntT l,IntT r)
{
	IntT resl=randi(l-1,r),resr=randi(l,r);
	if(resl<l) return make_pair(resr,resr);
	return minmax(resl,resr);
}

template<typename InIter,typename OutIter>
inline OutIter sample(InIter first,InIter last,int n,OutIter out) //[first,last)
{
	using diff_t=typename iterator_traits<InIter>::difference_type;
	diff_t m=distance(first,last);
	for(n=min((diff_t)n,m);n;++first,m--)
		if(mt_rnd()%m<n) *out++=*first,n--;
	return out;
}

template<typename IntT,typename OutIter,typename=enable_if_t<is_integral<IntT>::value>>
inline OutIter rangeSample(const IntT &l,const IntT &r,int n,OutIter out) //[l,r]
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
		int T=abs(span),x;
		if(span>0) for(x=0;T--;x=max(x,randi(1,i-1)));
		else if(span<0) for(x=n+1;T--;x=min(x,randi(1,i-1)));
		else x=randi(1,i-1);
		fa[i]=x;
	}
	for(int i=2;i<=n;i++) res.emplace_back(p[i-1],p[fa[i]-1]);
	shuffle(res.begin(),res.end());
	return res;
}

} // namespace Random
using namespace Random;

int n=1e4,m=1e4,V=1e4;
int main()
{
	// freopen("input.in","w",stdout);
	qout<<n<<' '<<m<<' '<<0<<'\n';
	for(int i=1;i<=n;i++) qout<<randi(1,V)<<' ';
	qout<<'\n';
	for(int i=1;i<=m;i++)
	{
		auto [l,r]=subinterval(1,n);
		qout<<l<<' '<<r<<' ';
		qout<<randi(0,(r-l+2)/2)<<'\n';
	}
	return 0;
}