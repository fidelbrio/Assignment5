#ifndef SEGMENT_H
#define SEGMENT_H

include <stdio.h>

typedef struct _segment{
	int inode[1024]; // inode[i] holds the inode# of block i (SUMMARY BLOK)
	int offset[1024]; // offset[i] holds block i's offset# of the file it belongs to (SUMMARY BLOCK)
	char buffer[1048576]; // all the bytes of the segment
	int validBit[1024]; // is block i empty?
	int currBlock = 0; // the current block being written to
	int currByte = 0; // the current bit being written to
	int segNum = 0;
} segment;

void initializeSegment(segment *s); 
void nextByte(segment *s); // access next byte in segment
void nextBlock(segment *s); // access next block in segment
void writeSegment(segment *s); //writes buffer to segment

#endif;