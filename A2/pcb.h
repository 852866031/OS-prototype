#ifndef _PCB_H
#define _PCB_H
struct PCB{
    int PC; 
    /*PC will be an integer number that refers to the cell number of ram[] 
    containing the instruction to execute.
    Note 1: the PCB’s PC field is not the CPU instruction pointer, 
    therefore it is updated only after a task switch. 
    The PCB’s PC is updated after the quanta is finished.
    Note 2: when a program is launched its PCB is created 
    and the PC field points to the first line of the program.*/
    int start; 
    //start contains the cell number of ram[] of the first instruction of the program.
    int end;
    //end variable contains the cell number of ram[] of the last instruction of the program
    };


struct PCBNode{
    struct PCB *this;
    struct PCBNode *next;
    } ;

struct PCBList{
    struct PCBNode *head;
    struct PCBNode *tail;
    } ;

struct PCB *makePCB(int start, int end);
void addToReady(struct PCB *pcb);
void assign_head_to_cpu();
void from_cpu_to_tail();
int is_empty();
void destoryReady();
#endif