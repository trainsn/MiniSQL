//
//Attribute.h 关于属性类 
//
//by 张碧雲 on 16/5/23
//

#ifndef minisql_Attribute_h
#define minisql_Attribute_h

#include<string>
#include<iostream>
using namespace std;

class Attribute //属性类 
{
	public:
		string name; //属性名字
		int type; //属性类型，-1——float, 0——int,正整数n——char型，n表示多少个char
		bool ifUnique;
		string index;  //缺省为“”空字符串，表示没有索引
		Attribute(string n,int t, bool i);//构造函数，name=n,type=t,ifUnique=i
		
	    int static const TYPE_FLOAT = -1;
	    int static const TYPE_INT = 0;
	    string indexNameGet(){return index;} //判断是否有索引，返回空字符串或索引名
		
		void print()//输出属性信息，名字，类型，是否unique 
		{
			cout << "name:" << name << ";type:" << ";ifUnique: " << ifUnique << ";index:" << index << endl;	
		}
	 	    
};
 
#endif
