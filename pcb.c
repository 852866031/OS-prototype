#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"ram.h"
#include"cpu.h"
#include"pcb.h"
#include"kernel.h"

struct PCBList *readyList;
struct PCB *makePCB(int pageNumber, FILE *f){
    if(detail()) printf("Calling function makePCB in pcb.c\n");
    struct PCB *new = malloc(sizeof(struct PCB));
    new->PC=0;
    new->PC_offset=0;
    new->PC_page=0;
    new->pages_max=pageNumber;
    new->file=f;
    for(int i=0; i<10; i++) new->pageTable[i]=-1;//-1 for not used
    if(detail()) printf("-> New PCB created with PC: %d\n", new->PC);
    return new;
}

int ReadyLength(){
    if(readyList==NULL) return 0;
    if(readyList->head==NULL) return 0;
    int i=0;
    struct PCBNode *cur = readyList->head;
    while(cur!=NULL){
        cur = cur->next;
        i++;
    }
    return i;
}

void printPageTable(struct PCB *pcb){
    printf("-> current PCB page table is :\n");
    for(int i=0; i<pcb->pages_max; i++){
        printf("Page: %d ---> Frame: %d\n", i, pcb->pageTable[i]);
    }
}

void terminatePCB(struct PCB *pcb){
    if(detail()) printf("-> Terminating current PCB\n");
    for(int i=0; i<10; i++) {
        if(pcb->pageTable[i]>=0) unloadFrame(pcb->pageTable[i]);
    }
    fclose(pcb->file);
    free(pcb);
    if(detail()) printf("-> Terminated\n");
}

void updateVictimPCB(int frameNumber){
    struct PCBNode *cur = readyList->head;
    while(cur!=NULL){
        for(int i=0; i<cur->this->pages_max; i++){
            if(cur->this->pageTable[i]==frameNumber) {//now page i is to be deleted
                cur->this->pageTable[i]=-1;
                return;
            }
        }
        cur=cur->next;
    }//loop through the readyList to find the victim PCB

    for(int i=0; i<cpu->cur_PCB->pages_max; i++){
        if(cpu->cur_PCB->pageTable[i]==frameNumber) {
            cpu->cur_PCB->pageTable[i]=-1;
            return;
        }
    }//see if the PCB in the cpu is the victim (unlikely to happen)
}

void addToReady(struct PCB *pcb){
    if(detail()) printf("Calling function addToReady in pcb.c\n");
    if(readyList==NULL) {
        if(detail()) printf("-> Initializing ReadyList\n");
        readyList=malloc(sizeof(struct PCBList));
        readyList->head=NULL;
        readyList->tail=NULL;
    }
    struct PCBNode *new = malloc(sizeof(struct PCBNode));
    new->this=pcb;
    new->next=NULL;
    if(detail()) printf("-> Adding new PCB to ReadyList\n");
    if(readyList->head==NULL) {
        readyList->head=new;
        readyList->tail=new;
    }
    else{ 
        readyList->tail->next=new;
        readyList->tail=new;
    }
    if(detail()) printf("-> %d PCBs in ReadyList currently\n", ReadyLength());
}

void assign_head_to_cpu(){
    if(detail()) printf("Calling function assign_head_to_cpu in pcb.c\n");
    if(readyList->head==NULL) {
        printf("Error: empty readylist\n");
        return;
    }
    cpu->cur_PCB=readyList->head->this;
    if(readyList->head==readyList->tail){
        free(readyList->head);
        readyList->head=NULL;
        readyList->tail=NULL;
    }
    else{
        struct PCBNode *newhead=readyList->head->next;
        free(readyList->head);
        readyList->head=newhead;
    }
    if(detail()) {
        printf("-> Head of the ReadyList is removed and assigned to CPU\n");
        printf("-> %d PCBs in ReadyList currently\n", ReadyLength());
    }
}

void from_cpu_to_tail(){
    if(detail()) printf("Calling function from_cpu_to_tail in pcb.c\n");
    if(readyList->head==NULL) {
        addToReady(cpu->cur_PCB);
    }
    else{
        struct PCBNode *tmp = malloc(sizeof(struct PCBNode));
        tmp->this=cpu->cur_PCB;
        tmp->next=NULL;
        readyList->tail->next=tmp;
        readyList->tail=tmp;
    }
    cpu->cur_PCB=NULL;
    if(detail()) printf("-> PCB in the cpu assign back to tail of readyList\n");
}

int is_empty(){
    if(readyList->head==NULL && cpu->cur_PCB==NULL) {
        if(detail()) printf("-> The readyList and CPU's PCB is empty\n");
        return 1;
    }
    else return 0;
}

void destoryReady() {free(readyList);}
