//
//Interpreter.cpp
//
//by 张碧雲 on 16/6/2 
//

#include"Interpreter.h" 
#include"Condition.h"
#include"Attribute.h"
#include<string>
#include<string.h>
#include<iostream>
using namespace std;

class SyntaxException{};//异常类

//将字符串语句解析成机器能识别的信息，并调用相应函数进行处理
//传入参数：字符串
//出错返回0,成功返回1
int Interpreter::interpreter(string s)
{
	int tmp=0;//记录下标 
	string word;//存放一个单词
	
	word=getWord(s,&tmp);//从字符串下标为tmp的位置开始获取一个单词,tmp下标更新 
	
	if(strcmp(word.c_str(),"create")==0)//如果是create语句 
	{
		word=getWord(s,&tmp);//获取下一个单词 
		
		if(strcmp(word.c_str(),"table")==0)//如果是create table 
		{
		    string primaryKey="";//主键 
			string tableName="";//表名
			word=getWord(s,&tmp);//获取下一个单词
			if(!word.empty())tableName=word;
			else
			{
				cout<<"Syntax Error for no table name"<<endl;//报错，没有表名 
				return 0;
			}
			
			word=getWord(s,&tmp);//获取下一个单词 
			if(word.empty()||strcmp(word.c_str(),"(")!=0)//语法报错 
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}
			else
			{
				word=getWord(s,&tmp);
				std::vector<Attribute> attributeVector;
				while(!word.empty()&&strcmp(word.c_str(),"primary")!=0&&strcmp(word.c_str(),")")!=0)
				{
				    string attributeName=word;//获取属性名
					int type=0;//类型默认为int型
					bool ifUnique=false;//默认为不是Unique 
					word=getWord(s,&tmp);
					if(strcmp(word.c_str(),"int")==0)type=0;
					else if(strcmp(word.c_str(),"float")==0)type=-1;
					else if(strcmp(word.c_str(),"char")==0)
					{
						word=getWord(s,&tmp);
						if(strcmp(word.c_str(),"("))
						{
							cout<<"Syntax Error:unknown data type"<<endl;
							return 0;
						}
						word=getWord(s,&tmp);
						istringstream convert(word);
						//用 istringstream 读 string 对象，即可重新将数值型数据找回来。读取 istringstream 对象自动地将数值型数据的字符表示方式转换为相应的算术值
						if(!(convert>>type))//传入type参数，失败报错 
						{
							cout<<"Syntax error:illegal number in char()"<<endl;
							return 0;
						}					
					}
					else//其他类型报错 
					{
						cout<<"Syntax Error:unknown or missing data type!"<<endl;
						return 0;
					} 
					word=getWord(s,&tmp);
					if(strcmp(word.c_str(),"unique")==0)//若为unique
					{
						ifUnique=true;
						word=getWord(s,&tmp);//获取句末单词 
					}
					Attribute attr(attributeName,type,ifUnique);//初始化构造一个属性对象
					attributeVector.push_back(attr);//将属性对象放入属性向量 
					if(strcmp(word.c_str(),",")!=0) 
					{
						if(strcmp(word.c_str(),")")!=0)
						{
							cout<<"Syntax Error for ,!"<<endl;
							return 0;
						}
						else break;			
					}
					word=getWord(s,&tmp);
				}
				int primaryKeyLocation=-1;
				if(strcmp(word.c_str(),"primary")==0)//处理primaryKey
				{
					word=getWord(s,&tmp);
					if(strcmp(word.c_str(),"key")!=0)
					{
						cout<<"Error in syntax!"<<endl;
						return 0;
					}
					else
					{
						word=getWord(s,&tmp);
						if(strcmp(word.c_str(),"(")==0)
						{
							word=getWord(s,&tmp);
							primaryKey=word;
							int i=0;
							for(int i=0;i<attributeVector.size();i++)
							{
								if(primaryKey==attributeVector[i].name)//若该属性为主键，则设置为unique
								{
									attributeVector[i].ifUnique=true;
									break;//记录主键位置
								}
							}
							if(i==attributeVector.size())//找不到主键报错
							{
								cout<<"Syntax Error: primaryKey does not exist in attributes"<<endl;
								return 0;
							}
							primaryKeyLocation=i;
							word=getWord(s,&tmp);
							if(strcmp(word.c_str(),")")!=0)
							{
								cout<<"Error in syntax!"<<endl;
								return 0;
							}
						}
						else
						{
							cout<<"Error in syntax!"<<endl;
							return 0;
						}
						word=getWord(s,&tmp);
						if(strcmp(word.c_str(),")")!=0)//primaryKey的回括号
						{
							cout<<"Error in syntax!"<<endl;
							return 0;
						}
					}

				}
				else if(word.empty())
				{
					cout<<"Syntax Error:')' absent!"<<endl;
					return 0;
				}
				else if(primaryKeyLocation==-1)
				{
					cout<<"Syntax Error:no primary key!"<<endl;
					return 0;
				}

				ap->tableCreate(tableName,&attributeVector,primaryKey,primaryKeyLocation);//获取相应信息后调用API函数创建表
				return 1;
			} 
		}
		else if(strcmp(word.c_str(),"index")==0)
		{
			string indexName="";
			string tableName="";
			string attributeName="";
			word=getWord(s,&tmp);
		    if(!word.empty())indexName=word;
			else
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}

			word=getWord(s,&tmp);
			try//含异常处理块
			{
				if(strcmp(word.c_str(),"on")!=0)
					throw SyntaxException();//抛出异常
				word = getWord(s, &tmp);
				if(word.empty())
					throw SyntaxException();
				tableName=word;
				word=getWord(s,&tmp);
				if(strcmp(word.c_str(),"(")!=0)
					throw SyntaxException();
				if(word.empty())
					throw SyntaxException();
				attributeName=word;
				word=getWord(s,&tmp);
				if(strcmp(word.c_str(),")")!=0)
					throw SyntaxException();
				ap->indexCreate(indexName,tableName,attributeName);//调用API函数创建索引
				return 1;
			} catch(SyntaxException&)
			{
				cout<<"Syntax Error!"<<endl;
				return 0;
			}		
		}
		else//create其他东西报错
		{
			cout<<"Syntax Error for "<<word<<endl;
			return 0;
		}
		return 0;
	}//create语句部分

	else if(strcmp(word.c_str(),"select")==0)//如果是select语句
	{
		vector<string>attrSelected;//需select的属性
		string tableName="";
		word=getWord(s,&tmp);
		if(strcmp(word.c_str(),"*")!=0)//一般select语句
		{
			while(strcmp(word.c_str(),"from")!=0&&strcmp(word.c_str(),";")!=0)
			{
				attrSelected.push_back(word);
				word=getWord(s,&tmp);
			}
			if ((word.c_str(),";")==0)
			{
				cout<<"Syntax Error for no 'from'!"<<endl;
			    return 0;
			}
		}
		else//select*语句
		{
			word=getWord(s,&tmp);//获取*的下一个单词
		}
		if(strcmp(word.c_str(),"from")!=0)
		{
			cout<<"Error in syntax!"<<endl;
			return 0;
		}
		word=getWord(s,&tmp);
		if(!word.empty())tableName=word;
		else
		{
			cout<<"Error in syntax!"<<endl;
			return 0;
		}
		word=getWord(s,&tmp);
		if (word.empty())//后不跟where条件
		{
			if(attrSelected.size()==0){//select*语句显示表所有信息
				ap->recordShow(tableName);//调用API函数显示
			}
			else ap->recordShow(tableName,&attrSelected);//显示表中关于属性向量里属性的信息
			return 1;
		}
		else if(strcmp(word.c_str(),"where")==0)//后跟where条件
		{
			string attributeName="";
			string value="";//被比较值
			int operate= Condition::OPERATOR_EQUAL;
			std::vector<Condition> conditionVector;//Condition对象列表
			word=getWord(s,&tmp);
			while(1)//支持and连接多条件查询
			{
				try
				{
					if(word.empty())
						throw SyntaxException();
					attributeName=word;
					word=getWord(s,&tmp);
					if (strcmp(word.c_str(),"<=") == 0)
						operate = Condition::OPERATOR_LESS_EQUAL;
					else if (strcmp(word.c_str(),">=") == 0)
						operate = Condition::OPERATOR_MORE_EQUAL;
					else if (strcmp(word.c_str(),"<") == 0)
						operate = Condition::OPERATOR_LESS;
					else if (strcmp(word.c_str(),">") == 0)
						operate = Condition::OPERATOR_MORE;
					else if (strcmp(word.c_str(),"=") == 0)
						operate = Condition::OPERATOR_EQUAL;
					else if (strcmp(word.c_str(),"<>") == 0)
						operate = Condition::OPERATOR_NOT_EQUAL;
					else
						throw SyntaxException();
					word = getWord(s,&tmp);
					if(word.empty())
						throw SyntaxException();
					value = word;//被比较值
					Condition c(attributeName,value,operate);//初始化Condition对象
					conditionVector.push_back(c);//Condition对象存储在Condition向量里
					word = getWord(s,&tmp);
					if(word.empty())//后不再跟条件，单条件查询
						break;
                    if (strcmp(word.c_str(),"and") != 0)//只支持and连接两个条件查询
						throw SyntaxException();
					word = getWord(s,&tmp);//获取下一个条件
				}
				catch(SyntaxException&)
				{
					cout<<"Syntax Error!"<<endl;
					return 0;
				}
			}
			if(attrSelected.size()==0)//select*条件查询
				ap->recordShow(tableName,NULL,&conditionVector);
			else
				ap->recordShow(tableName, &attrSelected,&conditionVector);//特定属性select条件查询
			return 1;
		}
	}//select语句部分

	else if(strcmp(word.c_str(), "drop")==0)//如果是drop语句
	{
		word = getWord(s,&tmp);
        if (strcmp(word.c_str(), "table") == 0)//drop table语句
		{
			word = getWord(s,&tmp);
			if (!word.empty())
			{
				ap->tableDrop(word);//调用API函数删除表
				return 1;
			}
			else
			{
				cout<<"Syntax Error!"<<endl;
				return 0;
			}
		}

		else if(strcmp(word.c_str(), "index") == 0)//删除索引
		{
			word = getWord(s,&tmp);
			if (!word.empty())
			{
				ap->indexDrop(word);//调用API函数删除索引
				return 1;
			}
			else
			{
				cout<<"Error in syntax!"<<endl;
				return 1;
			}
		}
		else//drop语句只支持drop表和索引
		{
			cout<<"Error in syntax!"<<endl;
			return 0;
		}       
	}//drop语句部分

	else if(strcmp(word.c_str(), "delete")==0)
	{
		string tableName = "";
		word = getWord(s,&tmp);
		if (strcmp(word.c_str(), "from") != 0)//delete from...句式
		{
			cout<<"Error in syntax!"<<endl;
			return 0;
		}
		word = getWord(s,&tmp);
		if (!word.empty())
			tableName = word;
		else
		{
			cout<<"Error in syntax!"<<endl;
			return 0;
		}

		word = getWord(s,&tmp);
		if (word.empty())//后不跟条件，删除表的所有记录
		{
			ap->recordDelete(tableName);//调用API函数
			return 1;
		}
		else if (strcmp(word.c_str(),"where") == 0)//后跟where条件
		{
			string attributeName = "";
			string value = "";
			int operate = Condition::OPERATOR_EQUAL;
			std::vector<Condition> conditionVector;
			word = getWord(s,&tmp);	
			while(1){
				try {
					if(word.empty())
						throw SyntaxException();
					attributeName = word ;
					word = getWord(s,&tmp);
					if (strcmp(word.c_str(),"<=") == 0)
						operate = Condition::OPERATOR_LESS_EQUAL;
					else if (strcmp(word.c_str(),">=") == 0)
						operate = Condition::OPERATOR_MORE_EQUAL;
					else if (strcmp(word.c_str(),"<") == 0)
						operate = Condition::OPERATOR_LESS;
					else if (strcmp(word.c_str(),">") == 0)
						operate = Condition::OPERATOR_MORE;
					else if (strcmp(word.c_str(),"=") == 0)
						operate = Condition::OPERATOR_EQUAL;
					else if (strcmp(word.c_str(),"<>") == 0)
						operate = Condition::OPERATOR_NOT_EQUAL;
					else
						throw SyntaxException();
					word = getWord(s,&tmp);
					if(word.empty())
						throw SyntaxException();
					value = word;//被比较值
					Condition c(attributeName,value,operate);
					conditionVector.push_back(c);//条件对象放置条件向量里
					if(word.empty()) //单条件比较
						break;
					if (strcmp(word.c_str(),"and") != 0)//只支持and连接
						throw SyntaxException();
					word = getWord(s,&tmp);
				  }
				//捕捉异常
				catch(SyntaxException&)
				{
					cout<<"Syntax Error!"<<endl;
					return 0;
				}
		      }
			ap->recordDelete(tableName, &conditionVector);//调用API函数删除符合条件的记录
			return 1;
		   }
	}//delete语句部分

	else if(strcmp(word.c_str(), "insert") == 0)//如果是insert语句
	{
		string tableName = "";
		std::vector<string> valueVector;//属性值向量
		word = getWord(s,&tmp);
		try 
		{
			if (strcmp(word.c_str(),"into") != 0)
				throw SyntaxException();
			word = getWord(s,&tmp);
			if (word.empty())
				throw SyntaxException();
			tableName = word;
			word = getWord(s,&tmp);
			if (strcmp(word.c_str(),"values") != 0)
				throw SyntaxException();
			word = getWord(s,&tmp);
			if (strcmp(word.c_str(),"(") != 0)
				throw SyntaxException();
			word = getWord(s,&tmp);
			while (!word.empty() && strcmp(word.c_str(),")") != 0)//获取属性值
			{
				valueVector.push_back(word);
				word = getWord(s,&tmp);
				if (strcmp(word.c_str(),",") == 0)//跳过“，”号
					word = getWord(s,&tmp);
			}
			if (strcmp(word.c_str(),")") != 0)//非正常结束
				throw SyntaxException();
		}
		catch(SyntaxException&)
		{
			cout<<"Syntax Error!"<<endl;
			return 0;
		}
		ap->recordInsert(tableName,&valueVector);//调用API函数插入一条记录
		return 1;
	}//insert语句部分

	else if (strcmp(word.c_str(), "quit") == 0)
	{ return 587;}

	else if (strcmp(word.c_str(), "commit") == 0)
	{ return 1;}
	else if (strcmp(word.c_str(), "execfile") == 0)
	{
		fileName = getWord(s,&tmp);
		cout<<"try to open "<<fileName<<"..."<<endl;
		return 2;
	}
	else
	{
		if(word != "")
			cout<<"Error, command "<<word<<" not found"<<endl;
		return 0;
	}
	return 0;

} 


//获取字符串从指定下标开始的一个单词
//传入参数字符串s,int型指针指向下标
//返回一个字符串单词word
string Interpreter::getWord(string s,int* tmp)//*tmp随着遍历s一直在改变
{
	string word;//存储结果
	int idx1,idx2;//word开始下标，终止的后一个下标

	while((s[*tmp] == ' ' || s[*tmp] == 10  || s[*tmp] == '\t') && s[*tmp] != 0)//前处理到所需的正常word开始位置，s[*tmp] == 10,换行键
	{
		(*tmp)++;//tmp增加，访问s后一下标对应值
	}
    idx1 = *tmp;

	if (s[*tmp] == '(' || s[*tmp] == ',' || s[*tmp] == ')')//特殊单词括号、逗号处理
	{
		(*tmp)++;
		idx2 = *tmp;
		word = s.substr(idx1,idx2-idx1);//获取单词字符串
		return word;
	}
	else if (s[*tmp] == 39)//ASCII码39——'(单引号)
	{
		(*tmp)++;
		while (s[*tmp] != 39 && s[*tmp] !=0)
			(*tmp)++;
		if (s[*tmp] == 39)
		{
			idx1++;//略过‘记录单词开始下标
			idx2 = *tmp;
			(*tmp)++;
			word = s.substr(idx1,idx2-idx1);
			return word;
		}
		else
		{
			word = "";
			return word;
		}
	}
	else
	{
		while (s[*tmp] != ' ' &&s[*tmp] != '(' && s[*tmp] != 10 && s[*tmp] != 0 && s[*tmp] != ')' && s[*tmp] != ',')//一般正常遍历获取单词末尾对应下标
			(*tmp)++;
		idx2 = *tmp;
		if (idx1 != idx2)
			word = s.substr(idx1,idx2-idx1);
		else
			word = "";

		return word;
	}
}
