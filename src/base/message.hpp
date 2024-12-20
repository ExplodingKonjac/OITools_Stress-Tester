#pragma once

#include <format>
#include <cstdio>

enum class TextAttr: int
{
	PLAIN     = 0,
	FG_BLACK  = 1<<0,
	FG_RED    = 1<<1,
	FG_GREEN  = 1<<2,
	FG_YELLOW = 1<<3,
	FG_BLUE   = 1<<4,
	FG_PURPLE = 1<<5,
	FG_CYAN   = 1<<6,
	FG_WHITE  = 1<<7,
	BG_BLACK  = 1<<8,
	BG_RED    = 1<<9,
	BG_GREEN  = 1<<10,
	BG_YELLOW = 1<<11,
	BG_BLUE   = 1<<12,
	BG_PURPLE = 1<<13,
	BG_CYAN   = 1<<14,
	BG_WHITE  = 1<<15,
	INTENSITY = 1<<16,
};

#define DEF_OP(op) \
inline TextAttr operator op(TextAttr lhs,TextAttr rhs) \
{ return TextAttr(static_cast<int>(lhs) op static_cast<int>(rhs)); } \
inline TextAttr operator op##=(TextAttr &lhs,TextAttr rhs) \
{ return lhs=lhs op rhs; }
DEF_OP(&)
DEF_OP(|)
DEF_OP(^)
#undef DEF_OP
inline TextAttr operator ~(TextAttr lhs)
{ return TextAttr(~static_cast<int>(lhs)); }

template<typename ...Args>
inline void setTextAttr(FILE *stream,TextAttr attr)
{
	int S=static_cast<int>(attr);

#if defined(_WIN32)
	WORD mask=0;
	if((S&0x1ffff)==0)
		mask=FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
	if(S&0x00ff)
	{
		int k=__builtin_ctz(S&0x00ff);
		if(k&1) mask|=FOREGROUND_RED;
		if(k&2) mask|=FOREGROUND_GREEN;
		if(k&4) mask|=FOREGROUND_BLUE;
	}
	if(S&0xff00)
	{
		int k=__builtin_ctz(S&0xff00)-8;
		if(k&1) mask|=BACKGROUND_RED;
		if(k&2) mask|=BACKGROUND_GREEN;
		if(k&4) mask|=BACKGROUND_BLUE;
	}
	if(S&(1<<16))
		mask|=FOREGROUND_INTENSITY;

	HANDLE h=nullptr;
	if(stream==stderr)
		h=GetStdHandle(STD_OUTPUT_HANDLE);
	else if(stream==stdout)
		h=GetStdHandle(STD_ERROR_HANDLE);
	SetConsoleTextAttribute(h,mask);

#elif defined(__linux__)
	std::string str("\e[");

	if((S&0x1ffff)==0)
		(str+=std::to_string(0))+=';';
	if(S&(1<<16))
		(str+=std::to_string(1))+=';';
	if(S&0x00ff)
		(str+=std::to_string(__builtin_ctz(S&0x00ff)+30))+=';';
	if(S&0xff00)
		(str+=std::to_string(__builtin_ctz(S&0xff00)-8+40))+=';';
	str.back()='m';

	std::fputs(str.c_str(),stream);

#else
#error "setTextAttr is not supported."
#endif
}

template<typename ...Args>
inline void printMessage(FILE *stream,const std::format_string<Args...> &fmt,Args &&...args)
{
	std::fputs(std::format(fmt,std::forward<Args&&>(args)...).c_str(),stream);
	std::fflush(stream);
}

template<typename ...Args>
inline void printMessage(const std::format_string<Args...> &fmt,Args &&...args)
{
	printMessage(stderr,fmt,std::forward<Args&&>(args)...);
}

template<typename ...Args>
inline void printColor(FILE *stream,TextAttr attr,const std::format_string<Args...> &fmt,Args &&...args)
{
	setTextAttr(stream,attr);
	printMessage(stream,fmt,std::forward<Args&&>(args)...);
}

template<typename ...Args>
inline void printColor(TextAttr attr,const std::format_string<Args...> &fmt,Args &&...args)
{
	printColor(stderr,fmt,std::forward<Args&&>(args)...);
}

