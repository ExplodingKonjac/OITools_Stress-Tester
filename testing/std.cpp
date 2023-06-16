#include<cstdio>
#include<algorithm>
#include<vector>
#include<cstring>
#include<ctime>
using namespace std;
typedef long long ll;
typedef pair<ll,int> pr;
#define mkp make_pair
const int MAXN=1e5+5,MAXS=1e7;
const ll INF=1ll<<60;
int n,q,a[MAXN];
pr operator +(pr a,pr b){
	return mkp(a.first+b.first,a.second+b.second);
}
struct node{
	vector<int> f[2][2];
}dat[MAXN<<2];
struct Ques{
	int x,y,k,id;
	int l,r,mid;
}b[MAXN];
int seg[MAXN][50];
bool operator <(Ques a,Ques b){
	return a.mid>b.mid;
}
struct Dif{
	int v,k,x;
}d[MAXS];
int m;
bool operator <(Dif a,Dif b){
	return a.v>b.v;
}
node operator +(node a,node b){
	node c;
	for(int i=0; i<2; i++)
		for(int j=0; j<2; j++)
			for(int x=0; x<2; x++)
				for(int y=0; y<2; y++){
					const vector<int> &v1=a.f[i][x],&v2=b.f[y][j];
					if(x&&y||v1.empty()||v2.empty()) continue;
					vector<int> tmp;
					int p1=0,p2=0;
					while(p1+p2<v1.size()+v2.size()-1){
						tmp.push_back(v1[p1]+v2[p2]);
						if(p2+1==v2.size()||p1+1<v1.size()&&v1[p1+1]+v2[p2]>v1[p1]+v2[p2+1]) p1++;
						else p2++;
					}
					for(int k=0; k<tmp.size(); k++)
						if(k<c.f[i][j].size()) c.f[i][j][k]=max(c.f[i][j][k],tmp[k]);
						else c.f[i][j].push_back(tmp[k]);
				}
	return c;
}
#define lc k<<1
#define rc k<<1|1
#define ls lc,l,mid
#define rs rc,mid+1,r
void Build(int k,int l,int r){
	if(l==r){
		dat[k].f[0][0].push_back(0);
		dat[k].f[1][1].push_back(0);
		dat[k].f[1][1].push_back(a[l]);
		d[++m]=Dif{a[l],k,3};
		return ;
	}
	int mid=l+r>>1;
	Build(ls);
	Build(rs);
	dat[k]=dat[lc]+dat[rc];
	for(int x=0; x<2; x++)
		for(int y=0; y<2; y++)
			for(int i=1; i<dat[k].f[x][y].size(); i++)
				d[++m]=Dif{dat[k].f[x][y][i]-dat[k].f[x][y][i-1],k,x<<1|y};
	return ;
}
void Query(int k,int l,int r,int x,int y,int id){
	if(x<=l&&r<=y){
		seg[id][++*seg[id]]=k;
		return ;
	}
	int mid=l+r>>1;
	if(x<=mid) Query(ls,x,y,id);
	if(mid<y) Query(rs,x,y,id);
	return ;
}
int vis[MAXN<<2][2][2];
int ans[MAXN];
int main(){
	// freopen("classic.in","r",stdin);
	// freopen("classic.out","w",stdout);
	int K;
	scanf("%d%d",&n,&q);
	scanf("%d",&K);
	for(int i=1; i<=n; i++)
		scanf("%d",a+i);
	Build(1,1,n);
	sort(d+1,d+m+1);
	for(int i=1; i<=q; i++){
		int l,r,k;
		scanf("%d%d%d",&l,&r,&k);
		b[i]=Ques{l,r,k,i,(int)(-5e8),10000,-10000};
		Query(1,1,n,l,r,i);
	}
	while(1){
		bool ok=0;
		for(int i=1; i<=q; i++)
			if(b[i].l<b[i].r) ok=1;
		sort(b+1,b+q+1);
		memset(vis,0,sizeof(int)*(n<<4|5));
		for(int i=1,j=1; i<=q; i++){
			if(ok&&b[i].l==b[i].r) continue;
			while(j<=m&&d[j].v>b[i].mid)
				vis[d[j].k][d[j].x>>1&1][d[j].x&1]++,j++;
			pr f[2],g[2],p[2][2];
			f[0]=f[1]=mkp(0ll,0);
			for(int l=1; l<=*seg[b[i].id]; l++){
				int k=seg[b[i].id][l];
				for(int x=0; x<2; x++)
					for(int y=0; y<2; y++)
						if(!dat[k].f[x][y].empty()) p[x][y]=mkp(dat[k].f[x][y][vis[k][x][y]]-1ll*b[i].mid*vis[k][x][y],-vis[k][x][y]);
						else p[x][y]=mkp(-INF,0);
				g[0]=f[0];
				g[1]=f[1];
				f[0]=max(g[1]+p[0][0],g[0]+max(p[0][0],p[1][0]));
				f[1]=max(g[1]+p[0][1],g[0]+max(p[0][1],p[1][1]));
			}
			pr res=max(f[0],f[1]);
			if(ok){
				if(-res.second>b[i].k) b[i].l=b[i].mid+1;
				else b[i].r=b[i].mid;
				b[i].mid=b[i].l+b[i].r>>1;
			}else ans[b[i].id]=res.first+1ll*b[i].k*b[i].mid;
		}
		if(!ok) break;
	}
	for(int i=1; i<=q; i++)
		printf("%d\n",ans[i]);
	return 0;
}
