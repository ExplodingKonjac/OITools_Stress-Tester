#include "bits/stdc++.h"
#define For(i,a,b) for(int i=a;i<=b;i++)
#define Rev(i,a,b) for(int i=a;i>=b;i--)
#define Fin(file) freopen(file,"r",stdin)
#define Fout(file) freopen(file,"w",stdout)
#define assume(expr) ((!!(expr))||(exit((fprintf(stderr,"Assumption Failed: %s on Line %d\n",#expr,__LINE__),-1)),false))
using namespace std;
const int N=0; typedef long long ll;
mt19937 rng(chrono::high_resolution_clock::now().time_since_epoch().count());
long long rnd(ll l,ll r) { return uniform_int_distribution<ll>(l,r)(rng); }
void rnd(long long& l,long long& r,long long n) { l=rnd(1,n); r=rnd(1,n); if(l>r) swap(l,r); }
void rnd(int& l,int& r,int n) { l=rnd(1,n); r=rnd(1,n); if(l>r) swap(l,r); }
int n=2e5,K=2e5; set<pair<int,int>> S;
signed main(){
    Fout("data.in");
    int T=1;
    cout<<T<<'\n';
    while(T--){
        int m=rnd(n,3e5); S.clear();
        cout<<n<<' '<<m<<'\n';
        For(i,2,n){
            int x=rnd(1,i-1); cout<<x<<' '<<i<<'\n';
            S.insert(minmax(i,x));
        }
        For(i,1,m-n+1){
            int x,y; do x=rnd(1,n),y=rnd(1,n); while(x==y||S.count(minmax(x,y)));
            cout<<x<<' '<<y<<'\n'; S.insert(minmax(x,y));
        }
        cout<<K<<'\n';
        For(i,1,K){
            int x,y; do x=rnd(1,n),y=rnd(1,n); while(x==y);
            cout<<x<<' '<<y<<'\n';
        }
    }
    cerr<<"Time = "<<clock()<<" ms"<<endl;
    return 0;
}

// START TYPING IF YOU DON'T KNOW WHAT TO DO
// STOP TYPING IF YOU DON'T KNOW WHAT YOU'RE DOING
// CONTINUE, NON-STOPPING, FOR CHARLIEVINNIE

// Started Coding On: June 20 Tue, 17 : 16 : 56