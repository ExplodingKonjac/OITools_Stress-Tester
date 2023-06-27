#include <algorithm>
#include <cstdio>
#include <set>

using namespace std;
int n, x, t, lx, lt;
struct yijan
{
	int li;
	set<pair<int, int> > si;
	void insert(int l, int r)
	{
		l += li;
		r -= li;
		int fg = 0;
		set<pair<int, int> >::iterator it =
			si.lower_bound(make_pair(l + 1, -2e9));
		if(it != si.begin())
		{
			it--;
			if((*it).second >= r) fg = 1;
		}
		if(!fg)
		{
			while(1)
			{
				it = si.lower_bound(make_pair(l, -2e9));
				if(it == si.end() || (*it).second > r) break;
				si.erase(it);
			}
			si.insert(make_pair(l, r));
		}
	}
	int query(int x, int f1 = 1, int f2 = 1)
	{
		int as = 2e9;
		set<pair<int, int> >::iterator it =
			si.lower_bound(make_pair(x + li, -2e9));
		if(it != si.end())
		{
			int vl = max((*it).first - li - x, 0);
			if(vl && !f2) vl = 2e9;
			as = min(as, vl);
		}
		if(it != si.begin())
		{
			it--;
			int vl = max(x - (*it).second - li, 0);
			if(vl && !f1) vl = 2e9;
			as = min(as, vl);
		}
		return as;
	}
} s0, s1;
int main()
{
	scanf("%d", &n);
	s1.insert(0, 0);
	for(int i = 1; i <= n; i++)
	{
		scanf("%d%d", &t, &x);
		int ds = 2e9, lb = s0.query(x - (t - lt), 0, 1),
			rb = s0.query(x + (t - lt), 1, 0);
		if(s1.si.size())
		{
			ds = min(ds, lx > x ? lx - x : x - lx);
			if(lx >= x - (t - lt)) lb = min(lb, lx - x + t - lt);
			if(lx <= x + (t - lt)) rb = min(rb, x + t - lt - lx);
		}
		ds = min(ds, s0.query(x));
		int f0 = s1.query(x) == 0, f1 = s0.query(x) <= t - lt;
		if((lx > x ? lx - x : x - lx) > t - lt) s1.si.clear();
		if(lx != x) s0.si.clear();
		s0.li += t - lt;
		if(f0) s0.insert(lx - (t - lt), lx + (t - lt));
		if(f1) s1.insert(lx, lx);
		if(ds <= t - lt) s0.insert(x - (t - lt - ds), x + (t - lt - ds));
		if(lb <= (t - lt) * 2)
		{
			int lx = x - (t - lt) + lb, rx = x;
			if(lx > rx) swap(lx, rx);
			int ri = t - lt - rx + lx;
			s1.insert(lx - ri / 2, rx + ri / 2);
		}
		if(rb <= (t - lt) * 2)
		{
			int lx = x + (t - lt) - rb, rx = x;
			if(lx > rx) swap(lx, rx);
			int ri = t - lt - rx + lx;
			s1.insert(lx - ri / 2, rx + ri / 2);
		}
		lx = x;
		lt = t;
	}
	if(s0.si.size() || s1.si.size())
		printf("YES\n");
	else
		printf("NO\n");
}