#ifndef IMAP_H
#define IMAP_H

typedef struct _imap{
	unsigned int map[10240];
} imap;

void init(imap *);
void update(imap *map, int inode_num, unsigned int block_num);
unsigned int getInode(imap *map, int inode_num);

#endif
