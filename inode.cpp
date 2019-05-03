#include "inode.h"
#include <string>

using namespace std; 

int init(string filename, inode* node, int numBlocks, int extra){
	for(int i = 0; i<128; i++){
		node->blocks[i] = (unsigned int)-1;
	}//initializing all blocks with a val of 2^32-1
	node->fileSize = (numBlocks*1024) + extra;
	node->name = filename;
	for(int i = 0; i<numBlocks-1;i++){
		node->blocks[i] = 0;
	}
	node->blocks[numBlocks-1] = extra;//the last block will say how many bytes hold actual data
}
