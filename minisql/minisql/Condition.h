//
//Condition.h
//minisql

//by 张碧雲 on 16/5/22
//
#ifndef minisql_Condition_h
#define minisql_Condition_h 
#include<string>
#include<sstream>
using namespace std; 

//Condition类 ,提供where语句的条件比较函数 
class Condition 
{
	public:
		const static int OPERATOR_EQUAL = 0; // 0代表"="
		const static int OPERATOR_NOT_EQUAL = 1; //1代表"<>"（不等于） 
		const static int OPERATOR_LESS = 2; //2代表"<"
		const static int OPERATOR_MORE = 3; //3代表">"
		const static int OPERATOR_LESS_EQUAL = 4; //4代表"<="
		const static int OPERATOR_MORE_EQUAL = 5; //5代表">="
		
		Condition(string a, string v,int o); //构造函数
		
		string attributeName; //关于哪个属性的条件
		string value; //被比较的价值，判断标准
		int operate;  //被比较的类型，-1——float,0——int,正整数——string
		
		//三种类型的比较 
		bool ifRight(int content); //将string型value被比较价值转化成 int型与传入的参数content比较 
		bool ifRight(float content); //同上理 
		bool ifRight(string content); //同上理 	
	
}; 

#endif
