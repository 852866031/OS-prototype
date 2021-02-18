#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"ram.h"
#include"interpreter.h"
#include"cpu.h"
#include"kernel.h"
#include"pcb.h"

struct CPU *cpu;

void initializeCPU(){
    if(detail()) printf("-> CPU initializing\n");
    if(cpu==NULL){
        cpu=malloc(sizeof(struct CPU));
        cpu->quanta=2;
        cpu->cur_PCB=NULL;
        if(detail()) printf("-> CPU initialized with quanta 2\n");
    }
}

int run(int quanta){
    if(detail()) printf("Calling function run in cpu.c\n");
    if(detail()) printf("-> CPU Running, executing from line %d\n", cpu->IP);
    for(int i=0; i<quanta; i++){
        copyLineFromRam(cpu->IP, cpu->IR);
        if(detail()) {
            if(cpu->IR[strlen(cpu->IR)-1]!='\n') {
                printf("-> Executing command: %s\n", cpu->IR);
            }
            else printf("-> Executing command: %s", cpu->IR);
        }
        if(interpreter(cpu->IR)==999) return 999;
        cpu->IP++;
    }
    if(detail()) printf("-> CPU quanta complete\n");
    return quanta!=cpu->quanta;
}