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

unsigned int checkpoint_region[40];
char file_map[337920*4];

unsigned int findFreeInode(imap *map){
	int index = 0;
	while(true){
		if(map->inodes[index] == 0){
			return index;
		}
		index++;
	}
}

void checkIn(){
	fstream cr("./DRIVE/CHECKPOINT_REGION",ios::binary|ios::out|ios::in);
	char buff[160];
	cr.read(buff,160);
	memcpy(checkpoint_region,buff,160);
	cr.close();
}

void checkOut(){
	fstream cr("./DRIVE/CHECKPOINT_REGION",fstream::binary|ios::out);
	char buff[160];
	//cout<<checkpoint_region[0]<<endl;
	memcpy(buff,checkpoint_region,160);
	//for(int i = 0; i<160;i++){
	//	cout<<buff[i];
	//}
	//cout<<endl;
	//cout<<buff[0]<<buff[1]<<buff[2]<<buff[3]<<endl;
	cr.write(buff,160);
	cr.close();
}

void fileIn(){
	fstream map("./DRIVE/FILE_MAP",ios::binary|ios::out|ios::in);
	char buff[1351680];
	map.read(buff,1351680);
	memcpy(file_map,buff,1351680);
	map.close();
}

void fileOut(){
	fstream map("./DRIVE/FILE_MAP",fstream::binary|ios::out);
	char buff[1351680];
	memcpy(buff,file_map,1351680);
	map.write(buff,1351680);
	map.close();
}

void import2(string filename, string lfs_filename, segment *s, imap *map){
	checkIn();//these should be done on startup
	fileIn();
	inode node;
	node.name = lfs_filename;
	//find iNode number from mappings file
	int infile = open(filename.c_str(), O_RDONLY);
	int numBlocks = 0;
	int start = s->currBlock;
	int bytesRead;
	node.nodeNum = findFreeInode(map);
	//cout<<inodeNum<<endl;
	/*FILE* mapper;
	mapper = fopen("./DRIVE/FILE_MAP","r+b");
	fseek(mapper,132*node.nodeNum,SEEK_SET);
	fputs(node.name.c_str(),mapper);
	fseek(mapper, (132*node.nodeNum)+128, SEEK_SET);
	fputs(to_string(node.nodeNum).c_str(),mapper);*/
	cout<<node.nodeNum<<endl;
	cout<<node.name<<endl;
	char buffer[128];
	for(int i =0 ;i<128;i++){
		if(i<lfs_filename.size()){
			buffer[i] = lfs_filename[i];
		}else{
			buffer[i] = '0';
		}
	}
	//memcpy(buffer,lfs_filename.c_str(),128);
	cout<<sizeof(buffer)<<endl;
	cout<<buffer<<endl;
	memcpy(file_map+(node.nodeNum*132),buffer,128);
	for(int i =0; i<128;i++) cout<<file_map[(node.nodeNum*132)+i];
	cout<<endl;
	memcpy(file_map+((node.nodeNum*132) + 4),&(node.nodeNum),4);
	for(int i = 0; i<4;i++){
		cout<<file_map[(node.nodeNum*132)+i+128];
	}
	cout<<endl;
	//fileOut();
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
	//checkpoint_region[node.nodeNum/256] 

	//map->inodes[node.nodeNum] = (unsigned int)((s->segNum*1024)+ s->currBlock);
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
        checkpoint_region[node.nodeNum/256] = imapLocation;
	//cout<<checkpoint_region[node.nodeNum/256]<<endl;
	checkOut();
	checkIn();
	//cout<<checkpoint_region[0]<<endl;
        map->inodes[node.nodeNum] = (unsigned int)((s->segNum*1024)+ s->currBlock);
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

	//fstream check("./DRIVE/CHECKPOINT_REGION",fstream::in|fstream::out|fstream::binary);
	//if(!check) cout<<"ERROR"<<endl;
	//check.seekg((node.nodeNum/40)*4);
	//check.write(to_string(imapLocation).c_str(),4);
	//check<<"aouenfaoefinefoin";
	//check <<imapLocation;
	//int check = open("./DRIVE/CHECKPOINT_REGION",O_RDWR);
	//lseek(check,(node.nodeNum/40)*4,SEEK_SET);
	//cout<<node.nodeNum/10<<endl;
	//fstream outfile;
	//outfile.open("./DRIVE/CHECKPOINT_REGION");//,ios_base::binary);
	//cout<<"imapLocation: "<<imapLocation<<endl;
	//write(check,&imapLocation,sizeof(unsigned int));
	//outfile.seekp(imapLocation*4,ios_base::beg);
	//cout<<(char *)&imapLocation<<endl;
	//outfile.write((char *)&imapLocation,4);
	//outfile.write(to_string(imapLocation).c_str(),4);
	//close(check);
	//exit(0);
	//outfile.close();
	writeSegment(s);
}
void remove(string lfs_filename){ //should only need inodes
	
}

void list(imap map){ //should only need inodes
	ifstream mapper;
	ifstream cr;
	fileIn();
 	mapper.open("./DRIVE/FILE_MAP",ios::in|ios::binary);
	for(int i = 0; i<10240; i++){
		//mapper.seekg(132*i,mapper.beg);
		/*
		char name[128];
		char addr[4];
		unsigned int seg;
		memcpy(name,file_map+(132*i),128);
		cout<<"memcpy name: " << name<<endl;
		memcpy(addr,file_map+((132*i)+128)],4);
		seg = atoi(addr);
		cout<<seg<<endl;
		cout<<"memcpy addr: "<<addr<<endl;
		for(int j = 0; j<128;j++){
			mapper >> name[j];
		}
		mapper >> addr;
		cout<<name<<endl;
		cout<<addr<<endl;
		cr.open("./DRIVE/CHECKPOINT_REGION",ios::in|ios::binary);
		//cr.seekg(4*addr,cr.beg);
		cr>>seg;
		*/
		//do all this stuff only if the name in the file map is empty
		unsigned int inodeLoc = map.inodes[i];
		//cout<<"inodeNum "<<inodeNum<<endl;
		//cout<<"map.inodes[i] " <<map.inodes[i] <<endl;
		int segNum = inodeLoc/1024;
		cout<<"segnum " <<segNum<<endl;
		int offset = inodeLoc%1024;
		string filename = "./DRIVE/SEGMENT";
		//filename[15] = segNum;
		filename = filename +to_string(segNum);
		cout<<filename<<endl;
		ifstream infile (filename);
		//infile.open(filename, ios::binary|ios::in);
		inode temp/* = (inode *)malloc(1024 sizeof(inode))*/;
		cout << "after malloc" << endl;
//		FILE * infile;
//		infile = fopen(filename.c_str(),"rb");
		//infile+=1024*inodeLoc;
		cout << "after fileopen" << endl;
//		fseek(infile,inodeLoc*1024,SEEK_SET);
		infile.seekg(offset*1024);
		cout << "after fseek" << endl;
//		fread(&temp,sizeof(struct _inode),1,infile);
		char *inodeBlock = new char[sizeof(inode)];
		infile.read(inodeBlock, sizeof(inode));

		cout << inodeBlock << endl;


		memcpy(&temp, inodeBlock, sizeof(inode)/* INODE SIZE GOES HERE */);


		cout<< temp.name<<endl;

		if(i == 10) break;
	}
	exit(0);
}

void shutdown(segment *s){
	writeSegment(s);
	exit(0);
}


int main(int argc, char * argv[]){
        segment s;
        imap map;
        init(&map);
	checkIn();
	initializeSegment(&s);
        import2(argv[1],argv[2],&s,&map);
	list(map);
	import2(argv[3],argv[4],&s,&map);
	writeSegment(&s);
	s.currBlock= 0;
	s.segNum++;
	list(map);
	//list();

}

