//
//CatalogManager.cpp
//
//by 张碧雲 on 16/5/25
//

#include"CatalogManager.h" 
#include"BufferManager.h"
#include"IndexInfo.h"
#include<iostream>
#include<vector>
#include<string>
#include<cstring>
#include<sstream>
#include"Attribute.h"
#define UNKNOWN_FILE 8
#define TABLE_FILE 9
#define INDEX_FILE 10

//构造函数 、析构函数系统自动生成
CatalogManager::CatalogManager()
{

}

CatalogManager::~CatalogManager()
{

}


// 删除表的原数据文件，文件名等于表名 
// 传入参数表名
//成功返回1，否则返回0
int CatalogManager::dropTable(string tableName) 
{
	bm.delete_fileNode(tableName.c_str());//借助BufferManager删除该文件节点
	if(remove(tableName.c_str()))//remove失败返回-1 
	{
		return 0; 
	}
	return 1;
}


//获取某一索引名的类型
//传入参数索引名
//返回索引类型 ，-1,0，正整数;-2表示出错
int CatalogManager::getIndexType(string indexName) 
{
	fileNode*ftmp=bm.getFile("Indexs");//查找存放所有索引信息的文件Indexs 
	blockNode*btmp=bm.getBlockHead(ftmp);//获取文件节点的第一块块节点
	if(btmp){
		char*addressBegin;
		addressBegin=bm.get_content(*btmp);//指向块节点内容开始的地方 
		IndexInfo*i=(IndexInfo*)addressBegin;//开始地址转化为IndexInfo型指针赋值 
		for(int j=0;j<(bm.get_usingSize(*btmp)/sizeof(IndexInfo));j++)//(bm.get_usingSize(*btmp)/sizeof(IndexInfo))索引个数
		{
			if((*i).indexName==indexName){
				return i->type;
			}
			i++;//指向下一个索引信息对象 
		} 
		return -2;//没找到该索引 
	}
	return -2;//btmp为null 
}


//获取Indexs文件里所有所有信息，放在一个索引向量里
//传入参数索引对象向量，用于存放结果
//返回1，表示结束 
int CatalogManager::getAllIndex(vector<IndexInfo>*indexs) 
{
	fileNode*ftmp=bm.getFile("Indexs");
	blockNode*btmp=bm.getBlockHead(ftmp);
	if(btmp)
	{
		char*addressBegin;
		addressBegin=bm.get_content(*btmp);
		IndexInfo*i=(IndexInfo*)addressBegin;
		for(int j=0;j<(bm.get_usingSize(*btmp)/sizeof(IndexInfo));j++)
		{
			indexs->push_back((*i));//在vector类中作用为在vector尾部加入一个数据。
			i++;//移到下一个索引对象读取 			
		}
	} 
	return 1;
}


//增加索引，往Indexs文件里增加一个索引信息对象，往table的表信息的相应属性设置索引属性
//传入参数索引名，表名相对应属性，索引类型 
//成功返回1，失败返回0
int CatalogManager::addIndex(string indexName,string tableName,string Attribute,int type)
{
	fileNode*ftmp=bm.getFile("Indexs");
	blockNode*btmp=bm.getBlockHead(ftmp);
	IndexInfo i(indexName,tableName,Attribute,type);//初始化赋值一个索引对象 
	while(true)
	{
		if(btmp==NULL)
		{
			return 0;
		}
		if(bm.get_usingSize(*btmp)<=bm.getBlockSize()-sizeof(IndexInfo))
		{
			char*addressBegin;
			addressBegin=bm.get_content(*btmp)+bm.get_usingSize(*btmp);
			memcpy(addressBegin,&i,sizeof(IndexInfo));//拷贝该索引对象信息到Indexs文件空位置 
			bm.set_usingSize(*btmp,bm.get_usingSize(*btmp)+sizeof(IndexInfo));
			bm.set_dirty(*btmp);
			
			return this->setIndexOnAttribute(tableName, Attribute, indexName);//往table的表信息的相应属性设置索引属性 
		}
		else//第一块块节点空间不够,移到下一块 
		{
			btmp=bm.getNextBlock(ftmp,btmp);			
		} 
	}
	return 0;
} 


//查找是否有该表名的表;
//输入参数表名 
//没有返回零，有返回TABLE_FILE 值为9
int CatalogManager::findTable(string tableName)
{
	FILE*fp;
	fp=fopen(tableName.c_str(),"r");//试图只读打开文件名为表名的文件 
	if(fp==NULL)//打不开该文件，说明没有该表
	{
		return 0; 
	} 
	else//成功打开该文件，说明存在 
	{
		fclose(fp);
		return TABLE_FILE;		
	} 
}



//查找是否有该索引名的索引存在
//传入参数索引名
//找不到返回UNKNOWN_FILE值为8，找到返回INDEX_FILE10
int CatalogManager::findIndex(string fileName) 
{
	fileNode*ftmp=bm.getFile("Indexs");
	blockNode*btmp=bm.getBlockHead(ftmp);
	if(btmp)
	{
		char*addressBegin;
		addressBegin=bm.get_content(*btmp);
		IndexInfo*i=(IndexInfo*)addressBegin;
		int flag=UNKNOWN_FILE;//标志能否找到该索引
		for(int j=0;j<(bm.get_usingSize(*btmp)/sizeof(IndexInfo));j++)
		{
			if((*i).indexName==fileName)//找到该索引 
			{
				flag=INDEX_FILE;
				break; 
			}
			i++; //移到下一个索引文件的索引对象 
		}
		return flag; 
	}
}



//删除索引（删除Indexs文件对应的索引信息，设置表信息对应属性索引信息为空字符串）
//传入参数（索引名字） 
//成功返回1，失败返回0;
int CatalogManager::dropIndex(string indexName) 
{
	fileNode*ftmp=bm.getFile("Indexs");
	blockNode*btmp=bm.getBlockHead(ftmp);
	if(btmp)
	{
		char*addressBegin;
		addressBegin=bm.get_content(*btmp);
		IndexInfo*i=(IndexInfo*) addressBegin;
		int j;
		for(j=0;j<(bm.get_usingSize(*btmp)/sizeof(IndexInfo));j++)
		{
			if((*i).indexName==indexName)//找到该索引对象 
			{
				break;
			}
			i++; 
		}
		this->revokeIndexOnAttribute((*i).tableName,(*i).Attribute,(*i).indexName);//设置表信息对应属性索引信息为空字符串
		for(;j<(bm.get_usingSize(*btmp)/sizeof(IndexInfo)-1);j++)//把删除的索引对象后面的索引都往前移一位
		{
			(*i)=*(i+sizeof(IndexInfo));
			i++;
		}
		bm.set_usingSize(*btmp, bm.get_usingSize(*btmp) - sizeof(IndexInfo));//已使用空间减去一个索引对象空间
        bm.set_dirty(*btmp);//标志为脏块

        return 1;		
	}
	return 0;
}


//设置表信息对应属性索引信息为空字符串
//传入参数（表名，索引属性，索引名字） 
//找到索引属性返回1，否则返回0 
int CatalogManager::revokeIndexOnAttribute(string tableName,string AttributeName,string indexName) 
{
	fileNode*ftmp=bm.getFile(tableName.c_str());
	blockNode*btmp=bm.getBlockHead(ftmp);
	if(btmp)//table信息表开头放size(int型，记录数目），主键位置 ，属性数目 
	{
		char*addressBegin=bm.get_content(*btmp);
		addressBegin+=(1+sizeof(int));//略过记录数目和主键数目内容
		int size = *addressBegin;//获取属性数目
		addressBegin++;
		Attribute*a=(Attribute*)addressBegin;//转化为属性指针
		int i;
		for(i=0;i<size;i++)
		{
			if(a->name==AttributeName)
			{
				if(a->index==indexName)
				{
					a->index="";//设置属性索引为空字符串 
					bm.set_dirty(*btmp);//修改后设置为脏块 
				}
				else//出错提醒
				{
					cout<<"Revoke unknown index:"<<indexName<<" on "<<tableName<<"!"<<endl;
					cout<<"Attribute: "<<AttributeName<<" on table "<<tableName<<" has index: "<<a->index<<"!"<<endl; 
				} 
				break;
			}
			a++;//移到下一条属性 
		}
	if(i<size)//找到该属性
	   return 1; 
	else
	   return 0; 	 
	}
	return 0;
}


//获取指定某表上所有的属性(打开Indexs文件，一个个查找所有索引对象，找到表名为指定表名的索引名) 
//传入参数表名，属性向量（存放结果
//成功返回1，失败返回0; 
int CatalogManager::indexNameListGet(string tableName,vector<string>*indexNameVector)
{
	fileNode*ftmp=bm.getFile("Indexs");
	blockNode*btmp=bm.getBlockHead(ftmp);
	if(btmp)
	{
		char*addressBegin;
		addressBegin=bm.get_content(*btmp);
		IndexInfo*i=(IndexInfo*)addressBegin;
		for(int j=0;j<(bm.get_usingSize(*btmp)/sizeof(IndexInfo));j++)
		{
			if((*i).tableName==tableName)
			{
				(*indexNameVector).push_back((*i).indexName);
			} 
			i++;//到下一个索引对象 
		}
		return 1;
	}
	return 0;
}



//删除记录时将表信息记录数目减去删除的记录数目
//传入参数表名，删除记录数目
//成功返回该表删除后的记录数目，否则返回0 
int CatalogManager::deleteValue(string tableName,int deleteNum)
{
	fileNode*ftmp=bm.getFile(tableName.c_str());
	blockNode*btmp=bm.getBlockHead(ftmp);
	if(btmp)
	{
		char*addressBegin=bm.get_content(*btmp);
		int*recordNum=(int*)addressBegin;//获取表信息第一个内容记录数目
		if((*recordNum)<deleteNum)//超额报错 
		{
			cout<<"Error in CatalogManager::deleteValue"<<endl;
			return 0;
		}
		else
		{
			(*recordNum)-=deleteNum; 
		} 
		bm.set_dirty(*btmp);//标志为脏块 
		return *recordNum;
	}
	return 0; 
} 


//增加记录时将表信息记录数目加上增加的记录数目
//传入参数表名，增加的记录数目
//成功返回该表增加后的记录数目，否则返回0 
int CatalogManager::insertRecord(string tableName, int addNum)//类似减少记录数目 
{
    fileNode *ftmp = bm.getFile(tableName.c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);

    if (btmp)
    {

        char* addressBegin = bm.get_content(*btmp) ;
        int * RecordNum = (int*)addressBegin;
        *RecordNum += addNum;
        bm.set_dirty(*btmp);
        return *RecordNum;
    }
    return 0;
}


//获取某表的记录数目
//传入表名
//成功返回记录数目，失败返回零 
int CatalogManager::getRecordNum(string tableName)
{
    fileNode *ftmp = bm.getFile(tableName.c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);

    if (btmp)
    {
        char* addressBegin = bm.get_content(*btmp) ;
        int * recordNum = (int*)addressBegin;//获取表信息第一个内容记录数目
        return *recordNum;
    }
    return 0;
}


//新建一个表信息文件，存放信息，表记录数目初始化为0，主键位置，属性数目，属性对象信息 
//传入参数表名，属性列表向量，主键名（初始化为空字符串），属性位置（初始化为0）
//成功返回1，否返回0
int CatalogManager::addTable(string tableName,vector<Attribute>*attributeVector,string primaryKeyName="",int primaryKeyLocation=0)
{
	FILE *fp;
    fp = fopen(tableName.c_str(), "w+");
    if (fp == NULL)
    {
        return 0;
    }
    fclose(fp);
    fileNode *ftmp = bm.getFile(tableName.c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);
    if(btmp){
	   char*addressBegin=bm.get_content(*btmp);
	   int* size=(int*)addressBegin;
	   *size=0;// 表记录数目初始化为0
	   addressBegin+=sizeof(int);
	   *addressBegin = primaryKeyLocation;
     	addressBegin++;
       *addressBegin = (*attributeVector).size();//获取属性列表大小 ，属性数目
	   addressBegin++;
	   for(int i= 0;i<(*attributeVector).size();i++)//拷贝索引信息 
        {
            memcpy(addressBegin, &((*attributeVector)[i]), sizeof(Attribute));
            addressBegin += sizeof(Attribute);
        }
        bm.set_usingSize(*btmp, bm.get_usingSize(*btmp) + (*attributeVector).size()*sizeof(Attribute)+2+sizeof(int));//重置已使用块节点空间 
        bm.set_dirty(*btmp);//设置为脏块 
        return 1;
   }
   return 0;
} 


//设置表信息的相应属性索引名
//传入参数表名，属性名，索引名
//成功找到属性设置索引名返回 1，失败返回0
int CatalogManager::setIndexOnAttribute(string tableName,string AttributeName,string indexName) 
{
	fileNode*ftmp=bm.getFile(tableName.c_str());
	blockNode*btmp=bm.getBlockHead(ftmp);
	if(btmp)
	{
		char*addressBegin=bm.get_content(*btmp);
		addressBegin+=1+sizeof(int);
		int size =*addressBegin;//属性位置
		addressBegin++;
		Attribute*a=(Attribute*)addressBegin;
		int i;
		for(i=0;i<size;i++)
		{
			if(a->name==AttributeName)
			{
				a->index=indexName;
				bm.set_dirty(*btmp);
				break;
			}
			a++;
		}
		if(i<size)
		   return 1;
		else
		   return 0;
	}
	return 0;
}



//获得某一表名的所有属性
//输入参数表名，属性向量(存放属性）
//成功返回1，失败返回0
int CatalogManager::attributeGet(string tableName,vector<Attribute>*attributeVector)
{
	fileNode*ftmp=bm.getFile(tableName.c_str());
	blockNode*btmp=bm.getBlockHead(ftmp);
	if(btmp)
	{
		char*addressBegin=bm.get_content(*btmp);
		addressBegin+=1+sizeof(int);
		int size=*addressBegin;//获得属性数目 
		addressBegin++;
		Attribute*a=(Attribute*)addressBegin;
		for(int i=0;i<size;i++) 
		{
			attributeVector->push_back(*a);
			a++;
		}
		return 1; 
	}
	return 0; 
} 


//计算表的一条记录的大小（从表信息文件计算获得） 
//传入参数表名
//返回记录大小，失败返回零 
int CatalogManager::calcuteLenth(string tableName)
{
	fileNode*ftmp=bm.getFile(tableName.c_str());
	blockNode*btmp=bm.getBlockHead(ftmp);
	if(btmp)
	{
		int singleRecordSize=0;//存放记录大小，初始化为0
		char*addressBegin=bm.get_content(*btmp);
		addressBegin+=1+sizeof(int) ;
		int size=*addressBegin;
		addressBegin++;
		Attribute*a=(Attribute*)addressBegin;
		for(int i=0;i<size;i++)
		{
			if((*a).type==-1)
		    {
		    	singleRecordSize+=sizeof(float);
		    }
		    else if((*a).type==0)
		    {
		    	singleRecordSize+=sizeof(int);
		    }
		    else if((*a).type>0)
		    {
		    	singleRecordSize+=(*a).type*sizeof(char);
		    }
		    else{//报错 
		    	cout<<"Catalog data damaged!"<<endl; 
		    	return 0;
		    }
		    a++; 
		}
		return singleRecordSize;
	}
	return 0;
}


//计算类型大小
//传入参数类型的整数值
//输出类型大小
int CatalogManager::calcuteLenth2(int type)
{
	if(type==Attribute::TYPE_INT)
	{
		return sizeof(int);
	}
	else if(type==Attribute::TYPE_FLOAT)
	{
		return sizeof(float);
	}
	else if(type>0)
	{
		return (int)sizeof(char)*type;
	}
	else
	{
		cout<<"No this type"<<endl;
	}
}


//已经查找获取记录内容，存放在recordContent向量里，将记录内容连成一个字符串并存在结果字符串里 
//传入参数表名，记录内容指针向量（由一个个属性值组成），结果字符串指针
//无返回
void CatalogManager::recordStringGet(string tableName,vector<string>*recordContent,char*recordResult) 
{
	vector<Attribute>attributeVector;
	attributeGet(tableName,&attributeVector);//获取一个表的所有属性列表向量
	char* contentBegin=recordResult;//保证recordResult一直指向结果开始，由contentBegin移动
	for(int i=0;i<attributeVector.size();i++) //默认属性数目与记录内容数目相等 
	{
		Attribute attribute=attributeVector[i];
		string content =(*recordContent)[i];//获取一条内容
		int type=attribute.type;
		int typeSize=calcuteLenth2(type);//计算类型大小，用于拷贝内容时 
		stringstream ss;
		ss<<content;
		if(type==Attribute::TYPE_INT)
		{
			int intTmp;
			ss>>intTmp;//内容值
			memcpy(contentBegin,((char*)&intTmp),typeSize);
		}
		else if (type == Attribute::TYPE_FLOAT)
        {
            float floatTmp;
            ss >> floatTmp;
            memcpy(contentBegin, ((char*)&floatTmp), typeSize);
        }
        else//为字符类型可直接拷贝 
        {
            memcpy(contentBegin, content.c_str(), typeSize);
        }
		contentBegin+=typeSize;//移动结果字符串到下一个可以拷贝内容的位置 
	}
	return; 	
}
