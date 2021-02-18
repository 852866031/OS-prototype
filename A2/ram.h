#ifndef _RAM_H
#define _RAM_H
void addToRam(FILE *p, int *start, int *end);
void unloadFromRam(int start, int end);
void copyLineFromRam(int line, char *place);
void initializeRAM();
#endif