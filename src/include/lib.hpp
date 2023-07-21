#pragma once
#include <bits/stdc++.h>
#include <winsock2.h>
#include <windows.h>
#include <psapi.h>
#include <conio.h>

enum class TextAttr: int
{
	plain     = 0,
	fg_black  = 1<<0,
	fg_red    = 1<<1,
	fg_green  = 1<<2,
	fg_yellow = 1<<3,
	fg_blue   = 1<<4,
	fg_purple = 1<<5,
	fg_cyan   = 1<<6,
	fg_white  = 1<<7,
	bg_black  = 1<<8,
	bg_red    = 1<<9,
	bg_green  = 1<<10,
	bg_yellow = 1<<11,
	bg_blue   = 1<<12,
	bg_purple = 1<<13,
	bg_cyan   = 1<<14,
	bg_white  = 1<<15,
	intensity = 1<<16,
	tg_stdout = 1<<17,
	tg_stderr = 1<<18,
};
#define DEF_OP(op) \
inline TextAttr operator op(TextAttr lhs,TextAttr rhs) \
{ return TextAttr(static_cast<int>(lhs) op static_cast<int>(rhs)); } \
inline TextAttr operator op##=(TextAttr &lhs,TextAttr rhs) \
{ return lhs=lhs op rhs; }
DEF_OP(&) DEF_OP(|) DEF_OP(^)
inline TextAttr operator ~(TextAttr lhs)
{ return TextAttr(~static_cast<int>(lhs)); }
#undef DEF_OP

inline HANDLE hStdin()
{ return GetStdHandle(STD_INPUT_HANDLE); }

inline HANDLE hStdout()
{ return GetStdHandle(STD_OUTPUT_HANDLE); }

inline HANDLE hStderr()
{ return GetStdHandle(STD_ERROR_HANDLE); }

inline constexpr std::size_t operator ""_KB(std::size_t x)
{ return x<<10; }
inline constexpr std::size_t operator ""_MB(std::size_t x)
{ return x<<20; }
inline constexpr std::size_t operator ""_GB(std::size_t x)
{ return x<<30; }

inline constexpr std::size_t strhash(const char *s)
{
	std::size_t res=0;
	while(*s) res=res*139+*s,s++;
	return res;
}

template<typename,typename=void>
struct HasCb: std::false_type {};
template<typename T>
struct HasCb<T,std::void_t<decltype(T::cb)>>: std::true_type {};

template<typename T>
inline std::enable_if_t<HasCb<T>::value> initmem(T &x)
{ ZeroMemory(&x,sizeof(x)),x.cb=sizeof(x); }

template<typename T>
inline std::enable_if_t<!HasCb<T>::value> initmem(T &x)
{ ZeroMemory(&x,sizeof(x)); }

inline HANDLE openFile(const std::string &s,char type)
{
	SECURITY_ATTRIBUTES sa{sizeof(sa),nullptr,true};
	auto t1=(type=='r'?GENERIC_READ:GENERIC_WRITE);
	auto t2=(type=='r'?OPEN_ALWAYS:CREATE_ALWAYS);
	return CreateFile(s.c_str(),t1,FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,t2,0,nullptr);
}

inline void resetFile(HANDLE file,bool clear=false)
{
	if(!file) return;
	SetFilePointer(file,0,nullptr,FILE_BEGIN);
	if(clear) SetEndOfFile(file);
}

inline std::string readFile(HANDLE file,DWORD size=INFINITE)
{
	constexpr DWORD BUF_SIZE=256;
	char buf[BUF_SIZE];
	std::string res;
	while(size)
	{
		DWORD len,ret;
		ret=ReadFile(file,buf,std::min(size,BUF_SIZE),&len,nullptr);
		if(!ret || !len) break;
		size-=len,res.append(buf,len);
	}
	return res;
}

inline std::string readPipe(HANDLE pipe,DWORD size=INFINITE)
{
	constexpr DWORD BUF_SIZE=256;
	char buf[BUF_SIZE];
	std::string res;
	while(size)
	{
		DWORD len,ret;
		PeekNamedPipe(pipe,nullptr,0,nullptr,&len,nullptr);
		if(!len) break;
		ret=ReadFile(pipe,buf,std::min(size,BUF_SIZE),&len,nullptr);
		if(!ret || !len) break;
		size-=len,res.append(buf,len);
	}
	return res;
}

inline bool writeFile(HANDLE file,const std::string &s)
{
	DWORD len;
	return WriteFile(file,s.c_str(),s.size(),&len,nullptr);
}

template<typename ...Args>
inline void setTextAttr(TextAttr attr,FILE *tg=stderr)
{
	int S=static_cast<int>(attr);
#if defined(_WIN32)
	WORD mask=0;
	if((S&0x1ffff)==0)
		mask=FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
	if(S&0x00ff)
	{
		int k=__builtin_ctz(S&0xff);
		if(k&1) mask|=FOREGROUND_RED;
		if(k&2) mask|=FOREGROUND_GREEN;
		if(k&4) mask|=FOREGROUND_BLUE;
	}
	if(S&0xff00)
	{
		int k=__builtin_ctz(S&0x00ff)-8;
		if(k&1) mask|=BACKGROUND_RED;
		if(k&2) mask|=BACKGROUND_GREEN;
		if(k&4) mask|=BACKGROUND_BLUE;
	}
	if(S&(1<<16))
		mask|=FOREGROUND_INTENSITY;
	HANDLE h;
	if(S&(1<<17))
		h=GetStdHandle(STD_OUTPUT_HANDLE);
	else
		h=GetStdHandle(STD_ERROR_HANDLE);
	SetConsoleTextAttribute(h,mask);
#elif defined(__unix__)
	std::vector<int> vec;
	if((S&0x1ffff)==0)
		vec.push_back(0);
	if(S&(1<<16))
		vec.push_back(1);
	if(S&0x00ff)
	{
		int k=__builtin_ctz(S&0x00ff);
		vec.push_back(k+30);
	}
	if(S&0xff00)
	{
		int k=__builtin_ctz(S&0xff00)-8;
		vec.push_back(k+40);
	}
	std::string str("\e[");
	for(auto i: vec)
		str.append(std::to_string(i));
	str.back()='m';
	std::fputs(str.c_str(),tg);
#else
#	warning "setTextAttr is not supported."
#endif
}

template<typename ...Args>
inline void printColor(TextAttr color,const char *fmt,Args ...args)
{
	setTextAttr(color);
	std::fprintf(stderr,fmt,args...);
	setTextAttr(TextAttr::plain);
}

template<typename ...Args>
[[noreturn]] inline void quitError(const char *fmt,Args ...args)
{
	printColor(TextAttr::fg_red|TextAttr::intensity,"[Error] ");
	std::fprintf(stderr,fmt,args...);
	std::fputc('\n',stderr);
	std::exit(1);
}

template<typename ...Args>
[[noreturn]] inline void quitFailed(const char *fmt,Args ...args)
{
	printColor(TextAttr::fg_red|TextAttr::intensity,"[Failed] ");
	std::fprintf(stderr,fmt,args...);
	std::fputc('\n',stderr);
	std::exit(1);
}

template<typename ...Args>
inline void printNote(const char *fmt,Args ...args)
{
	printColor(TextAttr::fg_cyan|TextAttr::intensity,"[Note] ");
	std::fprintf(stderr,fmt,args...);
	std::fputc('\n',stderr);
}

template<typename ...Args>
inline void printMessage(const char *fmt,Args ...args)
{
	printColor(TextAttr::fg_green|TextAttr::intensity,"[Message] ");
	std::fprintf(stderr,fmt,args...);
	std::fputc('\n',stderr);
}