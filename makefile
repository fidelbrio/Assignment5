all: LFS

LFS: LFS.cpp inode.cpp inode.h segment.cpp segment.h imap.cpp imap.h
	g++ inode.cpp imap.cpp segment.cpp LFS.cpp -g -o main
