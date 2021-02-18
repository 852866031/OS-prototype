#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"ram.h"
#include"shell.h"
#include"kernel.h"
#include"interpreter.h"
#include"pcb.h"

char *ram[1000];
int firstLinefree;
int loadedNum;

void initializeRAM(){
    if(detail()) printf("-> Initializing RAM with lines: 1000\n"); 
    for(int i=0; i<1000; i++) ram[i]=NULL;
    firstLinefree=0;
    loadedNum=0;
    if(detail()) printf("-> RAM initialized");
}

int fileToRam(FILE *p){
    if(detail()) printf("Calling function fileToRam in ram.c\n");
    char **tokens;
    char buffer[100];
    while(!feof(p)){
        if(firstLinefree==1000) {
            printf("ERROR: Not enough RAM to add program\n");
            return -1;
        }
        if(fgets(buffer, 100, p)==NULL) break;
        if(buffer[0]=='\n') continue;
        ram[firstLinefree]=strdup(buffer);
        tokens=tokenize(buffer);
        firstLinefree++;
        if(strcmp(tokens[0], "run")==0 && tokens[1]!=NULL && tokens[2]==NULL) {
            //we will load the file of run command into ram
            FILE *f=fopen(tokens[1], "r");
            fileToRam(f);  
            fclose(f);
        }
    }
    return 0;
}


void addToRam(FILE *p, int *start, int *end){
    if(detail()) printf("Calling function addToRam in ram.c\n");
    if(detail()) printf("-> Loading file to ram from line %d\n", firstLinefree);
    *start=firstLinefree;
    if(loadedNum>3) {
        *end=-1;
        printf("ERROR: too many programs loaded to ram\n");
        return;
    }
    int status=fileToRam(p);//this helper is for run command so we can do recursion
    *end=firstLinefree-1;;
    loadedNum++;
    if(status<0) *end=-1;
    if(detail()) printf("-> File loaded from line %d to line %d\n", *start, *end);
    if(detail()) printf("-> %d programs loaded in ram\n", loadedNum);
}

void unloadFromRam(int start, int end){
    if(detail()) printf("Calling function unloadFromRam in ram.c\n");
    if(detail()) printf("-> Unloading line %d to line %d from ram\n", start, end);
    for(int i=start;i<=end; i++){
        ram[i]=NULL;
    }
    loadedNum--;
    if(detail()) printf("-> Unload successfully\n");
    for(int i=0; i<1000;i++){
        if(ram[i]!=NULL) return;
    }
    firstLinefree=0;
}
void copyLineFromRam(int line, char *place){
    strcpy(place, ram[line]);
}