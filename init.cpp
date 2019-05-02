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
		ftruncate(fileno(fp),1024*1024);
		fclose(fp);
	}
	return 0;
}
