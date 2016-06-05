//
//Attribute.cpp
//
//by 张碧雲 on 16/5/23
// 

#include "Attribute.h" 

Attribute::Attribute(string n,int t, bool i)//属性类构造函数实现 
{
	name = n;
	type = t;
	ifUnique = i;
	index = ""; 
}
