//
//RecordManager.h 
//
//by 张碧雲 on 16/5/23
//

#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H
#include "Condition.h"
#include "Attribute.h" 
#include "BufferManager.h" 
#include "Minisql.h" 
#include <string>
#include <vector>
using namespace std;
class API;
class RecordManager
{
	public:
		RecordManager(){} 
		BufferManager bm;
		API *api;
		
		int tableCreate(string tableName); //建立表文件,成功返回1，否则返回0 
		int tableDrop(string tableName); //删除表文件，成功返回1，否则返回0 
		
		int indexCreate(string indexName); //创建索引文件，成功返回1，否则返回0 
		int indexDrop(string indexName); //删除索引文件，成功返回1，否则返回0 
		
		int recordInsert(string tableName,char*record,int recordSize);//向表里插入一条固定长度的记录 
		
	    int recordAllShow(string tableName,vector<string>*attributeNameVector,vector<Condition>*conditionVector);//显示输出所有块节点满足条件的记录 ，返回记录条数count 
		int recordBlockShow(string tableName,vector<string>*attributeNameVector,vector<Condition>*conditionVector,int blockOffset);//显示输出指定偏移量的块节点满足条件的记录，返回记录条数count  
		
		int recordAllFind(string tableName, vector<Condition>* conditionVector); //查找所有块节点满足条件记录，返回记录数目 
				
		int recordAllDelete(string tableName, vector<Condition>* conditionVector); //删除所有块节点满足条件的记录，返回删除的记录数目，-1 代表出错 
        int recordBlockDelete(string tableName,  vector<Condition>* conditionVector, int blockOffset);  //删除指定偏移量块节点满足条件的记录，返回删除的记录数目，-1 代表出错 
		
		int indexRecordAllAlreadyInsert(string tableName,string indexName);  //针对表的所有记录插入指定索引名的索引，返回满足要求的记录数目，-1代表出错 
		
		string tableFileNameGet(string tableName);//返回存放指定表名的文件名 
        string indexFileNameGet(string indexName);//返回存放指定索引名的文件名 
        
	private:
		int recordBlockFind(string tableName, vector<Condition>* conditionVector, blockNode* block);//查找某块节点满足条件记录，返回记录数目 ，便于查找所有
	    int recordBlockShow(string tableName, vector<string>*attributeNameVector, vector<Condition>* conditionVector, blockNode* block);//显示返回某块节点满足条件记录，返回记录数目 ，便于输出所有
        int recordBlockDelete(string tableName,  vector<Condition>* conditionVector, blockNode* block);//删除某块节点满足条件记录，返回记录数目 ，便于删除所有
        int indexRecordBlockAlreadyInsert(string tableName,string indexName, blockNode* block); //对某块节点记录插入指定索引，返回记录数目 ，便于对所有记录建立满足条件的索引 
		 
		bool recordConditionFit(char* recordBegin,int recordSize, vector<Attribute>* attributeVector,vector<Condition>* conditionVector); //判断从某处开始的指定长度记录是否满足条件，true-满足，false-不满足 
		void recordPrint(char* recordBegin, int recordSize, vector<Attribute>* attributeVector, vector<string> *attributeNameVector); //输出从某处开始，在所有属性attributeVector含一个表的所有属性中挑选attributeNameVector所需属性列的所有属性 
        bool contentConditionFit(char* content, int type, Condition* condition); //具体到判断每一个属性的内容是否满足条件，是recordConditionFit的子步骤 
        void contentPrint(char * content, int type);//将字符串形式的属性值转化为具体类型格式化输出 
};

#endif 
