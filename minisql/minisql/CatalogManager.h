//
//CatalogManager.h
//
//by 张碧雲 on 16/5/24
//

#ifndef CATALOGMANAGER_H
#define CATALOGMANAGER_H

#include<string> 
#include<vector>
#include"Attribute.h"
#include"BufferManager.h"
#include"IndexInfo.h"
using namespace std;

//类定义
class CatalogManager{
	public:
		BufferManager bm;
		CatalogManager();//构造函数 
		virtual ~CatalogManager();//析构函数
		
		int addIndex(string indexName,string tableName,string attributeName,int type);
		//增加索引，往Indexs文件里增加一个索引信息对象，往table的表信息的相应属性设置索引属性;传入参数索引名，表名相对应属性，索引类型 ;成功返回1，失败返回0 
		
		int revokeIndexOnAttribute(string tableName,string AttributeName,string indexName);
		//设置表信息对应属性索引信息为空字符串;传入参数（表名，索引属性，索引名字）;找到索引属性返回1，否则返回0 
		
		int findTable(string tableName);
		//查找是否有该表名的表；输入参数表名;没有返回零，有返回TABLE_FILE 值为9
		
		int findIndex(string indexName);
		//查找是否有该索引名的索引存在;传入参数索引名;找不到返回UNKNOWN_FILE值为8，找到返回INDEX_FILE10
		
		int dropTable(string tableName);
		// 删除表的原数据文件，即表信息文件，文件名等于表名；传入参数表名；成功返回1，否则返回0 
		
		int dropIndex(string indexName);
		//删除索引（删除Indexs文件对应的索引信息，设置表信息对应属性索引信息为空字符串）;传入参数（索引名字）;成功返回1，失败返回0；
		
		int deleteValue(string tableName,int deleteNum);
		//删除记录时将表信息记录数目减去删除的记录数目;传入参数表名，删除记录数目;成功返回该表删除后的记录数目，否则返回0 
		
		int insertRecord(string tableName,int addNum);
		//增加记录时将表信息记录数目加上增加的记录数目;传入参数表名，增加的记录数目;成功返回该表增加后的记录数目，否则返回0 
		
		int getRecordNum(string tableName);
		//获取某表的记录数目;传入表名;成功返回记录数目，失败返回零 
		
		int indexNameListGet(string tableName,vector<string>*indexVector);
		//获取指定某表上所有的属性(打开Indexs文件，一个个查找所有索引对象，找到表名为指定表名的索引名);传入参数表名，属性向量（存放结果);成功返回1，失败返回0；
		
		int getAllIndex(vector<IndexInfo>*indexs);
		//获取Indexs文件里所有所有信息，放在一个索引向量里；传入参数索引对象向量，用于存放结果；返回1，表示结束  
		
		int setIndexOnAttribute(string tableName,string AttributeName,string indexName);
		//设置表信息的相应属性索引名;传入参数表名，属性名，索引名;成功找到属性设置索引名返回 1，失败返回0
		
		int addTable(string tableName,vector<Attribute>*attributeVector,string primaryKeyName,int primaryKeyLocation);
		//新建一个表信息文件，存放信息，表记录数目初始化为0，主键位置，属性数目，属性对象信息 ;传入参数表名，属性列表向量，主键名（初始化为空字符串），属性位置（初始化为0;成功返回1，否返回0
		
		int getIndexType(string indexName);
		//获取某一索引名的类型；传入参数索引名；返回索引类型 ，-1,0，正整数；-2表示出错 
		
		int attributeGet(string tableName,vector<Attribute>*attributeVector);
		//获得某一表名的所有属性;输入参数表名，属性向量(存放属性）;成功返回1，失败返回0
		
		int calcuteLenth(string tableName);
		//计算表的一条记录的大小（从表信息文件计算获得;传入参数表名;返回记录大小，失败返回零 
		
		int calcuteLenth2(int type);
		//计算类型大小;传入参数类型的整数值;输出类型大小
		
		void recordStringGet(string tableName,vector<string>*recordContent,char*recordResult);
		//已经查找获取记录内容，存放在recordContent向量里，将记录内容连成一个字符串并存在结果字符串里;传入参数表名，记录内容指针向量（由一个个属性值组成），结果字符串指针;无返回
}; 

#endif
