#ifndef INODE_H
#define INODE_H
#include <string>
typedef struct _inode{
	int nodeNum;
	char name[128];
	//std::string name; //name of the file
	int fileSize; // size of the file
	unsigned int blocks[128]; //files have a max size of 128K
	char DUMMY[376];
} inode;

int init(std::string, inode *, int, int); //filename, inode pointer, number of blocks, number of extra bytes (less than a block)




#endif
