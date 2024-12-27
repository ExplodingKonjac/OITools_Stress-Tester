#pragma once

#include <format>
#include <iostream>
#include <cstdint>

struct TextAttr
{
	enum Mask: int
	{
		CLEAR=1<<0,
		FOREGROUND=1<<1,
		BACKGROUND=1<<2,
		BOLD=1<<3,
		ITALIC=1<<4,
		UNDERLINE=1<<5,
		STRIKE=1<<6
	};
	std::uint8_t foreground,background;
	bool bold:1,italic:1,underline:1,strike:1;
};

inline constexpr TextAttr::Mask operator &(TextAttr::Mask a,TextAttr::Mask b)
{ return TextAttr::Mask(static_cast<int>(a)&static_cast<int>(b)); }

inline constexpr TextAttr::Mask operator |(TextAttr::Mask a,TextAttr::Mask b)
{ return TextAttr::Mask(static_cast<int>(a)|static_cast<int>(b)); }

inline constexpr TextAttr::Mask operator ^(TextAttr::Mask a,TextAttr::Mask b)
{ return TextAttr::Mask(static_cast<int>(a)^static_cast<int>(b)); }

inline constexpr TextAttr::Mask operator ~(TextAttr::Mask a)
{ return TextAttr::Mask(~static_cast<int>(a)); }

inline constexpr TextAttr::Mask &operator &=(TextAttr::Mask &a,TextAttr::Mask b)
{ return a=a&b; }

inline constexpr TextAttr::Mask &operator |=(TextAttr::Mask &a,TextAttr::Mask b)
{ return a=a|b; }

inline constexpr TextAttr::Mask &operator ^=(TextAttr::Mask &a,TextAttr::Mask b)
{ return a=a^b; }

inline std::string ansi(TextAttr::Mask mask,const TextAttr &attr)
{
	if(mask&TextAttr::CLEAR) return "\e[0m";
	std::string res("\e[");
	if(mask&TextAttr::FOREGROUND)
		res+=std::format("38;5;{0};",attr.foreground);
	if(mask&TextAttr::BACKGROUND)
		res+=std::format("48;5;{0};",attr.background);
	if(mask&TextAttr::BOLD)
		res+=std::format("{0};",attr.bold?1:22);
	if(mask&TextAttr::ITALIC)
		res+=std::format("{0};",attr.italic?3:23);
	if(mask&TextAttr::UNDERLINE)
		res+=std::format("{0};",attr.underline?4:24);
	if(mask&TextAttr::STRIKE)
		res+=std::format("{0};",attr.strike?9:29);
	res.back()='m';
	return res;
}

class MessageStream
{
 private:
	std::ostream *stream;

 public:
	MessageStream(std::ostream *target):
		stream(target)
	{}

	void setTextAttr(TextAttr::Mask mask,const TextAttr &attr)
	{
		(*stream)<<ansi(mask,attr);
	}

	template<typename ...Args>
	void print(const std::format_string<Args...> &fmt,Args &&...args)
	{
		(*stream)<<std::format(fmt,std::forward<Args&&>(args)...);
	}

	template<typename ...Args>
	void print(TextAttr::Mask mask,const TextAttr &attr,const std::format_string<Args...> &fmt,Args &&...args)
	{
		setTextAttr(mask,attr);
		(*stream)<<std::format(fmt,std::forward<Args&&>(args)...);
		setTextAttr(TextAttr::CLEAR,{});
	}

	template<typename ...Args>
	void error(const std::format_string<Args...> &fmt,Args &&...args)
	{
		print(TextAttr::FOREGROUND|TextAttr::BOLD,TextAttr{.foreground=9,.bold=true},"error: ");
		(*stream)<<std::format(fmt,std::forward<Args&&>(args)...)<<std::endl;
	}

	template<typename ...Args>
	void fatal(const std::format_string<Args...> &fmt,Args &&...args)
	{
		print(TextAttr::FOREGROUND|TextAttr::BOLD,TextAttr{.foreground=9,.bold=true},"fatal error: ");
		(*stream)<<std::format(fmt,std::forward<Args&&>(args)...)<<std::endl;
	}

	template<typename ...Args>
	void note(const std::format_string<Args...> &fmt,Args &&...args)
	{
		print(TextAttr::FOREGROUND|TextAttr::BOLD,TextAttr{.foreground=14,.bold=true},"note: ");
		(*stream)<<std::format(fmt,std::forward<Args&&>(args)...)<<std::endl;
	}

	template<typename ...Args>
	void warning(const std::format_string<Args...> &fmt,Args &&...args)
	{
		print(TextAttr::FOREGROUND|TextAttr::BOLD,TextAttr{.foreground=13,.bold=true},"warning: ");
		(*stream)<<std::format(fmt,std::forward<Args&&>(args)...)<<std::endl;
	}
};

inline MessageStream msg(&std::cerr);
