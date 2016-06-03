#include "BufferManager.h"
#include "Minisql.h"
#include <stdlib.h>
#include <string>
#include <cstring>
#include <queue>

//Constructor Function:allocate memories for the pools and init the variable values
BufferManager::BufferManager() :totalBlock(0), totalFile(0), fileHead(NULL)
{
    for (int i = 0; i < MAX_FILE_NUM; i++)
    {
        filePool[i].fileName = new char[MAX_FILE_NAME];
        if (filePool[i].fileName == NULL)
        {
            printf("Can not allocate memory in initing the file pool!\n");
            exit(1);
        }
        init_file(filePool[i]);
    }
    for (int i = 0; i < MAX_BLOCK_NUM; i++)
    {
        blockPool[i].address = new char[BLOCK_SIZE];
        if (blockPool[i].address == NULL)
        {
            printf("Can not allocate memory in initing the block pool!\n");
            exit(1);
        }
        blockPool[i].fileName = new char[MAX_FILE_NAME];
        if (blockPool[i].fileName == NULL)
        {
            printf("Can not allocate memory in initing the block pool!\n");
            exit(1);
        }
        init_block(blockPool[i]);
    }
}

//Destructor : free memories of the pools and write back the buffer content if required
BufferManager::~BufferManager()
{
    writtenBackToDiskAll();
    for (int i = 0; i < MAX_FILE_NUM; i++)
    {
        delete[] filePool[i].fileName;
    }
    for (int i = 0; i < MAX_BLOCK_NUM; i++)
    {
        delete[] blockPool[i].address;
    }
}

// init the filenode values
// @param fileNode& the FILE you want to init
// @return void
void BufferManager::init_file(fileNode &file)
{
    file.nextFile = NULL;
    file.preFile = NULL;
    file.blockHead = NULL;
    //file.pin = false;
    memset(file.fileName, 0, MAX_FILE_NAME);
}

//init the blockNode values
//@param blockNode& the block you want to init
//@return void
void BufferManager::init_block(blockNode &block)
{
    memset(block.address, 0, BLOCK_SIZE);
    size_t init_usage = 0;
    memcpy(block.address, (char*)init_usage, sizeof(size_t));//set the block head
    block.using_size = sizeof(size_t);
    block.dirty = false;
    block.nextBlock = NULL;
    block.preBlock = NULL;
    block.offsetNum = -1;
    block.pin = false;
    block.reference = false;
    block.ifbottom = false;
    memset(block.fileName, 0, MAX_FILE_NAME);
}

//Get a fileNode
//1.If the file is already in the list,return this fileNode
//2.If the file is not in the list,replace some fileNode, if required, to make more space
//*
//@param const char* the file name
//@param bool if true, the file will be locked
//*
//@return fileNode*
fileNode* BufferManager::getFile(const char *fileName, bool ifPin)
{
    blockNode *btmp = NULL;
    fileNode *ftmp = NULL;
    if (fileHead != NULL)
    {
        for (ftmp = fileHead; ftmp != NULL; ftmp = ftmp->nextFile)
        {
            if (!strcmp(fileName, ftmp->fileName))// the fileNode is already in the list
            {
                ftmp->pin = ifPin;
                return ftmp;
            }
        }
    }
    //the fileNode is not in the list
    if (totalFile == 0)//No file in the list now
    {
        ftmp = &filePool[totalFile];
        totalFile++;
        fileHead = ftmp;
    }
    else if (totalFile<MAX_FILE_NUM)//There are empty fileNode in the pool
    {
        ftmp = &filePool[totalFile];
        //add this fileNode into the tail of the list
        filePool[totalFile - 1].nextFile = ftmp;
        ftmp->preFile = &filePool[totalFile - 1];
        totalFile++;
    }
    else //If the file>=MAX_FILE_NUM, find one fileNode to replace, write back the block node belonging to the fileNode
    {
        ftmp = fileHead;
        while (ftmp->pin)
        {
            if (ftmp->nextFile)
                ftmp = ftmp->nextFile;
            else//no enough file node in the pool
            {
                printf("There are no enough file node in the pool!");
                exit(2);                    
            }           
        }
        for (btmp = ftmp->blockHead; btmp != NULL; btmp = btmp->nextBlock)
        {
            //思路是把当前位置的前一个替换掉（因为这个在上一次循环的时候已经被写回磁盘中了），这次把当前位置的写回磁盘中
            if (btmp->preBlock)
            {
                init_block(*(btmp->preBlock));
                totalBlock--;
            }
            writtenBackToDisk(btmp->fileName, btmp);
        }
        //最后把原来的文件名给清空
        init_file(*ftmp);
    }
    if (strlen(fileName) + 1 > MAX_FILE_NAME)
    {
        printf("文件名长度过长，最高不能超过%d\n", MAX_FILE_NAME);
        exit(3);
    }
    strncpy(ftmp->fileName, fileName, MAX_FILE_NAME);
    set_pin(*ftmp,ifPin);
    return ftmp;
}

//Get a block node
//1.If the block is already in the list, return this blockNode
//2.If the block is not in the list, replace some fileNode, using LRU replacement, if required, to make more space
//3.Only if the block is dirty, he will be written back to disk when been replaced
//*
//@param fileNode * the file you want to add the block into 
//@param blockNode the position that the Node will added to 
//@param bool if true, the block will be locked
//*
//@return *blockNode
blockNode* BufferManager::getBlock(fileNode *file, blockNode *position, bool ifPin)
{
    const char* fileName = file->fileName;
    blockNode* btmp = NULL;
    if (totalBlock == 0)
    {
        btmp = &blockPool[0];
        totalBlock++;
    }
    else if (totalBlock < MAX_BLOCK_NUM)//there are empty blocks in the block pool
    {
        for (int i = 0; i < MAX_BLOCK_NUM; i++)
        {
            //说明此位置没有放置block
            if (blockPool[i].offsetNum == -1)
            {
                btmp = &blockPool[i];
                totalBlock++;
                break;
            }
            else continue;
        }
    }
    else //totalBlock>=MAX_BLOCK_NUM,which means that there are no empty block so we must replace one.
    {
        int i = replaced_block;
        while (true)
        {
            i++;
            if (i >= totalBlock)i = 0;
            if (!blockPool[i].pin)
            {
                if (blockPool[i].reference == true)
                    blockPool[i].reference = false;
                else //choose this block for replacement 
                {
                    btmp = &blockPool[i];
                    if (btmp->nextBlock)
                        btmp->nextBlock->preBlock = btmp->preBlock;
                    if (btmp->preBlock)
                        btmp->preBlock->nextBlock = btmp->nextBlock;
                    if (file->blockHead == btmp)
                        file->blockHead = btmp->nextBlock;
                    replaced_block = i;//record the replaced block and begin from the next block the next time
                    writtenBackToDisk(btmp->fileName, btmp);
                    init_block(*btmp);
                    break;
                }
            }//this block is been locked 
            else continue;
        }
    }

    //add the block into the block list,把btmp插到position的后面
    if (position != NULL && position->nextBlock == NULL)
    {
        btmp->preBlock = position;
        position->nextBlock = btmp;
        btmp->offsetNum = position->offsetNum + 1;
    }
    else if (position != NULL && position->nextBlock != NULL)
    {
        btmp->preBlock = position;
        btmp->nextBlock = position->nextBlock;
        position->nextBlock->preBlock = btmp;
        position->nextBlock = btmp;
        btmp->offsetNum = position->offsetNum + 1;        
    }
    else //the block will be the head of the list
    {
        btmp->offsetNum = 0;
        if (file->blockHead)//if the file has a wrong block head
        {
            file->blockHead->preBlock = btmp;
            btmp->nextBlock = file->blockHead;
        }
        file->blockHead = btmp;
    }
    set_pin(*btmp, ifPin);
    if (strlen(fileName) + 1 > MAX_FILE_NAME)
    {
        printf("文件名长度过长，最高不能超过%d\n", MAX_FILE_NAME);
        exit(3);
    }
    strncpy(btmp->fileName, fileName,MAX_FILE_NAME);

    //read the file content to the block
    FILE *fileHandle;
    if ((fileHandle = fopen(fileName, "ab+")) != NULL)
    {
        if (fseek(fileHandle, btmp->offsetNum*BLOCK_SIZE, 0) == 0)
        {
            if (fread(btmp->address, 1, BLOCK_SIZE, fileHandle) == 0)
                btmp->ifbottom = true;
            btmp->using_size = getUsingSize(btmp);
        }
        else
        {
            printf("Problem seeking the file %s in reading", fileName);
            exit(1);
        }
        fclose(fileHandle);
    }
    else
    {
        printf("Problem opening the file %s in reading", fileName);
        exit(1);
    }
    return btmp;
}

//Flush the block node to the disk, if the block is not dirty, do nothing.
//*
//@param const char* the file name
//@param blockNode& the block you want to flush
//*
//@return void
void BufferManager::writtenBackToDisk(const char* fileName, blockNode* block)
{
    if (!block->dirty)//this block is not been modified, so it do not need to written back to files
    {
        return;
    }
    else //written back to the file
    {
        FILE* fileHandle = NULL;
        if ((fileHandle = fopen(fileName, "rb+")) != NULL)
        {
            if (fseek(fileHandle, block->offsetNum*BLOCK_SIZE, 0) == 0)
            {
                if (fwrite(block->address, block->using_size + sizeof(size_t), 1, fileHandle) != 1)
                {
                    printf("Problem writing the file %s in written back to disk", fileName);
                    exit(1);
                }
            }   
            fclose(fileHandle);
        }
        else
        {
            printf("Problem opening the file %s in written back to disk", fileName);
            exit(1);
        }
    }
}

//flush the block node to the disk
//@return void
void BufferManager::writtenBackToDiskAll()
{
    blockNode *btmp = NULL;
    fileNode *ftmp = NULL;
    if (fileHead)
    {
        for (ftmp = fileHead; ftmp != NULL; ftmp = ftmp->nextFile)
        {
            if (ftmp->blockHead)
            {
                for (btmp = ftmp->blockHead; btmp != NULL; btmp = btmp->nextBlock)
                {
                    if (btmp->preBlock)
                        init_block(*(btmp->preBlock));
                    writtenBackToDisk(btmp->fileName, btmp);
                }
            }
        }
    }
}

//Get the next block node of the node inputed
//@param fileNode* the file you want to add a block
//@param blockNode& the block you want to be added to
//@return blockNode*
blockNode* BufferManager::getNextBlock(fileNode *file, blockNode *block)
{
    if (block->nextBlock == NULL)
    {
        if (block->ifbottom)
            block->ifbottom = false;
        return getBlock(file, block);
    }
    else
    {
        if (block->offsetNum == block->nextBlock->offsetNum - 1)
        {
            return block->nextBlock;
        }
        else //the block list is not in the right order
        {
            return getBlock(file, block);
        }
    }
}

//Get the block of the file by offset number 
//@param fileNode*
//@param int offsetNumber
//@return blockNode*
blockNode* BufferManager::getBlockByOffset(fileNode* file, int offsetNumber)
{
    blockNode *btmp = NULL;
    if (offsetNumber == 0)
        return getBlockHead(file);
    else
    {
        btmp = getBlockHead(file);
        while (offsetNumber > 0)
        {
            btmp = getNextBlock(file, btmp);
            offsetNumber --;
        }
        return btmp;
    }
}

//Get the head block of the file 
//@param fileNode *
//@return blockNode*
blockNode* BufferManager::getBlockHead(fileNode* file)
{
    blockNode* btmp = NULL;
    if (file->blockHead != NULL)
    {
        if (file->blockHead->offsetNum == 0)//the right offset of the first block
        {
            btmp = file->blockHead;
        }
        else
        {
            btmp = getBlock(file, NULL);
        }
    }
    else //if the file have no block head, get a new node for it
    {
        btmp = getBlock(file, NULL);
    }
    return btmp;
}

//delete the file node and its block node 
//@param const char *fileName
//@return 
void BufferManager::delete_fileNode(const char *fileName)
{
    fileNode *ftmp = getFile(fileName);
    blockNode *btmp = getBlockHead(ftmp);
    queue<blockNode*> blockQ;
    while (true)
    {
        if (btmp == NULL)
            return;
        blockQ.push(btmp);
        if (btmp->ifbottom) break;
        btmp = getNextBlock(ftmp, btmp);
    }
    totalBlock -= blockQ.size();
    while (!blockQ.empty())
    {
        init_block(*blockQ.back());
        blockQ.pop();
    }
    if (ftmp->preFile)
        ftmp->preFile->nextFile = ftmp->nextFile;
    if (ftmp->nextFile)
        ftmp->nextFile->preFile = ftmp->preFile;
    if (fileHead == ftmp)
        fileHead = ftmp->nextFile;
    init_file(*ftmp);
    totalFile--;       
}

void BufferManager::set_pin(blockNode &block, bool pin)
{
    block.pin = pin;
    if (!pin)
        block.reference = true;
}

void BufferManager::set_pin(fileNode &file, bool pin)
{
    file.pin = pin;
}

//set the block dirty node 
//must call this function if modify the block node 
//@param blockNode& the block you want to the added modify
//@return void
void BufferManager::set_dirty(blockNode &block)
{
    block.dirty = true;
}

void BufferManager::clean_dirty(blockNode &block)
{
    block.dirty = false;
}

size_t BufferManager::getUsingSize(blockNode* block)
{
    return *(size_t*)block->address;
}

void BufferManager::set_usingSize(blockNode &block, size_t usage)
{
    block.using_size = usage;
    memcpy(block.address, (char*)&usage, sizeof(size_t));
}

size_t BufferManager::get_usingSize(blockNode &block)
{
    return block.using_size;
}

//get the content of the block except the block head 
//@param blockNode&
//@return 
char* BufferManager::get_content(blockNode &block)
{
    return block.address + sizeof(size_t);
}
