#include "segment.h"
#include <string>
#include <fstream>
#include <iostream>
#include <bitset>
#include <utility>
using namespace std;

void initializeSegment(segment *s){
	for(int i = 0; i<1024*1024;i++){
		s->buffer[i] = '0';
	}
	char *segMap = new char[64];
	ifstream file ("DRIVE/CHECKPOINT_REGION");
	bool found = false;
	int position = 160;
	file.seekg(position);
	file.read(segMap, 64);
	int index = 0;
	while(!found){
		if(index == 64){
			cout << "All segments are full, there is not enough memory left" << endl;
		}
		if(segMap[index] != 0){
			found = true;
			index--;
		}
		index++;
	}
	s->segNum = index;
	file.close();
}

void initializeSegment2(segment *s){
	for(int i = 0; i< 1024*256;i++){
		s->buffer[i] = 0;
	}
	pair<int,int> filler(-3,-3);
	for(int i = 0; i<8*1024;i++){
		s->segmentSummary[i] = filler;
	}
	char *segMap = new char[64];
	ifstream file("./DRIVE/CR2",ios::in|ios::binary);
	bool found = false;
	//int position = 160;
	//file.seekg(position,file.beg);
	file.read(segMap,64);
	int index = 0;
	while(!found){
		if(index == 64){
			cout<<"All segment are full. There is not enough memory left" <<endl;
			exit(1);
		}
		cout<<"segMap bitset: "<< bitset<8>(segMap[index])<<endl;
		cout<<"(int)segMap[index]: "<<(int)segMap[index]<<endl;
		cout<<"segMap[index]: "<<segMap[index]<<endl;
		if((int)segMap[index] == 0){
			found = true;
			index--;
		}
		index++;
	}
	s->segNum = index;
	file.close();
}

void nextByte(segment *s){ // access next byte in segment
}
void nextBlock(segment *s){ // access next block in segment
}
void writeSegment(segment *s){ //writes buffer to segment
	string segmentN = "DRIVE/SEGMENT";
	string N = to_string(s->segNum);
	segmentN += N;
	//cout<<segmentN<<endl;
	/*ofstream myfile;
	myfile.open(segmentN);
	for(int i = 0; i < 1048576; i++){
		//s->buffer[i] << myfile;
		if(s->buffer[i] != '0'){
			//cout<< s->buffer<<endl;
		}
		myfile << s->buffer[i];
	}*/
	ofstream seg(segmentN,ios::out|ios::binary);
        seg.write((char *)(s->buffer),1024*1024);
	//myfile.close();
	//return 0;
}
