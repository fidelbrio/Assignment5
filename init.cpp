#include <iostream>
//#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string>

using namespace std;

int main(){
	int ret = system("mkdir DRIVE");
	if(ret == -1){
		cout<<"Error"<<endl;
	}else{
		cout<<"Done"<<endl;
	}
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
	}
	int buff =0;
	string name = "./DRIVE/CHECKPOINT_REGION";
	FILE *fp = fopen(name.c_str(),"w");
	for(int i = 0; i<224;i++){
		fprintf(fp,"%c",'0');
	}

	name = "./DRIVE/FILE_MAP";
	fp = fopen(name.c_str(),"w");
	for(int i = 0; i<1351680;i++){
		fprintf(fp,"%c",'0');
	}
	return 0;
}
