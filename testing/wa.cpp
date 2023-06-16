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
constexpr LL INF=1e13;

#ifndef DADALZY
#define FILEIO(file) freopen(file".in","r",stdin),freopen(file".out","w",stdout)
#else
#define FILEIO(file) freopen("classic1.in","r",stdin)
#endif

int n,m,sid,a[100005];
LL ans[100005];

LL nowk;
struct Data
{
	LL y,x;
	Data(): y(),x(){}
	Data(LL _y,LL _x): y(_y),x(_x){}
	friend Data operator *(const Data &lhs,const Data &rhs)
	{ return Data(lhs.y+rhs.y,lhs.x+rhs.x); }
	friend Data operator +(const Data &lhs,const Data &rhs)
	{
		LL b1=lhs.y-lhs.x*nowk,b2=rhs.y-rhs.x*nowk;
		return (b1!=b2?b1>b2:lhs.x<rhs.x)?lhs:rhs;
	}
};

class Convex: public vector<LL>
{
 private:
	using BaseT=vector<LL>;
 public:
	using BaseT::BaseT;
	friend inline Convex operator +(const Convex &lhs,const Convex &rhs)
	{
		int n1=lhs.size(),n2=rhs.size();
		Convex res(lhs);
		if(n2>n1) res.resize(n2);
		for(int i=0;i<n2;i++) res[i]=max(res[i],rhs[i]);
		return res;
	}
	friend inline Convex operator *(const Convex &lhs,const Convex &rhs)
	{
		int n1=lhs.size(),n2=rhs.size();
		if(!n1 || !n2) return Convex{};
		Convex res(n1+n2-1);
		int i=1,j=1;
		while(i<n1 && j<n2)
		{
			int dt1=lhs[i]-lhs[i-1],
				dt2=rhs[j]-rhs[j-1];
			res[i+j-1]=res[i+j-2]+max(dt1,dt2);
			++(dt1>dt2?i:j);
		}
		while(i<n1) res[i+j-1]=res[i+j-2]+(lhs[i]-lhs[i-1]),i++;
		while(j<n2) res[i+j-1]=res[i+j-2]+(rhs[j]-rhs[j-1]),j++;
		return res;
	}
};

Convex t[400005][4];
int pos[400005][4];
#define LC (i<<1)
#define RC (i<<1|1)
void build(int l,int r,int i=1)
{
	if(l==r)
	{
		t[i][0]=Convex{0};
		t[i][3]=Convex{0,a[l]};
		return;
	}
	int mid=(l+r)>>1;
	build(l,mid,LC),build(mid+1,r,RC);
	for(int x=0;x<4;x++) for(int y=0;y<4;y++)
	{
		if((x&2) && (y&1)) continue;
		int z=(x&1)|(y&2);
		t[i][z]=t[i][z]+t[LC][x]*t[RC][y];
	}
}
Data _qres[4];
void __query(int lq,int rq,int k,int i=1,int l=1,int r=n)
{
	if(l>=lq && r<=rq)
	{
		Data now[4],nxt[4];
		for(int x=0;x<4;x++)
		{
			if(t[i][x].empty()) continue;
			int &p=pos[i][x],sz=t[i][x].size();
			while(p<sz-1 && t[i][x][p+1]-t[i][x][p]>k) p++;
			while(p>0 && t[i][x][p]-t[i][x][p-1]<=k) p--;
			now[x]=Data(t[i][x][p],p);
		}
		for(int x=0;x<4;x++) for(int y=0;y<4;y++)
		{
			if((x&2) && (y&1)) continue;
			int z=(x&1)|(y&2);
			nxt[z]=nxt[z]+_qres[x]*now[y];
		}
		copy(nxt,nxt+4,_qres);
		return;
	}
	int mid=(l+r)>>1;
	if(mid>=lq) __query(lq,rq,k,LC,l,mid);
	if(mid<rq) __query(lq,rq,k,RC,mid+1,r);
}
Data query(int lq,int rq,int k)
{
	fill(_qres,_qres+4,Data{});
	__query(lq,rq,k);
	return accumulate(_qres,_qres+4,Data{});
}

struct Oper{ int l,r,k,id;bool fl; }q[100005];

void divide(int L,int R,int l,int r)
{
	if(L>R || l>r) return;
	if(L==R)
	{
		nowk=L;
		for(int i=l;i<=r;i++)
		{
			Data val=query(q[i].l,q[i].r,L);
			val.y+=L*(q[i].k-val.x);
			ans[q[i].id]=val.y;
		}
	}
	else
	{
		int MID=(L+R)>>1;
		nowk=MID;
		for(int i=l;i<=r;i++)
		{
			Data val=query(q[i].l,q[i].r,MID);
			if(val.x>q[i].k) q[i].fl=false;
			else q[i].fl=true;
		}
		auto mid=partition(q+l,q+r+1,[](auto &x){ return x.fl; })-q;
		divide(L,MID,l,mid-1);
		divide(MID+1,R,mid,r);
	}
}

int main()
{
	// FILEIO("classic");
	qin>>n>>m>>sid,qin.read(a+1,a+n+1);
	build(1,n);
	for(int i=1;i<=m;i++)
		qin>>q[i].l>>q[i].r>>q[i].k,q[i].id=i;
	divide(-1e9,1e9,1,m);
	for(int i=1;i<=m;i++)
		qout<<ans[i]<<'\n';
	return 0;
}
/*
5 1 0
4 3 10 10 10 
3 5 1

*/