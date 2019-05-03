#include "segment.h"
#include <string>
#include <fstream>
#include <iostream>
using namespace std;

void initializeSegment(segment *s){
	for(int i = 0; i<1024*1024;i++){
		s->buffer[i] = '0';
	}
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
	ofstream myfile;
	myfile.open(segmentN);
	for(int i = 0; i < 1048576; i++){
		//s->buffer[i] << myfile;
		if(s->buffer[i] != '0'){
			//cout<< s->buffer<<endl;
		}
		myfile << s->buffer[i];
	}
	myfile.close();
	//return 0;
}
