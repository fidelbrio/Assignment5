#include <stdio.h>
#include "segment.h"
#include "imap.h"
#include "inode.h"
#include <string>
#include <string.h>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

/*int main(int argc, char * argv[]){
	segment s;
	imap map;
	init(&map);
	import2(argv[1],argv[2],&s,&map);
}*/
unsigned int findFreeInode(imap *map){
	int index = 0;
	while(true){
		if(map->inodes[index] == 0){
			return index;
		}
		index++;
	}
}

void import2(string filename, string lfs_filename, segment *s, imap *map){
	inode node;
	node.name = lfs_filename;
	//find iNode number from mappings file
	int infile = open(filename.c_str(), O_RDONLY);
	int numBlocks = 0;
	int start = s->currBlock;
	int bytesRead;
	node.nodeNum = findFreeInode(map);
	//cout<<inodeNum<<endl;
	FILE* mapper;
	mapper = fopen("./DRIVE/FILE_MAP","r+b");
	fseek(mapper,132*node.nodeNum,SEEK_SET);
	fputs(node.name.c_str(),mapper);
	fseek(mapper, (132*node.nodeNum)+128, SEEK_SET);
	fputs(to_string(node.nodeNum).c_str(),mapper);
	while(1){
		bytesRead = read(infile,s->buffer + ((s->currBlock) * 1024), 1024);
		node.blocks[numBlocks] = (unsigned int)((s->segNum*1024)+s->currBlock);
		s->offset[s->currBlock] = numBlocks;
		numBlocks++;
		s->inode[s->currBlock] = node.nodeNum;
		s->currBlock++;
		if(s->currBlock == 1015){
			memcpy(s->buffer+(s->currBlock*1024), s->inode, 4096);
			memcpy(s->buffer+(1019*1024), s->offset, 4096);
			writeSegment(s);
			s->currBlock = 0;
			s->segNum+=1;
		}
		//check if currBlock == 1024, and if so, write it out
		if(bytesRead <1024) break;
	}
	//close(infile);
	node.fileSize = (numBlocks*1024) + bytesRead;
	memcpy(s->buffer+(s->currBlock*1024), &node,sizeof(node));
	s->inode[s->currBlock] = node.nodeNum; 
	s->offset[s->currBlock] = numBlocks;
	numBlocks++;
	s->currBlock++;
	if(s->currBlock == 1015){
		memcpy(s->buffer+(s->currBlock*1024), s->inode, 4096);
		memcpy(s->buffer+(1019*1024), s->offset, 4096);	
		writeSegment(s);
		s->currBlock = 0;
		s->segNum += 1;

	}

	map->inodes[node.nodeNum] = (unsigned int)((s->segNum*1024)+ s->currBlock);
	//for(int i = 0; i<256;i++){
	memcpy(s->buffer+(s->currBlock*1024),map->inodes + node.nodeNum, 1024);
	s->inode[s->currBlock] = node.nodeNum; 
	s->offset[s->currBlock] = numBlocks;
	numBlocks++;
	s->currBlock++;
	if(s->currBlock == 1015){
		memcpy(s->buffer+(s->currBlock*1024), s->inode, 4096);
		memcpy(s->buffer+(1019*1024), s->offset, 4096);
		writeSegment(s);
		s->currBlock = 0;
		s->segNum+=1;
	}

	close(infile);
	unsigned int imapLocation = (s->segNum *1024) + s->currBlock;
	s->inode[s->currBlock] = node.nodeNum;
	s->offset[s->currBlock] = numBlocks;
	s->currBlock++;
	numBlocks++;
	if(s->currBlock == 1015){
		memcpy(s->buffer+(s->currBlock*1024), s->inode, 4096);
		memcpy(s->buffer+(1019*1024), s->offset, 4096);
		writeSegment(s);
		s->currBlock = 0;
		s->segNum+=1;
	}
	int check = open("DRIVE/CHECKPOINT_REGION",O_RDWR);
	lseek(check,(node.nodeNum/40)*4,SEEK_SET);
	write(check,&imapLocation,4);
	close(check);
	writeSegment(s);
}
void remove(string lfs_filename){ //should only need inodes
	
}

void list(imap map){ //should only need inodes
	ifstream mapper;
	ifstream cr;
 	mapper.open("./DRIVE/FILE_MAP",ios::in|ios::binary);
	for(int i = 0; i<10240; i++){
		//mapper.seekg(132*i,mapper.beg);
		char name[128];
		unsigned int addr;
		unsigned int seg;
		for(int j = 0; j<128;j++){
			mapper >> name[j];
		}
		mapper >> addr;
		cout<<name<<endl;
		cout<<addr<<endl;
		cr.open("./DRIVE/CHECKPOINT_REGION",ios::in|ios::binary);
		cr.seekg(4*addr,cr.beg);
		cr>>seg;
		unsigned int segNum = seg/1024;
		unsigned int offset = seg%1024;
		cout<<segNum<<endl;
		cout<<offset<<endl;


		if(i == 10) break;
	}
}

void shutdown(segment *s){
	writeSegment(s);
	exit(0);
}


int main(int argc, char * argv[]){
        segment s;
        imap map;
        init(&map);
	initializeSegment(&s);
        import2(argv[1],argv[2],&s,&map);
	import2(argv[3],argv[4],&s,&map);
	writeSegment(&s);
	s.currBlock= 0;
	s.segNum++;
	list(map);
	//list();

}

