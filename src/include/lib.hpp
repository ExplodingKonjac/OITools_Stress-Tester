#pragma once
#include <bits/stdc++.h>
#include <windows.h>
#include <psapi.h>
#include <conio.h>

inline constexpr auto COLOR_DEFAULT=FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
inline constexpr auto COLOR_RED=FOREGROUND_RED;
inline constexpr auto COLOR_BLUE=FOREGROUND_BLUE;
inline constexpr auto COLOR_GREEN=FOREGROUND_GREEN;
inline constexpr auto COLOR_PURPLE=FOREGROUND_RED|FOREGROUND_BLUE;
inline constexpr auto COLOR_YELLOW=FOREGROUND_RED|FOREGROUND_GREEN;
inline constexpr auto COLOR_SKYBLUE=FOREGROUND_BLUE|FOREGROUND_GREEN;

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

inline bool keyDown(int key)
{ return GetAsyncKeyState(key)&0x8000; }

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
	std::string res;
	char buf[BUF_SIZE];
	DWORD len,ret;
	while(true)
	{
		ret=ReadFile(file,buf,BUF_SIZE,&len,nullptr);
		if(!ret || !len) break;
		res.append(buf,len);
	}
	return res;
}

template<typename ...Args>
inline void printColor(DWORD color,const char *fmt,Args ...args)
{
	HANDLE out=hStderr();
	SetConsoleTextAttribute(out,color);
	std::fprintf(stderr,fmt,args...);
	SetConsoleTextAttribute(out,COLOR_DEFAULT);
}

template<typename ...Args>
[[noreturn]] inline void quitError(const char *fmt,Args ...args)
{
	printColor(COLOR_RED|FOREGROUND_INTENSITY,"[Error] ");
	std::fprintf(stderr,fmt,args...);
	std::fputc('\n',stderr);
	std::exit(1);
}

template<typename ...Args>
[[noreturn]] inline void quitFailed(const char *fmt,Args ...args)
{
	printColor(COLOR_RED|FOREGROUND_INTENSITY,"[Failed] ");
	std::fprintf(stderr,fmt,args...);
	std::fputc('\n',stderr);
	std::exit(1);
}

template<typename ...Args>
inline void printNote(const char *fmt,Args ...args)
{
	printColor(COLOR_SKYBLUE,"[Note] ");
	std::fprintf(stderr,fmt,args...);
	std::fputc('\n',stderr);
}

template<typename ...Args>
inline void printMessage(const char *fmt,Args ...args)
{
	printColor(COLOR_GREEN,"[Message] ");
	std::fprintf(stderr,fmt,args...);
	std::fputc('\n',stderr);
}