#include "imap.h"
#include <iostream>
#include <fstream>

using namespace std;

void update(imap *map, int inode_num, unsigned int block_num){
	map->inodes[inode_num] = block_num;
}

unsigned int getInode(imap *map, int inode_num){
	return map->inodes[inode_num];
}

void init(imap *map){
	ifstream infile("DRIVE/CHECKPOINT_REGION",ifstream::binary);
	unsigned int buff;
	for(int i = 0; i < 40; i++){
		infile.read(reinterpret_cast<char *>(&buff),4);
		int segment = buff/1024;
		int offset = buff%1024;
		string name = "DRIVE/SEGMENT";
		name+=to_string(buff);
		FILE *fp = fopen(name,"rb");
		fseek(fp,offset*1024, SEEK_SET);
		for(int j = 0; j<256; j++){
			fread(&map->inodes[(i*256) + j],4,1,fp);
		}
		fclose(fp);
	}
	infile.close();
}
