#include "bits/stdc++.h"
#define For(i,a,b) for(int i=a;i<=b;i++)
#define Rev(i,a,b) for(int i=a;i>=b;i--)
#define Fin(file) freopen(file,"r",stdin)
#define Fout(file) freopen(file,"w",stdout)
#define assume(expr) ((!!(expr))||(exit((fprintf(stderr,"Assumption Failed: %s on Line %d\n",#expr,__LINE__),-1)),false))
using namespace std; using ull = unsigned long long;
#define WLXORZ
#if ( defined WLXORZ || defined ONLINE_JUDGE )
#define printf(...) void()
class _Cerr{
public:
    template<typename T> _Cerr& operator<< (T) { return *this; }
}Cerr;
#define cerr Cerr
#endif
const int N=5e5+5,M=1e7+5; typedef long long ll;
// class STree{
//     int n,tot; vector<ll> a[N],A;
//     int newnode() { tot++; a[tot].clear(); a[tot].resize(n+1); return tot; }
// public:
//     void init(int _n) { n=_n; tot=0; A.clear(); A.resize(n+1); }
//     void modify(int& p,int x,int y){
//         if(!p) p=newnode();
//         A[x]+=y;
//     }
//     void modi(int& p,int l,int r,int x){
//         if(!p) p=newnode();
//         For(i,l,r) A[i]+=(x==1?1e9:-1e9);
//     }
//     void mod2(int& p,int l,int r,int x){
//         if(!p) p=newnode();
//         For(i,l,r) a[p][i]+=x;
//     }
//     void merge(int& p,int q){
//         if(!q) return; else if(!p) return p=q,void();
//         For(i,1,n) a[p][i]+=a[q][i];
//     }
//     ll query(int p,int l,int r){
//         ll res=-1e18; For(i,l,r) res=max(res,a[p][i]+A[i]);
//         cerr<<"res="<<res<<'\n';
//         return res;
//     }
// }T;
class STree{
    int n,tot,lc[M],rc[M],Tag[N<<2],tag2[M],tagi[M]; ll Max[M],mx[M];
    #define k1 k<<1
    #define k2 k<<1|1
    int newnode(int k){
        tot++; lc[tot]=rc[tot]=tag2[tot]=tagi[tot]=0;
        mx[tot]=Max[k]; return tot;
    }
    void build(int k,int l,int r){
        Max[k]=Tag[k]=0; if(l==r) return;
        int mid=(l+r)>>1; build(k1,l,mid); build(k2,mid+1,r);
    }
    void Setg(int k,int x) { Tag[k]+=x; Max[k]+=x*ll(1e9); }
    void seti(int p,int x) { if(p) tagi[p]+=x,mx[p]+=x*ll(1e9); }
    void Pusheen(int k) { if(Tag[k]) Setg(k1,Tag[k]),Setg(k2,Tag[k]),Tag[k]=0; }
    void pushdown(int p) { if(tagi[p]) seti(lc[p],tagi[p]),seti(rc[p],tagi[p]),tagi[p]=0; }
    ll gmx(int p,int k) { return p?mx[p]:Max[k]; }
    void modify(int& p,int k,int l,int r,int x,int y){
        if(l==r) { Max[k]+=y; if(p) mx[p]+=y;; return; }
        int mid=(l+r)>>1; Pusheen(k); if(p) pushdown(p);
        x<=mid?modify(lc[p],k1,l,mid,x,y):modify(rc[p],k2,mid+1,r,x,y);
        Max[k]=max(Max[k1],Max[k2]); if(p) mx[p]=max(gmx(lc[p],k1),gmx(rc[p],k2))+tag2[p];
    }
    void modi(int& p,int k,int l,int r,int ql,int qr,int x){
        if(ql<=l&&r<=qr) return Setg(k,x),seti(p,x),void();
        int mid=(l+r)>>1; Pusheen(k); if(p) pushdown(p);
        if(ql<=mid) modi(lc[p],k1,l,mid,ql,qr,x);
        if(mid+1<=qr) modi(rc[p],k2,mid+1,r,ql,qr,x);
        Max[k]=max(Max[k1],Max[k2]); if(p) mx[p]=max(gmx(lc[p],k1),gmx(rc[p],k2))+tag2[p];
    }
    void mod2(int& p,int k,int l,int r,int ql,int qr,int x){
        if(!p) p=newnode(k);
        if(ql<=l&&r<=qr) return tag2[p]+=x,mx[p]+=x,void();
        int mid=(l+r)>>1; Pusheen(k); pushdown(p);
        if(ql<=mid) mod2(lc[p],k1,l,mid,ql,qr,x);
        if(mid+1<=qr) mod2(rc[p],k2,mid+1,r,ql,qr,x);
        Max[k]=max(Max[k1],Max[k2]); mx[p]=max(gmx(lc[p],k1),gmx(rc[p],k2))+tag2[p];
    }
    void merge(int& p,int q,int k,int l,int r){
        if(!p||!q) return p|=q,void();
        tag2[p]+=tag2[q];
        if(l==r) return mx[p]=Max[k]+tag2[p],void();
        int mid=(l+r)>>1; Pusheen(k); pushdown(p); pushdown(q);
        merge(lc[p],lc[q],k1,l,mid);
        merge(rc[p],rc[q],k2,mid+1,r);
        Max[k]=max(Max[k1],Max[k2]); mx[p]=max(gmx(lc[p],k1),gmx(rc[p],k2))+tag2[p];
    }
    ll query(int p,int k,int l,int r,int ql,int qr){
        if(ql<=l&&r<=qr) return gmx(p,k);
        int mid=(l+r)>>1; ll res=-1e18; Pusheen(k); pushdown(p);
        if(ql<=mid) res=max(res,query(lc[p],k1,l,mid,ql,qr));
        if(mid+1<=qr) res=max(res,query(rc[p],k2,mid+1,r,ql,qr));
        return res+tag2[p];
    }
public:
    void init(int _n) { n=_n; tot=0; mx[0]=-1e18; build(1,1,n); }
    void modify(int& p,int x,int y) { printf("modify(%d,%d,%d)\n",p,x,y); modify(p,1,1,n,x,y); cerr<<"p="<<p<<'\n'; }
    void modi(int& p,int l,int r,int x) { printf("modi(%d,%d,%d,%d)\n",p,l,r,x); modi(p,1,1,n,l,r,x); cerr<<"p="<<p<<'\n'; }
    void mod2(int& p,int l,int r,int x) { printf("mod2(%d,%d,%d,%d)\n",p,l,r,x); mod2(p,1,1,n,l,r,x); cerr<<"p="<<p<<'\n'; }
    void merge(int& p,int q) { printf("merge(%d,%d)\n",p,q); merge(p,q,1,1,n); cerr<<"p="<<p<<'\n'; }
    ll query(int p,int l,int r) { ll res=query(p,1,1,n,l,r); printf("query(%d,%d,%d)=%lld\n",p,l,r,res); cerr<<"res="<<res<<'\n'; return res==-1e18?-1e9:res; }
}T;
mt19937_64 rng(190345);
int n,m,K,c[N],dep[N],fa[N],A[N],B[N],dfn[N],ans,Mx,Se,ffa[N][20],rt[N],lst[N];
struct Node{int l,r,x;};
vector<int> to[N]; vector<Node> lis[N]; ull hsh[N]; set<ull> Hvis; map<ull,int> Lst,Fir;
void dfs0(int u){
    dfn[u]=1; for(int v:to[u]) if(!dfn[v]) fa[v]=u,dep[v]=dep[u]+1,dfs0(v);
}
void dfs1(int u){
    for(int v:to[u]) dfs1(v),c[u]+=c[v],hsh[u]^=hsh[v];
    if(hsh[u]==0){
        if(c[u]>Mx) Se=Mx,Mx=c[u]; else if(c[u]>Se) Se=c[u];
    }
    else{
        if(Hvis.count(hsh[u])) ans=max(ans,c[u]);
        if(!Lst.count(hsh[u])) Lst[hsh[u]]=dep[u];
        Fir[hsh[u]]=dep[u];
    }
}
void dfs2(int u){
    if(dep[u]) T.modify(rt[u],dep[u],c[u]-lst[dep[u]]),lst[dep[u]]=c[u];
    if(hsh[u]&&Lst[hsh[u]]==dep[u]){
        T.modi(rt[u],Fir[hsh[u]],Lst[hsh[u]],-1);
    }
    for(auto nd:lis[u]) T.mod2(rt[u],nd.l,nd.r,-2);
    for(int v:to[u]){
        dfs2(v); T.merge(rt[u],rt[v]);
    }
    if(hsh[u]&&Lst[hsh[u]]==dep[u]){
        T.modi(rt[u],Fir[hsh[u]],Lst[hsh[u]],1);
    }
    if(hsh[u]&&Fir[hsh[u]]!=dep[u]){
        int ww=T.query(rt[u],Fir[hsh[u]],dep[u]-1)+c[u];
        ans=max(ans,ww);
    }
}
void solve(){
    // Bump: June 01 Thu, 19 : 31 : 31
    // Bump: June 02 Fri, 14 : 34 : 54
    cin>>n>>m; For(i,1,n) to[i].clear(),lis[i].clear(),dfn[i]=hsh[i]=c[i]=rt[i]=lst[i]=0;
    For(i,1,m) { int x,y; cin>>x>>y; A[i]=x,B[i]=y; to[x].push_back(y); to[y].push_back(x); }
    For(i,1,n) reverse(to[i].begin(),to[i].end());
    dep[0]=-1; dfs0(1);
    For(i,1,n) ffa[i][0]=fa[i];
    For(j,1,19) For(i,1,n) ffa[i][j]=ffa[ffa[i][j-1]][j-1];
    Hvis.clear(); Lst.clear(); Fir.clear(); For(i,1,n) to[i].clear();
    For(i,1,m){
        int x=A[i],y=B[i];
        if(fa[y]==x) to[x].push_back(y);
        else if(fa[x]==y) to[y].push_back(x);
        else{
            if(dep[x]>dep[y]) swap(x,y);
            ull hh=rng(); hsh[x]^=hh; hsh[y]^=hh; Hvis.insert(hh);
        }
    }
    cin>>K;
    For(_,1,K){
        int x,y; cin>>x>>y; if(dep[x]<dep[y]) swap(x,y);
        int tx=x,ty=y;
        Rev(i,19,0) if(dep[ffa[tx][i]]>=dep[ty]) tx=ffa[tx][i];
        if(tx==ty){
            tx=x;
            Rev(i,19,0) if(dep[ffa[tx][i]]>dep[ty]) tx=ffa[tx][i];
            c[x]++; c[y]--; lis[x].push_back(Node{dep[y]+1,dep[x],-2});
        }
        else{
            Rev(i,19,0) if(ffa[tx][i]!=ffa[ty][i]) tx=ffa[tx][i],ty=ffa[ty][i];
            int z=fa[tx];
            c[x]++; c[z]--; lis[x].push_back(Node{dep[z]+1,dep[x],-2});
            c[y]++; c[z]--; lis[y].push_back(Node{dep[z]+1,dep[y],-2});
        }
    }
    Mx=Se=ans=0; dfs1(1);
    ans=max(ans,Mx+Se);
    T.init(n); dfs2(1);
    cout<<ans<<'\n';
    // Bump: June 01 Thu, 20 : 29 : 38
    // Bump: June 21 Wed, 09 : 32 : 25
}
signed main(){
    Fin("data.in"); Fout("data.out");
    int Ti; cin>>Ti; while(Ti--) solve();
    // cerr<<"Time = "<<clock()<<" ms"<<endl;
    return 0;
}

// START TYPING IF YOU DON'T KNOW WHAT TO DO
// STOP TYPING IF YOU DON'T KNOW WHAT YOU'RE DOING
// CONTINUE, NON-STOPPING, FOR CHARLIEVINNIE

// Started Coding On: June 01 Thu, 17 : 04 : 26