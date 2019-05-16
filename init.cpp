#include <iostream>
//#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <fstream>

using namespace std;

int main(){
	int ret = system("mkdir DRIVE");
	if(ret == -1){
		cout<<"Error"<<endl;
	}else{
		cout<<"Done"<<endl;
	}
	unsigned int ogBuff[256*1024];
	for(int i = 0; i<256*1024;i++){
		ogBuff[i]  = 0;
	}
	for(int i = 0; i<64;i++){
		string name = "./DRIVE/SEGMENT";
		name+=to_string(i);
		ofstream seg(name,ios::out|ios::binary);
	        seg.write((char *)&ogBuff,1024*1024);
        	seg.close();
	}
	/*
	for(int i = 0; i<64;i++){
		string name = "./DRIVE/SEGMENT";
		name = name + to_string(i);
		//cout<<(char)i<<" "<<i<<endl;
		//name[15] = (char)i;
		cout<<name<<endl;
		FILE *fp = fopen(name.c_str(),"w");
		for(int j = 0; j <1024*1024;j++){
			fprintf(fp,"%c",'0');
		}
		//ftruncate(fileno(fp),1024*1024);
		fclose(fp);
	}*/
	/*
	int buff =0;
	string name = "./DRIVE/CHECKPOINT_REGION";
	FILE *fp = fopen(name.c_str(),"w");
	for(int i = 0; i<224;i++){
		fprintf(fp,"%c",'0');
	}*/

	unsigned int buff[40];
	for(int i= 0; i<40;i++){
		buff[i] = 0;
	}
	ofstream cr("./DRIVE/CHECKPOINT_REGION",ios::out|ios::binary);
	cr.write((char *)&buff,160);
	cr.close();

	unsigned int buff2[337920];
	for(int i = 0; i<337920;i++){
		buff2[i] = 0;
	}
	ofstream map("./DRIVE/FILE_MAP",ios::out|ios::binary);
	map.write((char *)&buff2,1351680);
	map.close();
	/*
	string name = "./DRIVE/FILE_MAP";
	FILE * fp = fopen(name.c_str(),"w");
	for(int i = 0; i<1351680;i++){
		fprintf(fp,"%c",'0');
	}*/
	unsigned int newBuff[16];
	for(int i = 0; i<16;i++){
		newBuff[i] = 0;
	}
	ofstream cr2("./DRIVE/CR2",ios::out|ios::binary);
	cr2.write((char *)&newBuff,64);
	cr.close();
	return 0;
}
