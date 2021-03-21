#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"pcb.h"
#include"ram.h"
#include"kernel.h"
#include"memorymanager.h"
#include"interpreter.h"

char *ram[40];
int FILENAME=0;
int countTotalPages(FILE *f){
    rewind(f);
    int num=0;
    char c=fgetc(f);
    while(c!=EOF){
        if(c=='\n') num++;
        c=fgetc(f);
    }
    num++;//EOF also presents a line;
    rewind(f);
    if(detail()) printf("-> %d lines found\n", num);
    int total;
    if(num%4>0) total=((int) num/4)+1;
    else total=(int) num/4;
    if(detail()) printf("-> The file has %d pages\n", total);
    return total;
}

void loadPage(int pageNumber, FILE *f, int frameNumber){
    if(detail()) printf("-> loading file from pageNumber %d into frame %d\n", pageNumber, frameNumber);
    rewind(f);
    char buffer[100];
    for(int i=0; i<pageNumber*4; i++){
        if(fgets(buffer, 100, f)==NULL) {
            printf("-> Error: pageNumber out of range\n");
            return;
        }
    }
    for(int i=0; i<4; i++){
        if(fgets(buffer, 100, f)!=NULL) ram[frameNumber*4+i]=strdup(buffer);
        else ram[frameNumber*4+i]=NULL;
    }
    if(detail()) printf("-> loaded successfully\n");
    //if(detail()) printFrame(frameNumber);
}

int findFrame(){
    for(int i=0; i<40; i+=4){
        if(ram[i]==NULL) return i/4;
    }
    return -1;
}

int findVictim(struct PCB *pcb){
    int r=rand()%10;
    if(pcb->pageTable[r]==-1) return r;
    else{
        while((pcb->pageTable[r]!=-1) && r<=10) {
            r++;
            if(r>10) r=1;
        }
        return r;
    }
}

int updatePageTable(struct PCB *p, int pageNumber, int frameNumber, int victimFrame){
    if(detail()) printf("-> updating page table\n");
    if(frameNumber<0) {
        if(detail()) printf("-> setting pageNumber %d to VictimFrame %d\n", pageNumber, victimFrame);
        p->pageTable[pageNumber]=victimFrame;
        updateVictimPCB(victimFrame);
    }
    else{
        if(detail()) printf("-> setting pageNumber %d to new frame %d\n", pageNumber, frameNumber);
        p->pageTable[pageNumber]=frameNumber;
    }
    return 0;
}

int fileCopier(FILE *f, FILE *target){
    char **tokens;
    char buffer[100];
    while(!feof(f)){
        if(fgets(buffer, 100, f)==NULL) break;
        if(buffer[0]=='\n') continue;
        int len=strlen(buffer);
        if(buffer[len-1]!='\n' && len<100){
            buffer[len]='\n';
            buffer[len+1]='\0';
        }
        fputs(buffer, target);
        tokens=tokenize(buffer);
        if(strcmp(tokens[0], "run")==0 && tokens[1]!=NULL && tokens[2]==NULL) {
            //we will load the file of run command into the new file
            FILE *new=fopen(tokens[1], "r");
            fileCopier(new, target);  
            fclose(new);
        }
    }
    free(tokens);
    return 0;
}
//this will handle run command and copy the run file into the new file togather by recursion

int launcher(FILE *p){
    if(p==NULL) {
        printf("Error: File does not exist\n");
        return -1;
    }
    if(detail()) printf("-> Calling launcher\n");
    char tmp[4];
    sprintf(tmp, "%d", FILENAME);
    char *filepath=malloc(20);
    filepath = strcpy(filepath, "BackingStore/");
    filepath = strcat(filepath, tmp);    FILENAME++;
    //create a new file with new name
    FILE *f=fopen(filepath, "w+");
    if(detail()) printf("-> New file created\n");
    fileCopier(p, f);
    fclose(p);
    if(detail()) printf("-> Old file closed\n");
    //copy and close the original file
    fclose(f);
    f=fopen(filepath, "r");
    free(filepath);
    struct PCB *pcb;
    int length=countTotalPages(f);
    if(length<2) {
        pcb=makePCB(1, f);
        int targetFrame=findFrame();
        if(targetFrame==-1) {
            targetFrame=findVictim(pcb);
        }
        loadPage(0, f, targetFrame);
        pcb->pageTable[0]=targetFrame;
    }
    else {
        pcb=makePCB(length,f);
        int targetFrame=findFrame();
        if(targetFrame==-1) {
            targetFrame=findVictim(pcb);
        }
        loadPage(0, f, targetFrame);
        pcb->pageTable[0]=targetFrame;

        targetFrame=findFrame();
         if(targetFrame==-1) {
            targetFrame=findVictim(pcb);
        }
        loadPage(1, f, targetFrame);
        pcb->pageTable[1]=targetFrame;
    }
    pcb->PC=pcb->pageTable[pcb->PC_page]*4;
    addToReady(pcb);
    return 0;
}