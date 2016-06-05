//
//Condition.cpp 
//
//by张碧雲 on 16/5/23
//

#include"Condition.h" 
#include<stdio.h> 

Condition::Condition(string a,string v,int o){//构造函数实现 
	attributeName = a;
	value = v;
	operate = o;
} 

bool Condition::ifRight(int content)
{
	stringstream ss; //stringstream——方便输入输出的类 
	ss << value; //被比较值输出给ss
	int myContent;
	ss >> myContent; //被比较值输入给int型 myContent
	
	switch(operate)//根据被比较值操作类型选择分支 
	{
		case Condition::OPERATOR_EQUAL:
		     return content == myContent; //相等返回1，不相等返回0 
			 break;
		case Condition::OPERATOR_NOT_EQUAL:
		     return content != myContent; //不相等返回1，相等返回0 
			 break; 
		case Condition::OPERATOR_LESS:
		     return content < myContent; //小于返回1，不小于返回0 
			 break;
		case Condition::OPERATOR_MORE:
		     return content > myContent; //大于返回1，不大于返回0 
			 break;
		case Condition::OPERATOR_LESS_EQUAL:
		     return content <= myContent; //小于等于返回1，不小于等于返回0 
			 break;
		case Condition::OPERATOR_MORE_EQUAL:
		     return content >= myContent; //小于返回1，不小于返回0 
			 break;
		default:
		     printf("没有针对该比较符操作!\n");
		     return false;
		     break;
	}
}
	
	
bool Condition::ifRight(float content)
{
	stringstream ss; //stringstream——方便输入输出的类 
	ss << value; //被比较值输出给ss
	float myContent;
	ss >> myContent; //被比较值输入给int型 myContent
	
	switch(operate)//根据被比较值操作类型选择分支 
	{
		case Condition::OPERATOR_EQUAL:
		     return content == myContent; //相等返回1，不相等返回0 
			 break;
		case Condition::OPERATOR_NOT_EQUAL:
		     return content != myContent; //不相等返回1，相等返回0 
			 break; 
		case Condition::OPERATOR_LESS:
		     return content < myContent; //小于返回1，不小于返回0 
			 break;
		case Condition::OPERATOR_MORE:
		     return content > myContent; //大于返回1，不大于返回0 
			 break;
		case Condition::OPERATOR_LESS_EQUAL:
		     return content <= myContent; //小于等于返回1，不小于等于返回0 
			 break;
		case Condition::OPERATOR_MORE_EQUAL:
		     return content >= myContent; //小于返回1，不小于返回0 
			 break;
		default:
		     printf("没有针对该比较符操作!\n");
		     return false;
		     break;
	} 
}
	
	
bool Condition::ifRight(string content)
{	
<<<<<<< HEAD
	string myContent = value; //不需要转化，可直接比较 
=======
	string myContent; //不需要转化，可直接比较 
>>>>>>> origin/master
		
	switch(operate)//根据被比较值操作类型选择分支 
	{
		case Condition::OPERATOR_EQUAL:
		     return content == myContent; //相等返回1，不相等返回0 
			 break;
		case Condition::OPERATOR_NOT_EQUAL:
		     return content != myContent; //不相等返回1，相等返回0 
			 break; 
		case Condition::OPERATOR_LESS:
		     return content < myContent; //小于返回1，不小于返回0 
			 break;
		case Condition::OPERATOR_MORE:
		     return content > myContent; //大于返回1，不大于返回0 
			 break;
		case Condition::OPERATOR_LESS_EQUAL:
		     return content <= myContent; //小于等于返回1，不小于等于返回0 
			 break;
		case Condition::OPERATOR_MORE_EQUAL:
		     return content >= myContent; //小于返回1，不小于返回0 
			 break;
		default:
		     printf("没有针对该比较符操作!\n");
		     return false;
		     break;
	}
}
