#include "imap.h"
#include <iostream>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

using namespace std;

void update(imap *map, int inode_num, unsigned int block_num){
	map->inodes[inode_num] = block_num;
}

unsigned int getInode(imap *map, int inode_num){
	return map->inodes[inode_num];
}

void init2(imap *map,unsigned int * cr){
	for(int i = 0; i<40; i++){
		if(cr[i] == 0){
			for(int j = 0; j<256;j++){
				map->inodes[(i*256)+j] = 0;
			}
			continue;
		}

		int segment = cr[i]/1024;
                int offset = cr[i]%1024;
                string name = "DRIVE/SEGMENT";
                name+=to_string(segment);
                //cout<<"NAME OF SEGMENT CONTAING IMAP #"<<i<<": "<<name<<endl;
		//cout<<"LOCATED AT BLOCK #"<<offset<<endl;
                FILE *fp = fopen(name.c_str(),"rb");
                fseek(fp,offset*1024, SEEK_SET);
                fread(&map->inodes[i*256],1024,1,fp);
                /*for(int j = 0; j<1024; j++){
                        cout<< map->inodes[(i*256)+j];
                }
                cout<<"BLOCK"<<endl;*/
                /*unsigned int temp;
                for(int j = 0; j<256; j++){
                        fread(&temp,4,1,fp);
                        map->inodes[(i*256)+j] = temp;
                }*/
                for(int j = 0; j<1024; j++){
                        cout<< map->inodes[(i*256)+j];
                }
                //cout<<"BLOCK"<<endl;

                fclose(fp);
	}
}

void init3(imap * map, unsigned int * cr){
	for(int i = 0; i<40; i++){
                if(cr[i] == 0){
                        for(int j = 0; j<256;j++){
                                map->inodes[(i*256)+j] = 0;
                        }
                        continue;
                }

                int segment = cr[i]/1024;
                int offset = cr[i]%1024;
                string name = "DRIVE/SEGMENT";
                name+=to_string(segment);
                //cout<<"NAME OF SEGMENT CONTAING IMAP #"<<i<<": "<<name<<endl;
                //cout<<"LOCATED AT BLOCK #"<<offset<<endl;
                ifstream file_map(name,ios::in|ios::binary);
		file_map.seekg((offset)*1024,file_map.beg);
		file_map.read((char *)&(map->inodes[i*256]),1024);
		file_map.close();
                for(int j = 0; j<1024; j++){
                        cout<< map->inodes[(i*256)+j];
                }
                //cout<<"BLOCK"<<endl;
        }
}



void init(imap *map){
	int tempBuff[256];
	for(int i = 0; i<256; i++){
		tempBuff[i] = 0;
	}
	//cout<<"TEMP BUFF: "<<tempBuff<<endl;
	int ar[40];
	//int infile = open("DRIVE/CHECKPOINT_REGION",O_RDWR);
	ifstream infile;//("DRIVE/CHECKPOINT_REGION",ifstream::binary);
	infile.open("DRIVE/CHECKPOINT_REGION",ios::binary|ios::in);
	unsigned int buff;
	for(int i = 0; i < 40; i++){
		infile >> ar[i];
		//cout<<"ar[i] = "<<ar[i]<<endl;
		if(ar[i] == 0){
			//memcpy(&map->inodes[i*256],tempBuff,1024);
			for(int j = 0; j<256;j++){
				map->inodes[(i*256)+j] = tempBuff[j];
				//cout<< map->inodes[(i*256) +j];
			}
			//cout<<"BLOCK SHORTCUT"<<endl;
			continue;
		}
		//infile.read(reinterpret_cast<char *>(&buff),4);
		//read(infile,&buff,4);
		//cout<<ar[i]<<endl;
		int segment = ar[i]/1024;
		int offset = ar[i]%1024;
		string name = "DRIVE/SEGMENT";
		name+=to_string(segment);
		cout<<name<<endl;
		FILE *fp = fopen(name.c_str(),"rb");
		fseek(fp,offset*1024, SEEK_SET);
		fread(&map->inodes[i*256],1024,1,fp);
		/*for(int j = 0; j<1024; j++){
			cout<< map->inodes[(i*256)+j];
		}
		cout<<"BLOCK"<<endl;*/
		/*unsigned int temp;
		for(int j = 0; j<256; j++){
			fread(&temp,4,1,fp);
			map->inodes[(i*256)+j] = temp;
		}*/
		for(int j = 0; j<1024; j++){
                	cout<< map->inodes[(i*256)+j];
                }
                cout<<"BLOCK"<<endl;

		fclose(fp);
	}
	//exit(0);
	//memcpy(
	infile.close();
}
