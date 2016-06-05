//
//RecordManager.cpp
//
//by 张碧雲 on 16/5/23
//

#include<iostream>
#include"RecordManager.h"
#include<cstring>
#include"API.h"

//创建一个表，传入参数表名
//放table的空文件创建，1表示成功，0表示失败
int RecordManager::tableCreate(string tableName) 
{
	string tableFileName=tableFileNameGet(tableName);//设置创建后放table的table's file name叫做“TABLE_FILE_**" ，**：tableName
	
	FILE *fp;
	fp=fopen(tableFileName.c_str(),"w+");//创建一个放表的文件
	//tableFileName，string类型，tableFileName.c_str()指向tableFileName字符串内容
	
	if(fp==NULL)
	{
		return 0;
	}
	fclose(fp);
	return 1;
  	
}


//删除一个表，传入参数表名
////放table的文件删除，1表示成功，0表示失败
int RecordManager::tableDrop(string tableName) //借助BufferManager
{
	string tableFileName=tableFileNameGet(tableName);//获取文件名
	bm.delete_fileNode(tableFileName.c_str());//BufferManager bm，delete the file node and its block node 
	if (remove(tableFileName.c_str()))	//remove函数删除指定文件,int  remove(const char *_Filename);参数Filename为指定的要删除的文件名，成功则返回0，失败则返回-1
    {
        return 0;
    }
	return 1;
} 


//创建一个索引，传入参数索引名 
//放index的空文件创建，1表示成功，0表示失败
int RecordManager::indexCreate(string indexName)//类似创建表 
{
	string indexFileName = indexFileNameGet(indexName);//INDEX_FILE_"+indexName
	FILE *fp;
    fp = fopen(indexFileName.c_str(), "w+");
    if (fp == NULL)
    {
        return 0;
    }
    fclose(fp);
    return 1;
}


//删除一个索引，传入参数索引名 
//放index的文件删除，1表示成功，0表示失败 
int RecordManager::indexDrop(string indexName)//借助BufferManager 
{
	string indexFileName = indexFileNameGet(indexName);//获取文件名 
    bm.delete_fileNode(indexFileName.c_str()); //借助BufferManager删除文件节点 
    if (remove(indexFileName.c_str()))//销毁文件 
    {
        return 0;
    }
    return 1;
}


//往表里插入一条记录，传入参数表名，指向记录内容的字符指针，记录大小
//返回插入记录的块节点的位置——偏移量,-1表示出错 
int RecordManager::recordInsert(string tableName,char*record,int recordSize)//借助BufferManager 
{
	fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());//获得名叫tableName的文件节点
    blockNode *btmp = bm.getBlockHead(ftmp);//获取该文件节点的头结点 
    while (true)
    {
        if (btmp == NULL)
        {
            return -1;
        }
        if (bm.get_usingSize(*btmp) <= bm.getBlockSize() - recordSize)//足够插入一条记录 
        {
            
            char* addressBegin;
            addressBegin = bm.get_content(*btmp) + bm.get_usingSize(*btmp);//块节点开始指针+已使用的
            memcpy(addressBegin, record, recordSize);//拷贝记录内容 
            bm.set_usingSize(*btmp, bm.get_usingSize(*btmp) + recordSize);//更新该块节点已使用的空间
            bm.set_dirty(*btmp);//标记为脏节点，要重新写入文件
            return btmp->offsetNum;//返回该节点位置 
        }
        else
        {
            btmp = bm.getNextBlock(ftmp, btmp); //不足一插入记录，往下找下一个快节点 
        }
    }
    
    return -1;
} 

//显示输出所有满足条件的记录 ，按属性向量列表的输出相应所需属性
//传入参数表名，所需显示属性列表 ，条件列表向量 
//返回满足条件的记录数目,-1表示出错 
int RecordManager::recordAllShow(string tableName, vector<string>* attributeNameVector,  vector<Condition>* conditionVector)//借助BufferManager到下一个块节点 
{
	fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);
    int count = 0;//count为所有块的recordblocknum之和
    while (true)
    {
        if (btmp == NULL)
        {
            return -1;
        }
        if (btmp->ifbottom)//如果文件最后一块
        {
            int recordBlockNum = recordBlockShow(tableName,attributeNameVector, conditionVector, btmp);//btmp-块节点，单块节点显示 
            count += recordBlockNum;
            return count;
        }
        else//否则块节点要移到下一个
        {
            int recordBlockNum = recordBlockShow(tableName, attributeNameVector, conditionVector, btmp);
            count += recordBlockNum;
            btmp = bm.getNextBlock(ftmp, btmp);
        }
    }
    
    return -1;
} 

//指定偏移量的块节点满足条件的记录相应显示
//传入参数表名，所需显示属性列表 ，条件列表向量 ，快节点位置偏移量
//返回满足条件的记录数目,-1表示出错 
int RecordManager::recordBlockShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector, int blockOffset)//显示特定偏移的块节点的所有符合记录
{
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode* block = bm.getBlockByOffset(ftmp, blockOffset);//获得偏移为blockOffset的块节点
    if (block == NULL)
    {
        return -1;
    }
    else
    {
        return  recordBlockShow(tableName, attributeNameVector, conditionVector, block);//调用私有函数 
    }
}


//单块块节点满足条件的记录相应显示
//传入参数表名，所需显示属性列表 ，条件列表向量 ，块节点指针 
//返回满足条件的记录数目,-1表示出错  
int RecordManager::recordBlockShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector, blockNode* block)
{
	//if block is null, return -1
    if (block == NULL)
    {
        return -1;
    }
    
    int count = 0;
    
    char* recordBegin = bm.get_content(*block);//开始等于blockBegin
    vector<Attribute> attributeVector;
    int recordSize = api->recordSizeGet(tableName);//获得一条记录的大小

    api->attributeGet(tableName, &attributeVector);//属性向量里赋值表的所有属性名 
    char* blockBegin = bm.get_content(*block);//指针指向block有内容开始的地方 
    size_t usingSize = bm.get_usingSize(*block);
    
    while (recordBegin - blockBegin  < usingSize)//每次recordBegin会移到下一条记录开始 
    {
        //if the recordBegin point to a record
        
        if(recordConditionFit(recordBegin, recordSize, &attributeVector, conditionVector))//是否满足条件
        {
            count ++;
            recordPrint(recordBegin, recordSize, &attributeVector, attributeNameVector);//attributeVector含一个表的所有属性，attributeNameVector，被比较的属性列
            printf("\n");
        }
        
        recordBegin += recordSize;
    }
    
    return count;
	
} 


//查找所有满足条件表的记录，不输出显示
//传入参数表名，条件列表向量 
//返回满足条件的记录数目，-1代表出错
int RecordManager::recordAllFind(string tableName, vector<Condition>* conditionVector)//类似输出显示 
{
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);
    int count = 0;
    while (true)
    {
        if (btmp == NULL)
        {
            return -1;
        }
        if (btmp->ifbottom)
        {
            int recordBlockNum = recordBlockFind(tableName, conditionVector, btmp);
            count += recordBlockNum;
            return count;
        }
        else
        {
            int recordBlockNum = recordBlockFind(tableName, conditionVector, btmp);
            count += recordBlockNum;
            btmp = bm.getNextBlock(ftmp, btmp);
        }
    }
    
    return -1;
}


//查找单块块节点满足条件表的记录，不输出显示
//传入参数表名，条件列表向量，块节点指针 
//返回满足条件的记录数目，-1代表出错
int RecordManager::recordBlockFind(string tableName, vector<Condition>* conditionVector, blockNode* block)
{
    //if block is null, return -1
    if (block == NULL)
    {
        return -1;
    }
    int count = 0;
    
    char* recordBegin = bm.get_content(*block);
    vector<Attribute> attributeVector;
    int recordSize = api->recordSizeGet(tableName);
    
    api->attributeGet(tableName, &attributeVector);
    
    while (recordBegin - bm.get_content(*block)  < bm.get_usingSize(*block))
    {
        //if the recordBegin point to a record
        
        if(recordConditionFit(recordBegin, recordSize, &attributeVector, conditionVector))
        {
            count++;
        }
        
        recordBegin += recordSize;
        
    }
    
    return count;
}


//删除所有满足条件的记录
//输入参数表名，条件列表向量 
//输出满足条件的记录个数，-1代表出错
int RecordManager::recordAllDelete(string tableName, vector<Condition>* conditionVector)
{
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);

    int count = 0;
    while (true)
    {
        if (btmp == NULL)
        {
            return -1;
        }
        if (btmp->ifbottom)
        {
            int recordBlockNum = recordBlockDelete(tableName, conditionVector, btmp);//分解成删除单块块节点满足条件的记录 
            count += recordBlockNum;
            return count;
        }
        else
        {
            int recordBlockNum = recordBlockDelete(tableName, conditionVector, btmp);
            count += recordBlockNum;
            btmp = bm.getNextBlock(ftmp, btmp);
        }
    }
    
    return -1;
}


//删除指定偏移量块节点满足条件的记录
//输入参数表名，条件列表向量 ，块节点偏移量 
//输出满足条件的记录个数，-1代表出错
int RecordManager::recordBlockDelete(string tableName,  vector<Condition>* conditionVector, int blockOffset)
{
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode* block = bm.getBlockByOffset(ftmp, blockOffset);
    if (block == NULL)
    {
        return -1;
    }
    else
    {
        return  recordBlockDelete(tableName, conditionVector, block);
    }
}


//删除某块节点满足条件的记录
//输入参数表名，条件列表向量 ，块节点指针
//输出满足条件的记录个数，-1代表出错
int RecordManager::recordBlockDelete(string tableName,  vector<Condition>* conditionVector, blockNode* block)//所有删除记录函数的基础 
{
    //if block is null, return -1
    if (block == NULL)
    {
        return -1;
    }
    int count = 0;
    
    char* recordBegin = bm.get_content(*block);
    vector<Attribute> attributeVector;
    int recordSize = api->recordSizeGet(tableName);
    
    api->attributeGet(tableName, &attributeVector);
    
    while (recordBegin - bm.get_content(*block) < bm.get_usingSize(*block))
    {
        //if the recordBegin point to a record
        
        if(recordConditionFit(recordBegin, recordSize, &attributeVector, conditionVector))
        {
            count ++;
            
            api->recordIndexDelete(recordBegin, recordSize, &attributeVector, block->offsetNum);//删除相应索引值
            int i = 0;
            for (i = 0; i + recordSize + recordBegin - bm.get_content(*block) < bm.get_usingSize(*block); i++)
            {
                recordBegin[i] = recordBegin[i + recordSize];//删除后此时recordbegin后的所有记录向前挪一个
            }
            memset(recordBegin + i, 0, recordSize);//将最后一个置为零
			//void *memset(void *s,int c,size_t n)将已开辟内存空间 s 的首 n 个字节的值设为值 c
            bm.set_usingSize(*block, bm.get_usingSize(*block) - recordSize);
            bm.set_dirty(*block);//标记为脏块
        }
        else
        {
            recordBegin += recordSize;
        }
    }
    
    return count;
}


//针对表的所有记录建立某一索引
//传入参数表名，索引名 
//输出满足条件的记录个数，-1代表出错
int RecordManager::indexRecordAllAlreadyInsert(string tableName,string indexName)//借助BufferManager 
{
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);
    int count = 0;
    while (true)
    {
        if (btmp == NULL)
        {
            return -1;
        }
        if (btmp->ifbottom)
        {
            int recordBlockNum = indexRecordBlockAlreadyInsert(tableName, indexName, btmp);//单个块节点建立索引 
            count += recordBlockNum;
            return count;
        }
        else
        {
            int recordBlockNum = indexRecordBlockAlreadyInsert(tableName, indexName, btmp);
            count += recordBlockNum;
            btmp = bm.getNextBlock(ftmp, btmp);
        }
    }
    
    return -1;
}



//针对某一块节点的记录建立某一索引
//传入参数表名，索引名 ，快节点指针 
//输出满足条件的记录个数，-1代表出错
int RecordManager::indexRecordBlockAlreadyInsert(string tableName,string indexName,  blockNode* block)//在一个块节点里，表名为**的每条记录检查是否需要建立indexName的索引
{
    //if block is null, return -1
    if (block == NULL)
    {
        return -1;
    }
    int count = 0;
    
    char* recordBegin = bm.get_content(*block);
    vector<Attribute> attributeVector;
    int recordSize = api->recordSizeGet(tableName);
    
    api->attributeGet(tableName, &attributeVector);
    
    int type;
    int typeSize;
    char * contentBegin;
    
    while (recordBegin - bm.get_content(*block)  < bm.get_usingSize(*block))//一条记录一条记录地找
    {
        contentBegin = recordBegin;
        //if the recordBegin point to a record
        for (int i = 0; i < attributeVector.size(); i++)//对一条记录的每个属性操作
        {
            type = attributeVector[i].type;
            typeSize = api->typeSizeGet(type);
            
            //find the index  of the record, and insert it to index tree
            if (attributeVector[i].index == indexName)//default value is "", representing no index
            {
                api->indexInsert(indexName, contentBegin, type, block->offsetNum);
                count++;
            }
            
            contentBegin += typeSize;//下一条属性
        }
        recordBegin += recordSize;//下一条记录
    }
    
    return count;
}


//判断某条记录是否满足条件
//传入参数记录开始地址，记录大小 ，记录的所有属性属性列表向量，条件列表
//输出bool值，tru——满足，false——不满足
bool RecordManager::recordConditionFit(char* recordBegin,int recordSize, vector<Attribute>* attributeVector,vector<Condition>* conditionVector)
{
    if (conditionVector == NULL) {
        return true;
    }
    int type;
    string attributeName;
    int typeSize;
    char content[255];
    
    char *contentBegin = recordBegin;
    for(int i = 0; i < attributeVector->size(); i++)//一个一个属性操作
    {
        type = (*attributeVector)[i].type;//-1、0、正整数
        attributeName = (*attributeVector)[i].name;
        typeSize = api->typeSizeGet(type);
        
        //init content (when content is string , we can get a string easily)
        memset(content, 0, 255);
        memcpy(content, contentBegin, typeSize);//拷贝从contentBegin开始的地址的一块内容值
        for(int j = 0; j < (*conditionVector).size(); j++)//对每一个属性一个条件一个条件判断
        {
            if ((*conditionVector)[j].attributeName == attributeName)
            {
                //if this attribute need to deal about the condition
                if(!contentConditionFit(content, type, &(*conditionVector)[j]))//利用condition->ifRight(tmp)判断
                {
                    //if this record is not fit the conditon
                    return false;
                }
            }
        }

        contentBegin += typeSize;
    }
    return true;
}



//输出一条记录信息
//传入参数记录开始地址，记录大小，记录属性列表，需输出的属性列表
// 无返回值
void RecordManager::recordPrint(char* recordBegin, int recordSize, vector<Attribute>* attributeVector, vector<string> *attributeNameVector)
{
    int type;
    string attributeName;
    int typeSize;
    char content[255];
    
    char *contentBegin = recordBegin;
    for(int i = 0; i < attributeVector->size(); i++)//表的属性遍历，一条条属性来
    {
        type = (*attributeVector)[i].type;
        typeSize = api->typeSizeGet(type);//每个类型固定大小
        
        //init content (when content is string , we can get a string easily)
        memset(content, 0, 255);
        
        memcpy(content, contentBegin, typeSize);//获取地址从contentBegin开始的一块内容

        for(int j = 0; j < (*attributeNameVector).size(); j++)//attributeVector含一个表的所有属性，attributeNameVector，用户输进去的属性列
        {
            if ((*attributeNameVector)[j] == (*attributeVector)[i].name)
            {
                contentPrint(content, type);//type是一个不同输出时if函数判断条件
                break;
            }
        }
        
        contentBegin += typeSize;
    }
} 


//某一属性内容输出 
//传入参数，属性内容指针，属性类型
//无返回 
//字符串相应类型转化后格式化输出
void RecordManager::contentPrint(char * content, int type)
{
    if (type == Attribute::TYPE_INT)
    {
        //if the content is a int
        int tmp = *((int *) content);   //get content value by point
        printf("%d ", tmp);
    }
    else if (type == Attribute::TYPE_FLOAT)
    {
        //if the content is a float
        float tmp = *((float *) content);   //get content value by point
        printf("%f ", tmp);
    }
    else
    {
        //if the content is a string
        string tmp = content;
        printf("%s ", tmp.c_str());
    }

}


//判断某一属性内容是否满足某一条件
//传入参数属性内容指针，属性类型 ，条件对象指针
//返回bool值，true——满足，false——不满足
bool RecordManager::contentConditionFit(char* content,int type,Condition* condition)//借助Condition的ifRight函数 
{
    if (type == Attribute::TYPE_INT)
    {
        //if the content is a int
        int tmp = *((int *) content);   //get content value by point
        return condition->ifRight(tmp);
    }
    else if (type == Attribute::TYPE_FLOAT)
    {
        //if the content is a float
        float tmp = *((float *) content);   //get content value by point
        return condition->ifRight(tmp);
    }
    else
    {
        //if the content is a string
        return condition->ifRight(content);
    }
    return true;
}



//获得一个索引文件的文件名 
//传入参数，索引名 
string RecordManager::indexFileNameGet(string indexName)
{
    return "INDEX_FILE_"+indexName;
} 



//获得一个表文件的文件名 
//传入参数，表名
string RecordManager::tableFileNameGet(string tableName)
{
    return "TABLE_FILE_" + tableName;
}
 

