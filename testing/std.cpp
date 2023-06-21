#include<bits/stdc++.h>
#define ll long long
#define ull unsigned ll
using namespace std;
const int N=5e5+5,M=1e7+5;
const ll I=1e9;
struct edge{
	int to,nxt;
}e[N<<1];
int id,n,m,k,q,hd[N],ct,dt,d[N],top[N],zs[N],F[N],ad[N],pr[N],as;
bool vis[N];
vector<int> to[N],in[N],up[N];
inline void add(int u,int v){
	e[++ct]=(edge){v,hd[u]};hd[u]=ct;
	e[++ct]=(edge){u,hd[v]};hd[v]=ct;
}
void ch1(int u,int fa){
	ad[u]=1;top[u]=u;zs[u]=0;
	d[u]=d[fa]+1;F[u]=fa;
	for(int v:to[u]){
		if(v==fa) continue;
		ch1(v,u);ad[u]+=ad[v];
		zs[u]=ad[v]>ad[zs[u]]?v:zs[u];
	}
}
void ch2(int u,int fa){
	if(zs[u]) top[zs[u]]=top[u];
	for(int v:to[u]) if(v!=fa) ch2(v,u);
}
void dfs1(int u,int fa){
	for(int v:to[u]) if(v!=fa) dfs1(v,u),ad[u]+=ad[v];
}
inline int glca(int u,int v){
	while(top[u]!=top[v]){
		if(d[top[u]]<d[top[v]]) swap(u,v);
		u=F[top[u]];
	}
	return d[u]<d[v]?u:v;
}
pair<int,int> E[N];
mt19937_64 rnd(1);
unordered_map<ull,int> mp,jp;
struct num{
	int u;ll w;
	inline num()=default;
	inline num(int _u,ll _w):u(_u),w(_w){};
	inline bool operator <(const num tp)const{
		return w<tp.w;
	}
	inline num operator +(const ll v)const{
		return num(u,v+w);
	}
	inline num& operator +=(const ll v){
		w+=v;return *this;
	}
}mx[M],a[N];
int nt[N],B,L,s[M][2],rt[N],x,y,z,md[M],lz[M];
ull hs[N];
void ch(int u,int fa){
	d[u]=d[fa]+1;L=max(L,d[u]);
	for(int i=hd[u],v;i;i=e[i].nxt){
		v=e[i].to;
		if(!d[v]){
			ch(v,u);hs[u]^=hs[v];
			to[u].push_back(v);
			pr[v]=(i+1)>>1;
		}
		else if(d[v]<d[u]-1){
			ull w=rnd();mp[w]=(i+1)>>1;
			hs[u]^=w;hs[v]^=w;
		}
	}
}
class seg{
public:
	int md[N<<2];
	num mx[N<<2];
	inline void push(int o){
		if(!md[o]) return ;
		md[o<<1|0]+=md[o];mx[o<<1|0]+=I*md[o];
		md[o<<1|1]+=md[o];mx[o<<1|1]+=I*md[o];
		md[o]=0;
	}
	void set(int o,int l,int r){
		if(l==r){mx[o]=num(l,z);return ;}
		int mid=(l+r)>>1;push(o);
		x<=mid?set(o<<1,l,mid):set(o<<1|1,mid+1,r);
		mx[o]=max(mx[o<<1],mx[o<<1|1]);
	}
	void mdi(int o,int l,int r){
		if(x<=l&&y>=r){
			md[o]+=z;mx[o]+=z*I;
			return ;
		}
		int mid=(l+r)>>1;push(o);
		if(x<=mid) mdi(o<<1,l,mid);
		if(y>mid) mdi(o<<1|1,mid+1,r);
		mx[o]=max(mx[o<<1],mx[o<<1|1]); 
	}
}T;
inline void push(int o){
	if(!md[o]) return ;
	if(s[o][0]) md[s[o][0]]+=md[o],mx[s[o][0]]+=I*md[o];
	if(s[o][1]) md[s[o][1]]+=md[o],mx[s[o][1]]+=I*md[o];
	md[o]=0;
}
inline num get(int o,int o2){
	return o?mx[o]:T.mx[o2];
}
void ins(int &o,int o2,int l,int r){
	if(!o){
		o=++B;mx[o]=T.mx[o2];
		s[o][0]=s[o][1]=md[o]=lz[o]=0;
	}
	if(x<=l&&y>=r){mx[o]+=-2;lz[o]-=2;return ;}
	int mid=(l+r)>>1;
	push(o);T.push(o2);
	if(x<=mid) ins(s[o][0],o2<<1,l,mid);
	if(y>mid) ins(s[o][1],o2<<1|1,mid+1,r);
	mx[o]=max(get(s[o][0],o2<<1),get(s[o][1],o2<<1|1))+lz[o];
}
void mdi(int o,int o2,int l,int r){
	if(!o) return ; 
	if(x<=l&&y>=r){
		md[o]+=z;mx[o]+=z*I;
		return ;
	}
	int mid=(l+r)>>1;
	push(o);T.push(o2);
	if(x<=mid&&s[o][0]) mdi(s[o][0],o2<<1,l,mid);
	if(y>mid&&s[o][1]) mdi(s[o][1],o2<<1|1,mid+1,r);
	mx[o]=max(get(s[o][0],o2<<1),get(s[o][1],o2<<1|1))+lz[o];
}
void merge(int &o,int o2,int o3,int l,int r){
	if(!o||!o2){o=o|o2;return ;}
	lz[o]+=lz[o2];
	if(l==r){mx[o]=T.mx[o3]+lz[o];return ;}
	int mid=(l+r)>>1;
	push(o);push(o2);T.push(o3);
	merge(s[o][0],s[o2][0],o3<<1,l,mid);
	merge(s[o][1],s[o2][1],o3<<1|1,mid+1,r);
	mx[o]=max(get(s[o][0],o3<<1),get(s[o][1],o3<<1|1))+lz[o];
}
num que(int o,int o2,int l,int r){
	if(x>y) return num(0,-1e18);
	if(x<=l&&y>=r){return get(o,o2);}
	int mid=(l+r)>>1;
	push(o);T.push(o2);
	num w=num(0,-1e18);
	if(x<=mid) w=max(w,que(s[o][0],o2<<1,l,mid)); 
	if(y>mid) w=max(w,que(s[o][1],o2<<1|1,mid+1,r));
	return w+lz[o];
}
inline void upd(ll w,int u,int v){
	if(w<=as) return ;
	as=w;E[0]=make_pair(u,v);
}
void dfs(int u,int fa){
	zs[d[u]]=pr[u];
	if(hs[u]){
		nt[u]=jp[hs[u]];
		jp[hs[u]]=u;
		if(nt[u]){
			x=d[nt[u]]+1;y=d[u]-1;z=-1;
			if(x<=y) T.mdi(1,1,L);
			top[u]=top[nt[u]];
		}
		else top[u]=d[u];
		x=d[u];z=ad[u];
		T.set(1,1,L);
	}
	else a[++k]=num(pr[u],ad[u]);
	for(int v:to[u]) dfs(v,u),merge(rt[u],rt[v],1,1,L);
	for(int v:up[u]) x=d[v]+1,y=d[u],ins(rt[u],1,1,L);
	if(!hs[u]) return ;
	if(mp.find(hs[u])!=mp.end()) upd(ad[u],mp[hs[u]],pr[u]);
	x=top[u];y=d[u]-1;
	num w=que(rt[u],1,1,L)+ad[u];
	upd(w.w,zs[w.u],pr[u]);
	if(nt[u]){
		x=d[nt[u]]+1;y=d[u]-1;z=1;
		if(x<=y) T.mdi(1,1,L),mdi(rt[u],1,1,L);
	}
	jp[hs[u]]=nt[u];
}
inline void prt(int o){
	printf("%d %d\n",E[o].first,E[o].second);
}
inline void rd(int &op){
	char ch=getchar();op=0;
	while(ch<'0'||ch>'9') ch=getchar();
	while(ch>='0'&&ch<='9') op=(op<<1)+(op<<3)+(ch^48),ch=getchar();
}
int main(){
	freopen("data.in","r",stdin);
	freopen("data.ans","w",stdout);
	rd(id);
	while(id--){
		rd(n);rd(m);
		k=dt=ct=0;as=-1;a[0]=num(0,0);
		E[0]=make_pair(0,0);
		memset(hd,0,(n+1)<<2);
		for(int i=1,u,v;i<=m;i++){
			rd(u);rd(v);add(u,v);
			E[i]=make_pair(u,v); 
		}
		memset(hs,0,(n+1)<<3);
		memset(d,0,(n+1)<<2);
		mp.clear();jp.clear();
		for(int u=1;u<=n;u++) to[u].clear(),up[u].clear(); 
		ch(1,0);ch1(1,0);ch2(1,0);
		memset(ad,0,(n+1)<<2);
		rd(q);
		for(int i=1,u,v,w;i<=q;i++){
			rd(u);rd(v);w=glca(u,v);
			ad[u]++;ad[v]++;ad[w]-=2;
			if(w!=u) up[u].push_back(w);
			if(w!=v) up[v].push_back(w);
		}
		fill(T.mx+1,T.mx+(L<<2)+1,num(0,0));
		memset(T.md,0,(L+1)<<5);
		memset(rt,0,(n+1)<<2);
		dfs1(1,0);
		dfs(1,0);B=0;
		// printf("%d\n",as);
		nth_element(a,a+k-1,a+k+1);
		upd(a[k].w+a[k-1].w,a[k].u,a[k-1].u);
		if(!E[0].first) E[0].first=1+(E[0].second==1);
		if(!E[0].second) E[0].second=1+(E[0].first==1);
		printf("%d\n",as);
		// prt(E[0].first);prt(E[0].second);
	}
}