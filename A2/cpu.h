#ifndef _CPU_H
#define _CPU_H
struct CPU {int IP; char IR[1000]; int quanta; struct PCB *cur_PCB;};
extern struct CPU *cpu;
int run(int quanta);
void initializeCPU();
#endif