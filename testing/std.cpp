//BadWaper gg
#include<bits/stdc++.h> 
#define inf 1e9
#define eps 1e-6
#define N 100010
using namespace std;
typedef long long ll;
typedef unsigned long long ull;
inline ll read()
{
	char ch=getchar();
	ll s=0,w=1;
	while(ch<'0'||ch>'9'){if(ch=='-')w=-1;ch=getchar();}
	while(ch>='0'&&ch<='9'){s=s*10+ch-'0';ch=getchar();}
	return s*w;
}
ll n,m;
struct edge
{
	ll next,to;
}e[N<<1];
ll head[N],cnt,vis[N];
ll pos1[N],pos2[N];
ll a[N],s[N];
char s1[N],s2[N];
ll siz[N],C[N],p[N],ans;
ll minn=inf,rt,block;
struct SAM
{
	ll len[N],ch[N][26],fa[N];
	ll tot,son[N][26],R[N],siz[N],num[N],last;
	ll p[N],c[N];
	ll s[N];
	SAM(){tot=last=1;}
	inline void insert(ll x)
	{
		ll nowp=++tot,p=last;len[nowp]=len[p]+1;siz[nowp]=1;R[nowp]=len[nowp];
		while(p&&!ch[p][x])ch[p][x]=nowp,p=fa[p];
		if(!p)fa[nowp]=1;
		else 
		{
			ll q=ch[p][x];
			if(len[q]==len[p]+1)fa[nowp]=q;
			else
			{
				ll nowq=++tot;len[nowq]=len[p]+1;
				fa[nowq]=fa[q];fa[q]=nowq,fa[nowp]=nowq;
				for(ll i=0;i<26;i++)ch[nowq][i]=ch[q][i];
				while(p&&ch[p][x]==q)ch[p][x]=nowq,p=fa[p];
			}
		}
		last=nowp;
	}//建SAM
	inline void build()
	{
		for(register ll i=1;i<=tot;i++)c[len[i]]++;
		for(register ll i=1;i<=m;i++)c[i]+=c[i-1];
		for(register ll i=1;i<=tot;i++)p[c[len[i]]--]=i;//基排
		for(register ll i=tot;i>=2;i--)
		{
			ll x=p[i];
			siz[fa[x]]+=siz[x];R[fa[x]]=R[x];//R是right集合内任意一个值，所以这里你爱咋搞咋搞啦qwq
			son[fa[x]][s[R[x]-len[fa[x]]]]=x;//求son
		}
	}
	inline void clear(){for(register ll i=1;i<=tot;i++)num[i]=0;}
	inline void calc(ll now,ll father,ll p,ll L)
	{
		if(len[p]==L)p=son[p][a[now]];//情况2
		else if(s[R[p]-L]!=a[now])p=0;//情况1
		if(!p)return ;//T不在S直接return
                num[p]++;
		for(register ll i=head[now];i;i=e[i].next)
		{
			if(e[i].to==father||vis[e[i].to])continue;
			calc(e[i].to,now,p,L+1);
		}
	}
	inline void pushdown()
	{
		for(register ll i=2;i<=tot;i++)//自上而下
		{
			ll x=p[i];
			num[x]+=num[fa[x]];
		}
	}
}S1,S2;
inline void add_edge(ll from,ll to){e[++cnt]=(edge){head[from],to};head[from]=cnt;}
void getroot(ll now,ll Ns,ll father)
{
	siz[now]=1;ll maxn=-inf;
	for(register ll i=head[now];i;i=e[i].next)
	{
		if(e[i].to==father||vis[e[i].to])continue;
		getroot(e[i].to,Ns,now);siz[now]+=siz[e[i].to];
		maxn=max(maxn,siz[e[i].to]);
	}
	maxn=max(maxn,Ns-siz[now]);
	if(maxn<minn)minn=maxn,rt=now;
}//点分治求根
void dfs1(ll now,ll father)
{
	p[++p[0]]=now;
	for(register ll i=head[now];i;i=e[i].next)
	{
		if(e[i].to==father||vis[e[i].to])continue;
		dfs1(e[i].to,now);
	}
}//n^2暴力
void dfs2(ll now,ll father,ll x)
{
	x=S1.ch[x][a[now]];if(!x)return ;
	ans+=S1.siz[x];
	for(register ll i=head[now];i;i=e[i].next)
	{
		if(e[i].to==father||vis[e[i].to])continue;
		dfs2(e[i].to,now,x);
	}
}//n^2暴力
void calc(ll x,ll father,ll f)
{
	S1.clear(),S2.clear();
	if(father)
	{
		S1.calc(x,0,S1.son[1][a[father]],1);
		S2.calc(x,0,S2.son[1][a[father]],1);
	}
	else {S1.calc(x,0,1,0);S2.calc(x,0,1,0);}
	S1.pushdown();S2.pushdown();
	for(register ll i=1;i<=m;i++)
	{
		ans+=f*S1.num[pos1[i]]*S2.num[pos2[m-i+1]];
	}//统计答案，注意因为串反过来了所以后面是m-i+1
}
void DFS(ll now,ll father)
{
	C[now]=1;
	for(register ll i=head[now];i;i=e[i].next)
	{
		if(e[i].to==father||vis[e[i].to])continue;
		DFS(e[i].to,now);C[now]+=C[e[i].to];
	}
}//求分治子树大小
void dfs(ll now,ll Ns)
{
	if(Ns<=block)
	{
		p[0]=0;dfs1(now,0);
		for(ll i=1;i<=p[0];i++)dfs2(p[i],0,1);
		return ;
	}//n^2暴力
	vis[now]=1;calc(now,0,1);DFS(now,0);
	for(register ll i=head[now];i;i=e[i].next)
	{
		if(vis[e[i].to])continue;
		calc(e[i].to,now,-1);//容斥，把同一子树的答案容斥掉
                minn=inf,rt=0;
		getroot(e[i].to,C[e[i].to],0);dfs(rt,C[e[i].to]);
	}
}
int main()
{
	//freopen(".in","r",stdin);
	//freopen(".out","w",stdout);
	n=read(),m=read();block=sqrt(n);
	for(register ll i=1;i<n;i++)
	{
		ll x=read(),y=read();
		add_edge(x,y);add_edge(y,x);
	}
	scanf("%s",s1+1);
	for(register ll i=1;i<=n;i++)a[i]=s1[i]-'a';
	scanf("%s",s2+1);
	for(register ll i=1;i<=m;i++)s[i]=s2[i]-'a';
	for(register ll i=1;i<=m;i++)S1.insert(s[i]),pos1[i]=S1.last,S1.s[i]=s[i];
	reverse(s+1,s+m+1);//把串翻转之后第二个统计与第一个统计类似
	for(register ll i=1;i<=m;i++)S2.insert(s[i]),pos2[i]=S2.last,S2.s[i]=s[i];
	S1.build();S2.build();
	getroot(1,n,0);dfs(rt,n);
	printf("%lld\n",ans);
	return 0;
}

