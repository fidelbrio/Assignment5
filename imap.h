#ifndef IMAP_H
#define IMAP_H

typedef struct _imap{
	unsigned int inodes[10240];
} imap;

void init(imap *);
void init2(imap*, unsigned int *);
void init3(imap *, unsigned int *);
void update(imap *map, int inode_num, unsigned int block_num);
unsigned int getInode(imap *map, int inode_num);

#endif
