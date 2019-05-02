#include <stdio.h>
#include "segment.h"
#include "imap.h"
#include "inode.h"

void import(string filename, stirng lfs_filename, segment *s, imap * map){
	inode node;
	node.name = lfs_filename;
	string line;
	ifstream myfile(filename);
	int byteCounter = 0; 
	int blockCounter = 0;
	int lineLen;
	unsigned int tempBlocks[128];
	node.blocks[0] = s->currBlock;
	//tempBlocks[0] = s->currBlock;
	while(getline(myfile, line)){
		lineLen = sizeof(line);
		for(int i = 0; i < lineLen; i++){

			if(s->currByte == 1047551){ //if buffer== FULL-1 (add summary block)

				writeSegment(s);
				s->segNum++;
				s->currBlock += 1;
				node.blocks[blockCounter] = s->currBlock; 
				blockCounter++;
				//UPDATE Checkpoint region for clean dirty bits
			}

			s->buffer[s->currByte] = line[i];
			s->currByte++;
			byteCounter++;	
		
			if(byteCounter == 1024){ // keeps track of blocks
				//update summary block here
				s->inode[currBlock%1024] = node.nodeNum;
				s->offset[currBlock%1024] = blockCounter;
				blockCounter++;
				s->currBlock += 1;
				node.blocks[blockCounter] = s->currBlock;
			}

		}
	} 
	while(s->currByte%1024 != 0){ //MAKE SURE THAT the currByte is set at the next block
		s->currByte++;
	}
	blockCounter++;
	//we can update the mapping file once we allocate the inode
	//writing the file to the buffer
	//may have to reset this file pointer
	/*
	ifstream src(filename,"r");
	int start = s->currByte;
	int i = 0;
	int usedBlocks = 0;
	while(1){
		src >> s->buffer[currByte];
		i++;
		currByte++;
		if(i == 1024){
			usedBlocks++;
			s->currBlock++;
			//need to check if the buffer is full at this point and to add summary block?
			i = 0;
		}
	}
	src.close();
	*/
	//updating imap;
	//have to implement filename map to get what current inode were on
	int currINode = 0; //TEMPORARY NEED TO CHANGE
	map->inodes[currINode] = currBlock;
	//making a new inode and adding to buffer
	node.nodeNum = s->currByte++;
	node.fileSize = ((blockCounter-1)*1024) + byteCounter;
	for(int i = 0 ; i<256; i++){
		memcpy(s->buffer+(currBlock*1024) + i*4,map->inodes + i; 4);
	}
	currBlock++;
	if(s->currByte == 1047551){ //if buffer== FULL-1 (add summary block)
		writeSegment(s);
                s->segNum++;
                s->currBlock += 1;
                node.blocks[blockCounter] = s->currBlock;
                blockCounter++;
                //UPDATE Checkpoint region for clean dirty bits
        }
	//Update checkpoint region
	
	//update imap - done above

}

void remove(string lfs_filename){ //should only need inodes
	
}

void list(){ //should only need inodes
	
}

void shutdown(segment *s){
	writeSegment(s);
	exit(0);
}
