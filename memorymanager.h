#ifndef _MEMMANAGER_H
#define _MEMMANAGER_H
int launcher(FILE *p);
void loadPage(int pageNumber, FILE *f, int frameNumber);
int findFrame();
int findVictim(struct PCB *pcb);
int updatePageTable(struct PCB *p, int pageNumber, int frameNumber, int victimFrame);
#endif