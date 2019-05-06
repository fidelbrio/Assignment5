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
	unsigned int index = 0;
	while(true){
		if(map->inodes[index] == 0){
			cout<<"INDEX: "<<index<<endl;
			return index;
		}
		//cout<< map->inodes[index];
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
	//checkIn();//these should be done on startup
	fileIn();
	inode node;
	memcpy(node.name,lfs_filename.c_str(),128);
	//node.name = lfs_filename.c_str();
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
	cout<<"INITIAL NODENUM: "<<node.nodeNum<<endl;
	cout<<"INITIAL NODENAME: "<<node.name<<endl;
	char buffer[128];
	for(int i =0 ;i<128;i++){
		if(i<lfs_filename.size()){
			buffer[i] = lfs_filename[i];
		}else{
			buffer[i] = '0';
		}
	}
	//memcpy(buffer,lfs_filename.c_str(),128);
	//cout<<sizeof(buffer)<<endl;
	//cout<<buffer<<endl;
	memcpy(file_map+(node.nodeNum*132),buffer,128);
	//for(int i =0; i<128;i++) cout<<file_map[(node.nodeNum*132)+i];
	//cout<<endl;
	memcpy(file_map+((node.nodeNum*132) + 4),&(node.nodeNum),4);
	for(int i = 0; i<4;i++){
		//cout<<file_map[(node.nodeNum*132)+i+128];
	}
	//cout<<endl;
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
	node.fileSize = ((numBlocks-1)*1024) + bytesRead;
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
	map->inodes[node.nodeNum] = (unsigned int)((s->segNum*1024)+ (s->currBlock-1));
	cout<<"node.nodeNum: "<<node.nodeNum<<endl;
	cout<<"IMAP ADDRESS: " << map->inodes[node.nodeNum] <<endl;
	cout<<"CURR BLOCK: " << s->currBlock<<endl;
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
	cout<<"IMAP ADDRESS: "<< map->inodes[node.nodeNum]<<endl;

	close(infile);
	//unsigned int imapLocation = (s->segNum *1024) + s->currBlock;
        checkpoint_region[node.nodeNum/256] = map->inodes[node.nodeNum];
	//cout<<checkpoint_region[node.nodeNum/256]<<endl;
	//checkOut();
	//checkIn();
	//cout<<checkpoint_region[0]<<endl;
        //map->inodes[node.nodeNum] = (unsigned int)((s->segNum*1024)+ s->currBlock);
	s->inode[s->currBlock] = node.nodeNum;
	s->offset[s->currBlock] = numBlocks;
	//s->currBlock++;
	//numBlocks++;
	/*if(s->currBlock == 1015){
		memcpy(s->buffer+(s->currBlock*1024), s->inode, 4096);
		memcpy(s->buffer+(1019*1024), s->offset, 4096);
		writeSegment(s);
		s->currBlock = 0;
		s->segNum+=1;
	}*/

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
	char *buffer = new char[128];
	int namesize = lfs_filename.size();
	char *filename = new char[namesize];
	cout << lfs_filename << endl;
//	char g = '0';
	for(int i = 0; i < namesize; i++){
//		if(i < sizeof(lfs_filename)){
		filename[i] = lfs_filename[i];
//		}else{
//			n = sprintf(*filename[i], '0');
//			filename[i] = g;
//		}
	}
	int position;
	int index = 0;
	ifstream file ("DRIVE/FILE_MAP");
	bool found = false;
	while(!found){
		position = index*132;
		file.seekg(position);
		file.read(buffer, 128);
		found = true;
		for(int i = 0; i < namesize; i++){
//			if(buffer[i] == '0'){ // This if makes it quit
//				break;
//			}
			cout << buffer[i] << '+' << filename[i] << endl;
			if(buffer[i] != filename[i]){
				found = false;
			}
		}
		if(buffer[namesize] != '0'){
			found = false;
		}
		index++;
		if(index == 10240){
			break;
		}
	}
	index--;
	if(!found){
		cout << "Could not find the specified file" << endl;
	}
	if(found){
		cout << index << endl;
		map->inodes[index] = 0;
	}
	//Must erase the file name in the file map

	char *entireFile = new char[1351680];
	file.seekg(0);
	file.read(entireFile, 1351680);
//	index -= 1;
	for(int i = 0; i < 128; i++){
		entireFile[position+i] = '0';
	}
	file.close();
	ofstream file2;
	file2.open("DRIVE/FILE_MAP");
	file2 << entireFile;
	file2.close();

}


void list(imap * map){ //should only need inodes
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
		//unsigned int imapLoc =i/256;
		unsigned int inodeLoc = map->inodes[i];
		//if(inodeLoc == 0) continue;
		cout<<"INODE LOCATION (SHOULD BE 3): "<<inodeLoc<<endl;
		//inodeLoc = 3;
		//cout<<"inodeNum "<<inodeNum<<endl;
		//cout<<"map.inodes[i] " <<map.inodes[i] <<endl;
		int segNum = inodeLoc/1024;
		cout<<"segnum " <<segNum<<endl;
		int offset = inodeLoc%1024;
		cout<<"OFFSET: "<<offset<<endl;
		string filename = "./DRIVE/SEGMENT";
		//filename[15] = segNum;
		filename = filename +to_string(segNum);
		cout<<filename<<endl;
		//ifstream infile (filename);
		//infile.open(filename, ios::binary|ios::in);
		inode * temp = (inode *)malloc(1024)/* = (inode *)malloc(1024 sizeof(inode))*/;
		//cout << "after malloc" << endl;
		FILE * infile;
		infile = fopen(filename.c_str(),"rb");
		//infile+=1024*inodeLoc;
		//cout << "after fileopen" << endl;
		fseek(infile,(inodeLoc)*1024,SEEK_SET);
		//infile.seekg(offset*1024);
		//cout << "after fseek" << endl;
		fread(temp,sizeof(inode),1,infile);
		//char *inodeBlock = new char[sizeof(inode)];
		//infile.read(inodeBlock, sizeof(inode));

		//cout << inodeBlock << endl;


		//memcpy(&temp, inodeBlock, sizeof(inode)/* INODE SIZE GOES HERE */);


		cout<<"temp.name: "<< temp->name<<endl;
		cout<<"temp.size: "<< temp->fileSize<<endl;

		//if(i==10) break;
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
	exit(0);
	checkIn();
	initializeSegment(&s);
        import2(argv[1],argv[2],&s,&map);
	//writeSegment(&s);
	//list(&map);
	import2(argv[3],argv[4],&s,&map);
	list(&map);
	writeSegment(&s);
	s.currBlock= 0;
	s.segNum++;
	list(&map);
	//list();

}

