#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"pcb.h"
#include"ram.h"
#include"cpu.h"
#include"shell.h"

int detailMode=0;
int detail(){return detailMode;}
int detailOn(){if(detail()==0) detailMode=1; return 0;}
int detailOff(){if(detail()==1) detailMode=0; return 0;}
int myinit(char *filename) {
    if(detail()) printf("Calling myinit in kernel.c\n");
    FILE *f = fopen(filename, "r");
    if(f==NULL) {
        printf("File %s is not found\n", filename);
        return -1;
    }
    if(detail()) printf("-> File %s opened successfully\n", filename);
    int x=0;
    int y=0;
    int *start=&x;
    int *end=&y;
    addToRam(f, start, end);
    if(*end<0) {
        if(detail()) printf("-> Init fails, ending\n");
        return -1;
    }
    addToReady(makePCB(*start, *end));
    fclose(f);
    return 0;
}

int scheduler(){
    if(detail()) printf("-> Scheduler Launched\n");
    int quantaNeed;
    while(is_empty()==0){
        if(cpu->cur_PCB!=NULL) continue; //check if cpu is available
        quantaNeed=cpu->quanta;
        assign_head_to_cpu();
        cpu->IP=cpu->cur_PCB->PC;
        //Copy the PC from the PCB into the IP of the CPU.
        if(cpu->cur_PCB->end - cpu->cur_PCB->PC+1<quantaNeed) {
            quantaNeed=cpu->cur_PCB->end - cpu->cur_PCB->PC+1;
        }
        if(run(quantaNeed)==999) return 0;
        cpu->cur_PCB->PC+=quantaNeed;//update the PCB
        if(cpu->cur_PCB->PC>cpu->cur_PCB->end) {//check if finish
            if(detail()) printf("-> Program complete, terminating\n");
            unloadFromRam(cpu->cur_PCB->start, cpu->cur_PCB->end);
            free(cpu->cur_PCB);
            cpu->cur_PCB=NULL;
        }
        else from_cpu_to_tail();
    }
    if(detail()) printf("-> Scheduler ends\n");
    return 0;
}

int main(){
    initializeCPU();
    initializeRAM();
    printf("Kernel 1.0 loaded!\n");
    shellUI();
    free(cpu);
    destoryReady();
    return 0;
}
//gcc -c shell.c interpreter.c shellmemory.c kernel.c cpu.c pcb.c ram.c
//gcc -o mykernel shell.o interpreter.o shellmemory.o kernel.o cpu.o pcb.o ram.o