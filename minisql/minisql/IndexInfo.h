//
//IndexInfo.h
//
//by 张碧雲 on 16/5/26 
//

#ifndef minisql_IndexInfo_h
#define minisql_IndexInfo_h

#include<string>
using namespace std;

class IndexInfo//index信息类，索引名，表名，属性名，类型
{
	public:
		IndexInfo(string i,string t,string a,int ty)//构造函数 
		{
			indexName = i;tableName = t;Attribute = a;type = ty;
		}
		string indexName;
        string tableName;
        string Attribute;
        int type;
};

#endif 
