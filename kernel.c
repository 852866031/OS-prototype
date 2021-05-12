#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"pcb.h"
#include"ram.h"
#include"cpu.h"
#include"shell.h"
#include"DISK_driver.h"

int detailMode=0;
int detail(){return detailMode;}
int detailOn(){if(detail()==0) detailMode=1; return 0;}
int detailOff(){if(detail()==1) detailMode=0; return 0;}

int scheduler(){
    if(detail()) printf("-> Scheduler Launched\n");
    while(1){
        if(cpu->cur_PCB!=NULL) continue; //check if cpu is available
        assign_head_to_cpu();
        cpu->IP=cpu->cur_PCB->pageTable[cpu->cur_PCB->PC_page]*4;
        //Copy the PC from the PCB into the IP of the CPU.
        //note that cpu->IP can be less then 0, when there is a hole in the pagetable
        int result=run();
        if(result==1) {
            cpu->cur_PCB=NULL;
        }
        else if(result==999) return 999;
        else from_cpu_to_tail();
        if(is_empty()) break;
        
    }
    if(detail()) printf("-> Scheduler ends\n");
    return 0;
}

int kernel(){    
    printf("Kernel 1.0 loaded!\n");
    shellUI();
    free(cpu);
    destoryReady();
    destoryIO();
    return 0;
}

void boot(){
    initializeRAM();
    initializeCPU();
    initIO();
    system("rm -r BackingStore\n");
    system("mkdir BackingStore\n");
}

int main(){
    int error=0;
    boot();
    error=kernel();
    return error;
}

//gcc -c shell.c interpreter.c shellmemory.c kernel.c cpu.c pcb.c ram.c memorymanager.c
//gcc -o mykernel shell.o interpreter.o shellmemory.o kernel.o cpu.o pcb.o ram.o memorymanager.o