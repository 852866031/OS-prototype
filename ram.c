#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"shell.h"
#include"kernel.h"
#include"interpreter.h"
#include"pcb.h"
#include"ram.h"

char *ram[40];

void initializeRAM(){
    if(detail()) printf("-> Initializing RAM with 40 pointers\n"); 
    for(int i=0; i<40; i++) ram[i]=NULL;
    if(detail()) printf("-> RAM initialized");
}

void unloadFrame(int frameNumber){
    if(detail()) printf("Calling function unloadFromRam in ram.c\n");
    if(detail()) printf("-> Unloading frame %d from ram\n", frameNumber);
    for(int i=frameNumber*4; i<frameNumber*4+4; i++){
        ram[i]=NULL;
    }
    if(detail()) printf("-> Unload successfully\n");
}

void printFrame(int num){
    printf("Content in Frame %d: \n", num);
    for(int i=0; i<4; i++){
        printf("offset %d: ", i);
        if(ram[num*4+i]==NULL) printf("NULL\n");
        else printf("%s\n", ram[num*4+i]);
    }
}