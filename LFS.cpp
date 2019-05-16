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
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <bitset>
#include <utility>

using namespace std;
char clean_segs[64];
unsigned int checkpoint_region[40];
char file_map[337920*4];
bool clean = false;

unsigned int findFreeInode(imap *map){
	unsigned int index = 0;
	while(true){
		if(map->inodes[index] == 0){
			return index;
		}
		index++;
	}
}

void checkIn3(){
	ifstream cr("./DRIVE/CHECKPOINT_REGION",ios::in|ios::binary);
	cr.read((char *)&checkpoint_region,160);
	cr.close();

	ifstream cr2("./DRIVE/CR2",ios::in|ios::binary);
	cr2.read(clean_segs,64);
	cr2.close();
}


void checkOut3(){
	ofstream cr("./DRIVE/CHECKPOINT_REGION",ios::out|ios::binary);
	cr.write((char *)&checkpoint_region,160);
	cr.close();

	ofstream cr2("./DRIVE/CR2",ios::out|ios::binary);
	cr2.write(clean_segs,64);
	cr2.close();

}


void fileIn2(){
	ifstream map("./DRIVE/FILE_MAP",ios::binary|ios::in);
	map.read(file_map,1351680);
	map.close();
}




void fileOut2(){
	ofstream map("./DRIVE/FILE_MAP",ios::out|ios::binary);
	map.write((char *)&file_map,337920*4);
	map.close();
}

int checkSize(string filename){
	ifstream infile(filename, ios::binary);
	infile.seekg(0,ios::end);
	int size = infile.tellg();
	infile.close();
	return size;
}


void cleanSeg(imap * map){
	int index = 0;
	for(int i = 0; i<64;i++){
		if((int)clean_segs[index] == 0){
			index = i;
			break;
		}
	}
	int dirtyIndex;
	for(int i = 63; i>=0;i--){
		if((int)clean_segs[index] == 1){
			dirtyIndex = i;
			break;
		}
	}
	//tempSeg and SSB will hold dirty data
	unsigned int tempSeg[1016*256];
	pair<int,int> SSB[1024];
	segment cleanSegment;
	initializeSegment2(&cleanSegment);
	string filename = "./DRIVE/SEGMENT";
	filename = filename + to_string(dirtyIndex);
	FILE * currentSeg;
	currentSeg = fopen(filename.c_str(),"rb");
	if(currentSeg == NULL){
		cout<<"Input segment file does not exist"<<endl;
		exit(1);
	}
	fread(tempSeg,1016*1024,1,currentSeg);
	fread(SSB,1024*8,1,currentSeg);
	fclose(currentSeg);
	bool checkWrite = false;
	int remainder;
	clean_segs[dirtyIndex] = (char)0;
	clean_segs[cleanSegment.segNum] = (char)1;
	for(int i = 0; i<1016; i++){
		if(cleanSegment.currBlock == 1014){
			checkWrite = true;
			remainder = i+1;
		}
		int address = dirtyIndex * 1024;
		if(SSB[i].first > -3){
			if(SSB[i].first > -1){
				address += i;
				int inodeLoc = map->inodes[SSB[i].first];

				inode * temp = (inode *)malloc(sizeof(inode));
				if(inodeLoc/1024 == dirtyIndex){
					memcpy(temp,tempSeg+((inodeLoc%1024)*256),sizeof(inode));
				}else{
					string filename2 = "./DRIVE/SEGMENT";
					filename2 = filename2 +to_string(inodeLoc/1024);
					FILE * infile;
					infile = fopen(filename2.c_str(),"rb");
					fseek(infile,inodeLoc%1024,SEEK_SET);
					fread(temp,sizeof(inode),1,infile);
					fclose(infile);
				}
				if(temp->blocks[SSB[i].second] == address){
					memcpy(cleanSegment.buffer+((cleanSegment.currBlock)*256), tempSeg+(i*256),1024);
					cleanSegment.currBlock++;
					pair<int,int> fillPair(SSB[i].first,SSB[i].second);
					cleanSegment.segmentSummary[cleanSegment.currBlock-1] = fillPair;
					if(checkWrite){
						for(int j =0; j<1016;j++){
							if(cleanSegment.segmentSummary[j].first == temp->nodeNum){
									temp->blocks[cleanSegment.segmentSummary[j].second] = (cleanSegment.segNum*1024) + j;
							}
						}
						map->inodes[temp->nodeNum] = (cleanSegment.segNum * 1024) + (cleanSegment.currBlock);
						pair<int,int> fillPair2(-2,temp->nodeNum);
						cleanSegment.segmentSummary[cleanSegment.currBlock] = fillPair2;
						memcpy(cleanSegment.buffer+((cleanSegment.currBlock)*256),temp,sizeof(inode));
						cleanSegment.currBlock++;
						memcpy(file_map+((temp->nodeNum*132) + 128),&(temp->nodeNum),4);
						memcpy(cleanSegment.buffer+(cleanSegment.currBlock*256),map->inodes+((temp->nodeNum/256)*256),1024);
						pair<int,int> fillPair3(-1,temp->nodeNum/256);
						cleanSegment.segmentSummary[cleanSegment.currBlock] = fillPair3;
						checkpoint_region[temp->nodeNum/256] = (cleanSegment.segNum*1024) + (cleanSegment.currBlock);
						cleanSegment.currBlock++;
					}
				}
			}else if(SSB[i].first == -2){
				address += i;
				if(map->inodes[SSB[i].second] == address){
					inode * temp = (inode *)malloc(sizeof(inode));
					memcpy(temp,tempSeg+(i*256),sizeof(inode));
					for(int j =0; j<1016;j++){
						if(cleanSegment.segmentSummary[j].first == SSB[i].second){
							temp->blocks[cleanSegment.segmentSummary[j].second] = (cleanSegment.segNum*1024) + j;
						}
					}
					map->inodes[SSB[i].second] = (cleanSegment.segNum*1024) + (cleanSegment.currBlock);
					cleanSegment.currBlock++;
					pair<int,int> fillPair(-2,SSB[i].second);
					cleanSegment.segmentSummary[cleanSegment.currBlock-1] = fillPair;
					memcpy(cleanSegment.buffer+((cleanSegment.currBlock-1)*256),temp,sizeof(inode));
					memcpy(file_map+((temp->nodeNum*132) + 128),&(temp->nodeNum),4);
					memcpy(cleanSegment.buffer+(cleanSegment.currBlock*256),map->inodes+((SSB[i].second/256)*256),1024);
					pair<int,int> fillPair2(-1,SSB[i].second/256);
					cleanSegment.segmentSummary[cleanSegment.currBlock] = fillPair2;
					checkpoint_region[SSB[i].second/256] = (cleanSegment.segNum*1024) + (cleanSegment.currBlock);
					cleanSegment.currBlock++;
				}
			}else{
				address+=i;
				if(checkpoint_region[SSB[i].second] == address){
					checkpoint_region[SSB[i].second] = (cleanSegment.segNum*1024) + (cleanSegment.currBlock);
					pair<int,int> fillPair(-1,SSB[i].second);
					cleanSegment.segmentSummary[cleanSegment.currBlock] = fillPair;
					memcpy(cleanSegment.buffer+(cleanSegment.currBlock*256),map->inodes+(SSB[i].second*256),1024);
					cleanSegment.currBlock++;
				}
			}
		}
		if(checkWrite){
			memcpy(cleanSegment.buffer+(1016*256), cleanSegment.segmentSummary, 1024*8);
			break;
		}
		if(i == 1015 && cleanSegment.currBlock > 1010){
			i = 0;
			for(int b = 63; b>=0;b--){
				if((int)clean_segs[index] == 1){
				dirtyIndex = b;
				break;
				}
			}
			//unsigned int tempSeg[1016*256];
			//pair<int,int> SSB[1024];
			filename = "./DRIVE/SEGMENT";
			filename = filename + to_string(dirtyIndex);
			//FILE * currentSeg;
			currentSeg = fopen(filename.c_str(),"rb");
			if(currentSeg == NULL){
				cout<<"Input segment file does not exist"<<endl;
				exit(1);
			}
			fread(tempSeg,1016*1024,1,currentSeg);
			fread(SSB,1024*8,1,currentSeg);
			fclose(currentSeg);
			clean_segs[dirtyIndex] = (char)0;
		}
	}
	fileOut2();
	checkOut3();
	clean_segs[cleanSegment.segNum] = (char)1;
	writeSegment(&cleanSegment);
	if(checkWrite){
		segment cleanSegment2;
		initializeSegment2(&cleanSegment2);
		for(int k = remainder; k<1016;k++){
			int address = dirtyIndex * 1024;
			if(SSB[k].first > -3){
				if(SSB[k].first > -1){
					address += k;
					int inodeLoc = map->inodes[SSB[k].first];
					inode * temp = (inode *)malloc(sizeof(inode));
					if(inodeLoc/1024 == dirtyIndex){
						memcpy(temp,tempSeg+((inodeLoc%1024)*256),sizeof(inode));
					}else{
						string filename2 = "./DRIVE/SEGMENT";
						filename2 = filename2 +to_string(inodeLoc/1024);
						//cout<<filename2<<endl;
						FILE * infile;
						infile = fopen(filename2.c_str(),"rb");
						fseek(infile,inodeLoc%1024,SEEK_SET);
						fread(temp,sizeof(inode),1,infile);
						fclose(infile);
					}
					if(temp->blocks[SSB[k].second] == address){
						memcpy(cleanSegment2.buffer+((cleanSegment2.currBlock)*256), tempSeg+(k*256),1024);
						cleanSegment2.currBlock++;
						pair<int,int> fillPair(SSB[k].first,SSB[k].second);
						cleanSegment2.segmentSummary[cleanSegment2.currBlock-1] = fillPair;
					}
				}else if(SSB[k].first == -2){
					address += k;
					if(map->inodes[SSB[k].second] == address){
						//LIVE INODE
						inode * temp = (inode *)malloc(sizeof(inode));
						memcpy(temp,tempSeg+(k*256),sizeof(inode));
						for(int j =0; j<1016;j++){
							if(cleanSegment2.segmentSummary[j].first == SSB[k].second){
								temp->blocks[cleanSegment2.segmentSummary[j].second] = (cleanSegment2.segNum*1024) + j;
							}
						}
						map->inodes[SSB[k].second] = (cleanSegment2.segNum*1024) + (cleanSegment2.currBlock);
						cleanSegment2.currBlock++;
						pair<int,int> fillPair(-2,SSB[k].second);
						cleanSegment2.segmentSummary[cleanSegment2.currBlock-1] = fillPair;
						memcpy(cleanSegment2.buffer+((cleanSegment2.currBlock-1)*256),temp,sizeof(inode));
						memcpy(file_map+((temp->nodeNum*132) + 128),&(temp->nodeNum),4);
						memcpy(cleanSegment2.buffer+(cleanSegment2.currBlock*256),map->inodes+((SSB[k].second/256)*256),1024);
						pair<int,int> fillPair2(-1,SSB[k].second/256);
						cleanSegment2.segmentSummary[cleanSegment2.currBlock] = fillPair2;
						checkpoint_region[SSB[k].second/256] = (cleanSegment2.segNum*1024) + (cleanSegment2.currBlock);
						cleanSegment.currBlock++;
					}
				}else{
					address+=k;
					if(checkpoint_region[SSB[k].second] == address){
						checkpoint_region[SSB[k].second] = (cleanSegment2.segNum*1024) + (cleanSegment2.currBlock);
						pair<int,int> fillPair(-1,SSB[k].second);
						cleanSegment2.segmentSummary[cleanSegment2.currBlock] = fillPair;
						memcpy(cleanSegment2.buffer+(cleanSegment2.currBlock*256),map->inodes+(SSB[k].second*256),1024);
						cleanSegment2.currBlock++;
					}
				}
			}
		}
		memcpy(cleanSegment2.buffer+(1016*256), cleanSegment2.segmentSummary, 1024*8);
		fileOut2();
		checkOut3();
		clean_segs[cleanSegment2.segNum] = (char)1;
		writeSegment(&cleanSegment2);
	}
}





void import(string filename, string lfs_filename, segment *s, imap *map){
	if(checkSize(filename) > 128*1024){
		cout<<"Sorry, that file is too big. Please try another file"<<endl;
		return;
	}
	inode node;
	memcpy(node.name,lfs_filename.c_str(),128);
	//node.name = lfs_filename.c_str();
	//find iNode number from mappings file
	int infile = open(filename.c_str(), O_RDONLY);
	int numBlocks = 0;
	int start = s->currBlock;
	int bytesRead;
	node.nodeNum = findFreeInode(map);
	char buffer[128];
	for(int i =0 ;i<128;i++){
		if(i<lfs_filename.size()){
			buffer[i] = lfs_filename[i];
		}else{
			buffer[i] = '0';
		}
	}
	for(int i = 0; i<128; i++){
		node.blocks[i] = 65540;
	}
	for(int i = 0; i<128;i++){
		file_map[(node.nodeNum*132)+i] = buffer[i];
	}
	clean_segs[s->segNum] = (char)1;
	memcpy(file_map+((node.nodeNum*132) + 128),&(node.nodeNum),4);
	while(1){
		bytesRead = read(infile,s->buffer + ((s->currBlock) * 256), 1024);//CHANGED
		node.blocks[numBlocks] = (unsigned int)((s->segNum*1024)+s->currBlock);
		//s->offset[s->currBlock] = numBlocks;
		pair<int,int> summary(node.nodeNum,numBlocks);
		s->segmentSummary[s->currBlock] = summary;
		numBlocks++;
		//s->inode[s->currBlock] = node.nodeNum;
		s->currBlock++;
		if(s->currBlock == 1016){
			memcpy(s->buffer+(s->currBlock*256), s->segmentSummary, 1024*8);//CHANGED
			//memcpy(s->buffer+(1019*256), s->offset, 4096);//CHANGED
			writeSegment(s);
			if(clean){
				cleanSeg(map);
				clean = false;
			}
			initializeSegment2(s);
			s->currBlock = 0;
			s->currByte =0;
			//s->segNum+=1;
		}
		//check if currBlock == 1024, and if so, write it out
		if(bytesRead <1024) break;
	}
	//close(infile);
	node.fileSize = ((numBlocks-1)*1024) + bytesRead;
	memcpy(s->buffer+(s->currBlock*256), &node,sizeof(node));//CHANGED
	pair<int,int> summary(-2,node.nodeNum);
	s->segmentSummary[s->currBlock] = summary;
	//s->inode[s->currBlock] = node.nodeNum; 
	//s->offset[s->currBlock] = numBlocks;
	numBlocks++;
	s->currBlock++;
	if(s->currBlock == 1016){
		memcpy(s->buffer+(s->currBlock*256), s->segmentSummary, 1024*8);//CHANGED
		//memcpy(s->buffer+(1019*256), s->offset, 4096);//CHANGED
		writeSegment(s);
		if(clean){
			cleanSeg(map);
			clean = false;
		}
		initializeSegment2(s);
		s->currBlock = 0;
		s->currByte =0;
	}
	map->inodes[node.nodeNum] = (unsigned int)((s->segNum*1024)+ (s->currBlock-1));
	memcpy(s->buffer+(s->currBlock*256),map->inodes + node.nodeNum/256, 1024);
	pair<int,int> summary2(-1,node.nodeNum/40);
	s->segmentSummary[s->currBlock] = summary2;
	numBlocks++;
	s->currBlock++;
	if(s->currBlock == 1016){
		memcpy(s->buffer+(s->currBlock*256), s->segmentSummary, 1024*8);
		writeSegment(s);
		if(clean){
			cleanSeg(map);
			clean = false;
		}
		initializeSegment2(s);
		s->currBlock = 0;
		s->currByte =0;
	}
	close(infile);
    	checkpoint_region[node.nodeNum/256] = (s->segNum*1024) + s->currBlock - 1;
}

void remove(string lfs_filename, imap *map, segment * s){
	char buffer[128];
	int namesize = lfs_filename.size();
	for(int i = 0; i<128; i++){
		if(i<namesize){
			buffer[i] = lfs_filename[i];
		}else{
			buffer[i] = '0';
		}
	}
	int filler[33];
	for(int i = 0; i<33;i++){
		filler[i] = 0;
	}
	int index = 0;
	bool found = false;
	for(int i = 0; i<10240;i++){
		int currIndex = i*132;
		for (int j = 0; j<128;j++){
			if(buffer[j] != file_map[currIndex+j]){
				break;
			}
			found = true;
		}
		if(found){
			for(int j= 0; j<132;j++){
				file_map[currIndex+j] = (char)0;
			}
			map->inodes[i] = 0;
			memcpy(s->buffer+(s->currBlock*256),map->inodes + (i/256), 1024);
			checkpoint_region[i/256] = (s->segNum*1024) + (s->currBlock );
			s->currBlock++;
			return;
		}
	}
}

void list(imap * map, segment *s){
	for(int i = 0; i<10240; i++){
		unsigned int inodeLoc = map->inodes[i];
		if(inodeLoc == 0) continue;
		int segNum = inodeLoc/1024;
		int offset = inodeLoc%1024;
		string filename = "./DRIVE/SEGMENT";
		inode * temp = (inode *)malloc(sizeof(inode));
		if(segNum == s->segNum){
			memcpy(temp,&s->buffer[(offset*256)],sizeof(inode));
		}else{
			filename = filename +to_string(segNum);
			FILE * infile;
			infile = fopen(filename.c_str(),"rb");
			fseek(infile,(offset)*1024,SEEK_SET);
			fread(temp,sizeof(inode),1,infile);
			fclose(infile);
		}

		cout<<"File "<<i<<" Name: "<< temp->name<<endl;
		cout<<"File " << i<<" Size: "<< temp->fileSize<<endl;

	}
}


void cat(string lfs_filename, imap *map, segment *s){
	char buffer[128];
	bool found = false;
	unsigned int blockNum;
	for(int i =0 ;i<128;i++){
        	if(i<lfs_filename.size()){
                	buffer[i] = lfs_filename[i];
                }else{
                        buffer[i] = '0';
                }
        }
	for(int i = 0; i<10240; i++){
		char name[128];
		memcpy(name, file_map+(i*132),128);
		blockNum = map->inodes[i];
		for(int j = 0; j<128;j++){
			if(buffer[j] != name[j]){
				break;
			}
			if(j == 127) found = true;
		}
		if(found){
			break;
		}
	}
	if(!found){
		cout<<"Sorry, that specified file was not found. Please try again"<<endl;
		return;
	}
        int segNum = blockNum/1024;
        int offset = blockNum%1024;
	string filename;
	inode * temp = (inode *)malloc(1024);
	if(s->segNum == segNum){
		memcpy(temp,&s->buffer[(offset*256)],sizeof(inode));
	}else{
        	filename = "./DRIVE/SEGMENT";
        	filename = filename +to_string(segNum);
        	FILE * infile;
        	infile = fopen(filename.c_str(),"rb");
        	fseek(infile,(offset)*1024,SEEK_SET);
        	fread(temp,sizeof(inode),1,infile);
        	fclose(infile);
	}
	int size = temp->fileSize;
	int i = 0;
	char file[((size/1024)+1)*1024];
	char file2[((size/1024)+1) *1024];
	while(size>0){
		unsigned int currBlock = temp->blocks[i];
		segNum = currBlock/1024;
		offset = currBlock%1024;
		if(segNum == s->segNum){
			memcpy(file+(1024*i),s->buffer + (offset*256),1024);
		}else{
			filename = "./DRIVE/SEGMENT";
			filename = filename + to_string(segNum);
			FILE * currentSeg;
			if(currentSeg == NULL){
				cout<<"Input segment file does not exist"<<endl;
				exit(1);
			}
			currentSeg = fopen(filename.c_str(),"rb");
			fseek(currentSeg,1024*offset,SEEK_SET);
				fread(file+(1024*i),1024,1,currentSeg);
			fclose(currentSeg);
		}
		i++;
		size-=1024;
	}
	cout<<file<<endl;

}

void display(string lfs_filename, int howmany, int start, imap *map, segment *s){
	char buffer[128];
	bool found = false;
	unsigned int blockNum;
	for(int i =0 ;i<128;i++){
        	if(i<lfs_filename.size()){
                	buffer[i] = lfs_filename[i];
                }else{
                        buffer[i] = '0';
                }
        }
	for(int i = 0; i<10240; i++){
		char name[128];
		memcpy(name, file_map+(i*132),128);
		blockNum = map->inodes[i];
		for(int j = 0; j<128;j++){
			if(buffer[j] != name[j]){
				break;
			}
			if(j == 127) found = true;
		}
		if(found){
			break;
		}
	}
	if(!found){
		cout<<"Sorry, that specified file was not found. Please try again"<<endl;
		return;
	}
        int segNum = blockNum/1024;
        int offset = blockNum%1024;
	inode * temp = (inode *)malloc(1024);
	string filename;
	if(segNum == s->segNum){
		memcpy(temp,&s->buffer[(offset*256)],sizeof(inode));
	}else{
		filename = "./DRIVE/SEGMENT";
        	filename = filename +to_string(segNum);
        	FILE * infile;
        	infile = fopen(filename.c_str(),"rb");
        	fseek(infile,(offset)*1024,SEEK_SET);
        	fread(temp,sizeof(inode),1,infile);
        	fclose(infile);
	}
	int size = temp->fileSize;
	int i = 0;
	char file[((size/1024)+1)*1024];
	while(size>0){
		unsigned int currBlock = temp->blocks[i];
		segNum = currBlock/1024;
		offset = currBlock%1024;
		if(segNum == s->segNum){
			memcpy(file+(1024*i),s->buffer + (offset*256),1024);
		}else{
			filename = "./DRIVE/SEGMENT";
			filename = filename + to_string(segNum);
			FILE * currentSeg;
			if(currentSeg == NULL){
				cout<<"Input segment file does not exist"<<endl;
				exit(1);
			}
			currentSeg = fopen(filename.c_str(),"rb");
			fseek(currentSeg,1024*offset,SEEK_SET);
			fread(file+(1024*i),1024,1,currentSeg);
			fclose(currentSeg);
		}
		i++;
		size-=1024;
	}
	if(start+howmany > temp->fileSize){
		char outBuff2[(temp->fileSize)-start];
		memcpy(outBuff2,file+start,(temp->fileSize - start));
		cout<<outBuff2<<endl;
		return;
	}else{
		char outBuff[howmany+1];
		outBuff[howmany] = '\0';
		memcpy(outBuff,file+start,howmany);
		cout<<outBuff<<endl;
		return;
	}
}

void overwrite(string lfs_filename, int howmany, int start, char c, imap *map, segment *s){
	char buffer[128];
	bool found = false;
	unsigned int blockNum;
	for(int i =0 ;i<128;i++){
        	if(i<lfs_filename.size()){
                	buffer[i] = lfs_filename[i];
                }else{
                        buffer[i] = '0';
                }
        }
	for(int i = 0; i<10240; i++){
		char name[128];
		memcpy(name, file_map+(i*132),128);
		blockNum = map->inodes[i];
		for(int j = 0; j<128;j++){
			if(buffer[j] != name[j]){
				break;
			}
			if(j == 127) found = true;
		}
		if(found){
			break;
		}
	}
	if(!found){
		cout<<"Sorry, that specified file was not found. Please try again"<<endl;
		return;
	}
	inode * temp = (inode *)malloc(1024);
        int segNum = blockNum/1024;
        int offset = blockNum%1024;
	string filename;
	if(segNum == s->segNum){
		memcpy(temp,&s->buffer[(offset*256)],sizeof(inode));
	}else{
		filename = "./DRIVE/SEGMENT";
        	filename = filename +to_string(segNum);
        	FILE * infile;
        	infile = fopen(filename.c_str(),"rb");
        	fseek(infile,(offset)*1024,SEEK_SET);
        	fread(temp,sizeof(inode),1,infile);
        	fclose(infile);
	}
	int size = temp->fileSize;
	int i = 0;
	int tempSize = temp->fileSize + max(0,(start+howmany) - temp->fileSize);
	char file2[((tempSize/1024)+1)*1024];
	while(size>0){
		unsigned int currBlock = temp->blocks[i];
		segNum = currBlock/1024;
		offset = currBlock%1024;
		if(segNum == s->segNum){
			memcpy(file2+(1024*i),s->buffer + (offset*256),1024);
		}else{
			filename = "./DRIVE/SEGMENT";
			filename = filename + to_string(segNum);
			FILE * currentSeg;
			if(currentSeg == NULL){
				cout<<"Input segment file does not exist"<<endl;
				exit(1);
			}
			currentSeg = fopen(filename.c_str(),"rb");
			fseek(currentSeg,1024*offset,SEEK_SET);
			fread(file2+(1024*i),1024,1,currentSeg);
			fclose(currentSeg);
		}
		i++;
		size-=1024;
	}
	for(int j = 0; j<howmany; j++){
		file2[j+start] = c;
	}
	ofstream outer("temp.txt");
	outer << file2;
	outer.close();
	remove(lfs_filename,map,s);
	import("temp.txt",lfs_filename,s,map);
	cout<<file2<<endl;
}




int main(int argc, char * argv[]){
	checkIn3();
	fileIn2();
	imap map;
	init3(&map, checkpoint_region);
	segment s;
	initializeSegment2(&s);
	bool clean = false;
	while(true){
		int index = 0;
		for(int i = 0; i<64;i++){
			if((int)clean_segs[index] != 0){
				index++;
			}
			if(index == 2){
				clean = true;
				//clean();
				break;
			}
		}

	


		string currIn;
		vector<string> inputs;
		getline(cin,currIn);
		istringstream iss(currIn);
		copy(istream_iterator<string>(iss),istream_iterator<string>(),back_inserter(inputs));
		if(inputs.at(0).compare("import")==0){
			import(inputs.at(1),inputs.at(2),&s,&map);
		}else if(inputs.at(0).compare("remove")==0){
			remove(inputs.at(1),&map, &s);
		}else if(inputs.at(0).compare("list") ==0){
			list(&map, &s);
		}else if(inputs.at(0).compare("cat") == 0){
			cat(inputs.at(1),&map, &s);
		}else if(inputs.at(0).compare("display") == 0){
			display(inputs.at(1),stoi(inputs.at(2)), stoi(inputs.at(3)), &map, &s);
		}else if(inputs.at(0).compare("overwrite")== 0){
			overwrite(inputs.at(1),stoi(inputs.at(2)), stoi(inputs.at(3)), inputs.at(4)[0], &map, &s);
		}else if(inputs.at(0).compare("Shutdown")==0){
			checkOut3();
			fileOut2();
			memcpy(s.buffer+(1016*256), s.segmentSummary, 1024*8);
			writeSegment(&s);
			if(clean) cleanSeg(&map);
			exit(0);
		}else{
			cout<<"Sorry, that is not a recognized command. Please try again."<<endl;
		}
	}

}

