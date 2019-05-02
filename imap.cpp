#include "imap.h"

void update(imap *map, int inode_num, unsigned int block_num){
	*(map->inodes[inode_num]) = block_num;
}

unsigned int getInode(imap *map, int inode_num){
	return *(map->inodes[inode_num]);
}