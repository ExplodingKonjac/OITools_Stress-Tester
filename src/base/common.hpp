#pragma once

#include <vector>

template<typename T>
std::vector<T> &operator +=(std::vector<T> &lhs,const std::vector<T> &rhs)
{
	lhs.insert(lhs.end(),rhs.begin(),rhs.end());
	return lhs;
}

template<typename T>
std::vector<T> operator +(const std::vector<T> &lhs,const std::vector<T> &rhs)
{
	std::vector<T> res=lhs;
	res.insert(res.end(),rhs.begin(),rhs.end());
	return res;
}
