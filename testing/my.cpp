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

#ifndef DADALZY
#define FILEIO(file) freopen(file".in","r",stdin),freopen(file".out","w",stdout)
#else
#define FILEIO(file)
#endif

int n,m,B;
char s1[50005],s2[50005];
vector<int> g[50005];

class FenwickTree
{
 private:
	vector<int> t;
	int N;
 public:
	FenwickTree(): t(),N(){}
	void build(int _n)
	{ N=_n,t=vector<int>(_n+1,0); }
	void modify(int p,int x)
	{ for(;p<=N;t[p]+=x,p+=p&-p); }
	int query(int p)
	{ int x=0;for(;p;x+=t[p],p-=p&-p);return x; }
	int query(int l,int r)
	{ return query(r)-query(l-1); }
};

class SuffixAutomaton
{
 private:
	struct SAMNode{ int fa,len,siz,pos,ch[26]; }t[100005];
	int cnt=1,lst=1,s[50005];
	
	vector<int> g[100005];
	int dfn,pre[100005][26],lb[100005],rb[100005],id[50005];
	void dfs(int u)
	{
		lb[u]=++dfn;
		if(t[u].pos) id[t[u].pos]=u;
		for(auto &v: g[u])
		{
			dfs(v);
			t[u].pos=t[v].pos;
			t[u].siz+=t[v].siz;
			pre[u][s[t[v].pos-t[u].len]]=v;
		}
		rb[u]=dfn;
	}
	FenwickTree tr;

 public:
	void extend(int c)
	{
		int p=lst,np=lst=++cnt;
		t[np].pos=t[np].len=t[p].len+1,t[p].siz=1;
		s[t[np].len]=c;
		for(;p && !t[p].ch[c];p=t[p].fa) t[p].ch[c]=np;
		if(!p) return t[np].fa=1,void();
		int q=t[p].ch[c];
		if(t[q].len==t[p].len+1) t[np].fa=q;
		else
		{
			int nq=++cnt;
			t[nq]=t[q];
			t[nq].len=t[p].len+1,t[nq].pos=t[nq].siz=0;
			t[np].fa=t[q].fa=nq;
			for(;p && t[p].ch[c]==q;p=t[p].fa) t[p].ch[c]=nq;
		}
	}
	void prework()
	{
		for(int i=1;i<=cnt;i++)
			g[t[i].fa].push_back(i);
		dfs(1);
		tr.build(dfn);
	}
	int pushBack(int u,int c)
	{
		return t[u].ch[c];
	}
	int pushFront(int u,int len,int c)
	{
		if(!len) return t[1].ch[c];
		if(len<t[u].len)
			return s[t[u].pos-len]==c?u:0;
		return pre[u][c];
	}
	int occ(int p) { return t[p].siz; }
	void add(int p) { if(p) tr.modify(lb[p],1),tr.modify(rb[p]+1,-1); }
	int get(int p) { return tr.query(lb[id[p]]); }
	void clear() { tr.build(dfn); }
}tl,tr;

int wgt[50005],ord[50005];
void getWeight(int u,int fa=0)
{
	bool lf=true;
	int x=0,y=0;
	for(auto &v: g[u])
	{
		if(v==fa) continue;
		getWeight(v,u);
		x|=(wgt[v]&y),y|=wgt[v];
		lf=false;
	}
	if(lf) return wgt[u]=1,void();
	int k=(x?__lg(x)+1:0);
	wgt[u]=((y>>k)+1)<<k;
}

LL ans;
bool vis[50005];
void findall(int u,vector<int> &res,int fa=0)
{
	res.push_back(u);
	for(auto &v: g[u])
		if(!vis[v] && v!=fa)
			findall(v,res,u);
}
void dfs1(int u,int cur,int fa=0)
{
	cur=tl.pushBack(cur,s1[u]-'a');
	ans+=tl.occ(cur);
	for(auto &v: g[u])
		if(!vis[v] && v!=fa)
			dfs1(v,cur,u);
}
void dfs2(int u,int cur1,int cur2,int len,int fa=0)
{
	cur1=tl.pushFront(cur1,len,s1[u]-'a');
	cur2=tr.pushFront(cur2,len,s1[u]-'a');
	tl.add(cur1),tr.add(cur2);
	len++;
	for(auto &v: g[u])
		if(!vis[v] && v!=fa)
			dfs2(v,cur1,cur2,len,u);
}
void calc(int u)
{
	vector<int> vec;
	findall(u,vec);
	if((int)vec.size()<B)
	{
		for(auto &u: vec) dfs1(u,1);
		for(auto &u: vec) vis[u]=true;
		return;
	}
	auto upd=[&](int opt,int xx=1)
	{
		for(int i=1;i<=m;i++)
		{
			if(s2[i]!=s1[u]) continue;
			int sl=tl.get(i-1)+xx,
				sr=tr.get(m-i)+xx;
			ans+=(LL)opt*sl*sr;
		}
	};
	tl.clear(),tr.clear();
	for(auto &v: g[u])
		if(!vis[v]) dfs2(v,1,1,0,u);
	upd(1);
	for(auto &v: g[u])
	{
		if(vis[v]) continue;
		tl.clear(),tr.clear();
		dfs2(v,1,1,0,u);
		upd(-1,0);
	}
	vis[u]=true;
}

int main()
{
	qin>>n>>m;
	for(int i=1,u,v;i<n;i++)
	{
		qin>>u>>v;
		g[u].push_back(v);
		g[v].push_back(u);
	}
	qin>>(s1+1)>>(s2+1);
	for(int i=1;i<=m;i++) tl.extend(s2[i]-'a');
	for(int i=m;i>=1;i--) tr.extend(s2[i]-'a');
	tl.prework(),tr.prework();
	getWeight(1);
	iota(ord+1,ord+n+1,1);
	sort(ord+1,ord+n+1,[](int x,int y){ return wgt[x]>wgt[y]; });
	B=sqrt(n);
	for(int i=1;i<=n;i++) if(!vis[ord[i]]) calc(ord[i]);
	qout<<ans<<'\n';
	return 0;
}