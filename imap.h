#ifndef _imap_h
#define _imap_h

typedef struct{
	unsigned int map[10240];
} imap;

void update(imap *map, int inode_num, unsigned int block_num);
unsigned int getInode(imap *map, int inode_num);

#endif
