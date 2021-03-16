#ifndef _RAM_H
#define _RAM_H
extern char *ram[40];
void unloadFrame(int frameNumber);
void initializeRAM();
void copyLineFromRam(int line, char *place);
void printFrame(int num);
#endif