include <stdio.h>
#include "segment.h"
#include "imap.h"
#include "inode.h"

void import(string filename, stirng lfs_filename, segment *s){
	inode node;
	node.name = lfs_filename;
	string line;
	ifstream myfile(filename);
	int byteCounter = 0; 
	int blockCounter = 0;
	int lineLen;
	node.blocks[0] = s->currBlock;
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
	//PUT THE INODE IN THE BUFFER
	imap
	memcpy(s->buffer +blockCounter);
	//Update checkpoint region
	//update imap

}

void remove(string lfs_filename){ //should only need inodes
	
}

void list(){ //should only need inodes
	
}

void shutdown(segment *s){
	writeSegment(s);
	exit(0);
}