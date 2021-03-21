#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"ram.h"
#include"interpreter.h"
#include"kernel.h"
#include"pcb.h"
#include"memorymanager.h"
#include"cpu.h"

struct CPU *cpu;
char *ram[40];

void initializeCPU(){
    if(detail()) printf("-> CPU initializing\n");
    if(cpu==NULL){
        cpu=malloc(sizeof(struct CPU));
        cpu->quanta=2;
        cpu->cur_PCB=NULL;
        if(detail()) printf("-> CPU initialized with quanta 2\n");
    }
}

int pageFault(){
    if(detail()) printf("-> pageFault interrupt\n");
    cpu->cur_PCB->PC_page++;
    if(cpu->cur_PCB->PC_page >= cpu->cur_PCB->pages_max) {
        terminatePCB(cpu->cur_PCB);
        return 1;
    }
    int frame=-1;
    if(cpu->cur_PCB->PC_page<10) frame=cpu->cur_PCB->pageTable[cpu->cur_PCB->PC_page];
    if(frame!=-1) {
        if(detail()) printf("-> Next page exists in corresponding frame, points to next page\n");
        cpu->cur_PCB->PC=frame*4;
        cpu->cur_PCB->PC_offset=0;
    }
    else{
        if(detail()) printf("-> Next page does not exist in page table, finding new frame\n");
        frame=findFrame();
        if(frame<0) {
            if(detail()) printf("-> RAM is full, finding victim\n");
            frame=findVictim(cpu->cur_PCB);
            if(detail()) printf("-> Victim found, frame number: %d\n", frame);
            int page = (cpu->cur_PCB->PC_page > 9)? 9 : cpu->cur_PCB->PC_page;
            //if we find a victim, we always let the last page of the table pointing to it,
            //we dont change the value of PC_page to let it compare with pages_max
            updatePageTable(cpu->cur_PCB, page , -1, frame);
        }
        else{
            if(detail()) printf("-> New frame found, number: %d\n", frame);
            updatePageTable(cpu->cur_PCB, cpu->cur_PCB->PC_page, frame, 0);
        }
        loadPage(cpu->cur_PCB->PC_page, cpu->cur_PCB->file, frame);
        cpu->cur_PCB->PC=frame*4;
        if(cpu->IP>=0) cpu->cur_PCB->PC_offset=0;
    }
    if(detail()) printPageTable(cpu->cur_PCB);
    return 0;
}

int run(){
    if(detail()) printf("Calling function run in cpu.c\n");
     if(cpu->IP<0) {
            if(detail()) printf("-> The page we want to execute was picked as victim\n");
            cpu->cur_PCB->PC_page--;
            //so when we do page fault we will replace the current page;
            return pageFault();
        }
    if(detail()) printf("-> CPU Running, executing from page %d frame %d, offset %d, which is line %d in ram\n", cpu->cur_PCB->PC_page,
    cpu->cur_PCB->pageTable[cpu->cur_PCB->PC_page], cpu->cur_PCB->PC_offset, cpu->IP+cpu->cur_PCB->PC_offset);
    for(int i=0; i<cpu->quanta; i++){
        //this happens when findVictim gives the frame that we are executing
        if(ram[cpu->IP+cpu->cur_PCB->PC_offset]==NULL) {
            if(detail()) printf("-> NULL occur in current page, terminating this process\n");
            terminatePCB(cpu->cur_PCB);
            return 1;
            //if we return a null in this page, the process is end and shall be terminated
        }
        strcpy(cpu->IR, ram[cpu->IP+cpu->cur_PCB->PC_offset]);
        if(detail()) {
            if(cpu->IR[strlen(cpu->IR)-1]!='\n') {
                printf("-> Executing command: %s\n", cpu->IR);
            }
            else printf("-> Executing command: %s", cpu->IR);
        }

        if(interpreter(cpu->IR)==999) return 999;
        cpu->cur_PCB->PC_offset++;
        if(detail()) printf("-> Offset of current PCB is %d\n", cpu->cur_PCB->PC_offset);
        if(cpu->cur_PCB->PC_offset==4) {
            return pageFault();
        }
    }
    if(detail()) printf("-> CPU quanta complete\n");
    return 0;
}