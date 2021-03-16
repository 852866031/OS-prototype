#ifndef _PCB_H
#define _PCB_H
struct PCB{
    FILE *file;
    int PC; 
    int pageTable[10];
    int PC_page, PC_offset, pages_max;
    };


struct PCBNode{
    struct PCB *this;
    struct PCBNode *next;
    } ;

struct PCBList{
    struct PCBNode *head;
    struct PCBNode *tail;
    } ;

struct PCB *makePCB(int pageNumber, FILE *f);
void terminatePCB(struct PCB *pcb);
void printPageTable(struct PCB *pcb);
void updateVictimPCB(int frameNumber);
void addToReady(struct PCB *pcb);
void assign_head_to_cpu();
void from_cpu_to_tail();
int is_empty();
void destoryReady();
#endif