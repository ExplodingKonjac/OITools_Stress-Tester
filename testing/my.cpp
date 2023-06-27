#include <bits/stdc++.h>
using namespace std;
// #define OPENIOBUF

namespace FastIO
{

class FastIOBase
{
 protected:
#ifdef OPENIOBUF
	static const int BUFSIZE = 1 << 16;
	char buf[BUFSIZE + 1];
	int buf_p = 0;
#endif
	FILE *target;
	FastIOBase(FILE *f): target(f) {}
	~FastIOBase() = default;

 public:
#ifdef OPENIOBUF
	virtual void flush() = 0;
#endif
};

class FastOutput final: public FastIOBase
{
#ifdef OPENIOBUF
 public:
	void flush() { fwrite(buf, 1, buf_p, target), buf_p = 0; }
#endif
 private:
	void __putc(char x)
	{
#ifdef OPENIOBUF
		if(buf[buf_p++] = x, buf_p == BUFSIZE) flush();
#else
		putc(x, target);
#endif
	}
	template <typename T>
	void __write(T x)
	{
		char stk[64], *top = stk;
		if(x < 0) return __putc('-'), __write(-x);
		do *(top++) = x % 10, x /= 10;
		while(x);
		for(; top != stk; __putc(*(--top) + '0'))
			;
	}

 public:
	FastOutput(FILE *f = stdout): FastIOBase(f) {}
#ifdef OPENIOBUF
	~FastOutput() { flush(); }
#endif
	FastOutput &operator<<(char x) { return __putc(x), *this; }
	FastOutput &operator<<(const char *s)
	{
		for(; *s; __putc(*(s++)))
			;
		return *this;
	}
	FastOutput &operator<<(const string &s) { return (*this) << s.c_str(); }
	template <typename T>
	enable_if_t<is_integral<T>::value, FastOutput &> operator<<(const T &x)
	{
		return __write(x), *this;
	}
	template <typename... T>
	void writesp(const T &...x)
	{
		initializer_list<int>{(this->operator<<(x), __putc(' '), 0)...};
	}
	template <typename... T>
	void writeln(const T &...x)
	{
		initializer_list<int>{(this->operator<<(x), __putc('\n'), 0)...};
	}
	template <typename Iter>
	void writesp(Iter begin, Iter end)
	{
		while(begin != end) (*this) << *(begin++) << ' ';
	}
	template <typename Iter>
	void writeln(Iter begin, Iter end)
	{
		while(begin != end) (*this) << *(begin++) << '\n';
	}
} qout;

class FastInput final: public FastIOBase
{
#ifdef OPENIOBUF
 public:
	void flush() { buf[fread(buf, 1, BUFSIZE, target)] = EOF, buf_p = 0; }
#endif
 private:
	bool __eof;
	char __getc()
	{
		if(__eof) return EOF;
#ifdef OPENIOBUF
		if(buf_p == BUFSIZE) flush();
		char ch = buf[buf_p++];
#else
		char ch = getc(target);
#endif
		return ~ch ? ch : (__eof = true, EOF);
	}
	void __ungetc(char c)
	{
		__eof = false;
#ifdef OPENIOBUF
		buf_p--;
#else
		ungetc(c, target);
#endif
	}

 public:
	FastInput(FILE *f = stdin): FastIOBase(f), __eof(false)
#ifdef OPENIOBUF
	{
		buf_p = BUFSIZE;
	}
	bool eof() const { return buf[buf_p] == EOF; }
#else
	{
	}
	bool eof() const { return feof(target); }
#endif
	char peek() { return __getc(); }
	explicit operator bool() const { return !__eof; }
	FastInput &operator>>(char &x)
	{
		while(isspace(x = __getc()))
			;
		return *this;
	}
	template <typename T>
	enable_if_t<is_integral<T>::value, FastInput &> operator>>(T &x)
	{
		char ch, sym = 0;
		x = 0;
		while(isspace(ch = __getc()))
			;
		if(__eof) return *this;
		if(ch == '-') sym = 1, ch = __getc();
		for(x = 0; isdigit(ch);
			x = (x << 1) + (x << 3) + (ch ^ 48), ch = __getc())
			;
		return __ungetc(ch), sym ? x = -x : x, *this;
	}
	FastInput &operator>>(char *s)
	{
		while(isspace(*s = __getc()))
			;
		if(__eof) return *this;
		for(; !isspace(*s) && !__eof; *(++s) = __getc())
			;
		return __ungetc(*s), *s = '\0', *this;
	}
	FastInput &operator>>(string &s)
	{
		char str_buf[(1 << 8) + 1] = {0}, *p = str_buf;
		char *const buf_end = str_buf + (1 << 8);
		while(isspace(*p = __getc()))
			;
		if(__eof) return *this;
		for(s.clear(), p++;; p = str_buf)
		{
			for(; p != buf_end && !isspace(*p = __getc()) && !__eof; p++)
				;
			if(p != buf_end) break;
			s.append(str_buf);
		}
		__ungetc(*p), *p = '\0', s.append(str_buf);
		return *this;
	}
	template <typename... T>
	void read(T &...x)
	{
		initializer_list<int>{(this->operator>>(x), 0)...};
	}
	template <typename Iter>
	void read(Iter begin, Iter end)
	{
		while(begin != end) (*this) >> *(begin++);
	}
} qin;

}  // namespace FastIO
using FastIO::qin, FastIO::qout;

using LL = long long;
using LD = long double;
using UI = unsigned int;
using ULL = unsigned long long;
constexpr LL INF = 4e18;

#ifndef DADALZY
#define FILEIO(file) \
	freopen(file ".in", "r", stdin), freopen(file ".out", "w", stdout)
#else
#define FILEIO(file)
#endif

class IntervalSet
{
 private:
	set<pair<LL, LL>> s;
	LL tag;

 public:
	IntervalSet(): s(), tag() {}
	void insert(LL l, LL r)
	{
		l += tag, r -= tag;
		auto it = s.upper_bound({l, INF});
		if(it != s.begin() && (--it)->second >= r) return;
		while(true)
		{
			it = s.lower_bound({l, -INF});
			if(it == s.end() || it->second > r) break;
			s.erase(it);
		}
		s.emplace(l, r);
	}
	bool contain(LL x)
	{
		auto it = s.upper_bound({x + tag, INF});
		if(it == s.begin()) return false;
		it--;
		return (it->second + tag) >= x;
	}
	LL getpre(LL x)
	{
		if(contain(x)) return x;
		auto it = s.upper_bound({x + tag, INF});
		if(it == s.begin()) return -INF;
		return prev(it)->second + tag;
	}
	LL getnxt(LL x)
	{
		if(contain(x)) return x;
		auto it = s.lower_bound({x + tag, -INF});
		if(it == s.end()) return INF;
		return it->first - tag;
	}
	LL getdis(LL x) { return min(x - getpre(x), getnxt(x) - x); }
	void clear() { s.clear(), tag = 0; }
	void addTag(int x) { tag += x; }
	bool empty() { return s.empty(); }
};

int n;
IntervalSet s1, s2;

int main()
{
	qin >> n;
	s1.insert(0, 0);
	s2.insert(0, 0);
	LL t, x, lstt = 0, lstx = 0;
	for(int i = 1; i <= n; i++)
	{
		bool clr1 = true, clr2 = true;
		vector<tuple<int, int, int>> add1, add2;
		qin >> t >> x, t -= lstt;
		LL lb = x - t, rb = x + t;

		// case 1:
		if(x == lstx) clr2 = false, add2.emplace_back(1, t, 0);
		if(s1.contain(x)) add2.emplace_back(0, lstx - t, lstx + t);

		// case 2:
		if(abs(x - lstx) <= t) clr1 = false;
		if(s2.getdis(x) <= t) add1.emplace_back(0, lstx, lstx);

		// case 3:
		LL d = s2.getdis(x);
		if(!s1.empty()) d = min(d, abs(lstx - x));
		if(d <= t)
		{
			d = t - d;
			add1.emplace_back(0, x, x);
			add2.emplace_back(0, x - d, x + d);
		}

		// case 4:
		LL p = s2.getnxt(lb), l = INF, r = -INF;
		if(p <= rb)
		{
			d = (t - abs(p - x)) / 2;
			auto [lq, rq] = minmax(p, x);
			l = min(l, lq - d), r = max(r, rq + d);
		}
		p = s2.getpre(rb);
		if(p >= lb)
		{
			d = (t - abs(p - x)) / 2;
			auto [lq, rq] = minmax(p, x);
			l = min(l, lq - d), r = max(r, rq + d);
		}
		if(l <= r)
		{
			add1.emplace_back(0, l, r);
			add2.emplace_back(0, x, x);
		}

		// final:
		if(clr1) s1.clear();
		for(auto &[op, x, y] : add1)
			if(op == 0)
				s1.insert(x, y);
			else
				s1.addTag(x);
		if(clr2) s2.clear();
		for(auto &[op, x, y] : add2)
			if(op == 0)
				s2.insert(x, y);
			else
				s2.addTag(x);
		lstt += t, lstx = x;
	}
	qout << ((s1.empty() && s2.empty()) ? "NO" : "YES") << '\n';
	return 0;
}