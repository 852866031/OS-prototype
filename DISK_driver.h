#ifndef _DISK_DRIVER_H
#define _DISK_DRIVER_H

void initIO();
int partition(char *name, int blocksize, int totalblocks);
int mountFS(char *name);
void destoryIO();
int openfile(char *name);
int readBlock(int file);
int writeBlock(int file, char *data);
char *get_buffer_data();
int is_open(char *filename);
int get_block_size();;
int f_rewind(char *name);
void print_FAT(int index);
int info();
int f_close(char *name);
int f_seek(char *name, int position);
void print_all_blocks();
int file_exist(char *name);
#endif