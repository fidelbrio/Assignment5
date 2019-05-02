#include "segment.h"

using namespace std;

void initializeSegment(segment *s){
} 
void nextByte(segment *s){ // access next byte in segment
}
void nextBlock(segment *s){ // access next block in segment
}
void writeSegment(segment *s){ //writes buffer to segment
	string segmentN = "DRIVE/SEGMENT";
	string N = to_string(s->segNum);
	segmentN += N;
	myfile.open(segmentN);
	for(i = 0; i < 1048576; i++){
		myfile << s->buffer[i];
	}
	myfile.close();
	return 0;
}
