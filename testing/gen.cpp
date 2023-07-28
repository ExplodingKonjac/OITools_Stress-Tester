#include <bits/stdc++.h>
using namespace std;

mt19937 mt_rnd(random_device{}());
int main()
{
	// freopen("input.in","w",stdout);
	auto x=mt_rnd();
	cout<<(x&0xffff)<<' '<<(x>>16)<<'\n';
	return 0;
}