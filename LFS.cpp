#include <stdio.h>
#include "segment.h"
#include "imap.h"
#include "inode.h"
#include <string>
#include <string.h>
#include <fstream>
#include <iostream>

using namespace std;

/*int main(int argc, char * argv[]){
	segment s;
	imap map;
	init(&map);
	import(argv[1],argv[2],&s,&map);
}*/


void import2(string filename, string lfs_filename, segment *s, impa *map){
	

void import(string filename, string lfs_filename, segment *s, imap * map){
	s->buffer[0] = 'a';
	s->currByte++;
	inode node;
	node.name = lfs_filename;
	node.nodeNum = s->currInode;
	string line;
	ifstream myfile(filename);
	int byteCounter = 0; 
	int blockCounter = 0;
	int lineLen;
	unsigned int tempBlocks[128];
	node.blocks[0] = s->currBlock;
	//tempBlocks[0] = s->currBlock;

	char temp;
	myfile >> temp;

//	while(getline(myfile, line)){
	while(temp != EOF){
		//cout<<line<<endl;
		lineLen = sizeof(line);
//		for(int i = 0; i < lineLen; i++){

			if(s->currByte == 1048576){ //if buffer== FULL-1 (add summary block)

				writeSegment(s);
				s->segNum++;
				s->currBlock += 1;
				node.blocks[blockCounter] = s->currBlock; 
				blockCounter++;
				//UPDATE Checkpoint region for clean dirty bits
			}
/*			cout<<"line[i] is: "<<line[i]<<endl;
			cout<< "s->currByte is: "<< s->currByte<<endl;
			s->buffer[s->currByte] = line[i];
			cout<< "s->buffer[s-currByte] is: "<< s->buffer[s->currByte]<<endl;
			s->currByte++;
			byteCounter++;	
*/			s->buffer[s->currByte] = temp;
			s->currByte++;


			if(byteCounter == 1024){ // keeps track of blocks
				//update summary block here
				s->inode[s->currBlock%1024] = node.nodeNum;
				s->offset[s->currBlock%1024] = blockCounter;
				blockCounter++;
				s->currBlock += 1;
				node.blocks[blockCounter] = s->currBlock;
			}


			myfile>>temp;

//		}
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
	map->inodes[currINode] = s->currBlock;
	//making a new inode and adding to buffer
	node.nodeNum = s->currByte++;
	node.fileSize = ((blockCounter-1)*1024) + byteCounter;
	for(int i = 0 ; i<256; i++){
		memcpy(s->buffer+(s->currBlock*1024) + i*4,map->inodes + i, 4);
	}
	s->currBlock++;
	if(s->currByte == 1047551){ //if buffer== FULL-1 (add summary block)
		writeSegment(s);
                s->segNum++;
                s->currBlock += 1;
                node.blocks[blockCounter] = s->currBlock;
                blockCounter++;
                //UPDATE Checkpoint region for clean dirty bits
        }
	//Update checkpoint region
	FILE * check;
	check = fopen("DRIVE/CHECKPOINT_REGION","r+");
	fseek(check,(currINode/40)*sizeof(int),SEEK_SET);
	unsigned int location = (unsigned int)((s->segNum*1024)+(s->currBlock-1));
	fwrite(&location,4,1,check);
	fclose(check);
	//check.close();
	//update imap - done above
	s->buffer[0] = 'g';
	s->buffer[1] = 'r';
	writeSegment(s);

}

void remove(string lfs_filename){ //should only need inodes
	
}

void list(){ //should only need inodes
	
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
        import(argv[1],argv[2],&s,&map);
	writeSegment(&s);

}

