#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include"DISK_driver.h"
#include"kernel.h"

/////////////////////////////////////////////////////////////////// globles
struct PARTITION {
    char *name;
    int total_blocks;
    int block_size;
} *Partition;

struct FAT {
    char filename[18];
    int file_length;
    int blockPtrs[10];
    int current_location;
} *fat[20];

char *block_buffer;

int info_size=0;//this store the size of the structs so when we need to get to data blocks just fseek(f, info_size)

struct fileTable{
    FILE *file; //the file pointer
    int fat_index; //which entry of far this file belongs to. -1 for not in fat
} *active_file_table[5];

///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////// helpers
void print_all_blocks(){
    FILE *f=fopen(Partition->name, "r");
    fseek(f, info_size, SEEK_SET);
    for(int i=0; i<Partition->total_blocks; i++){
        for(int j=0; j<Partition->block_size; j++){
            printf("%c", fgetc(f));
        }
        printf("\n");
    }
    fclose(f);
}

int get_block_size() {return Partition->block_size;}

void to_block(FILE *f, int num){
    if(num>Partition->total_blocks) return;
    fseek(f, info_size+num*Partition->block_size, SEEK_SET);
    //now f points the num th block
}

int copyBlock(FILE *f, int index, char *buffer){
    to_block(f, index);
    for(int i=0; i<Partition->block_size; i++) buffer[i]=fgetc(f);
    return 1;
}

char *get_buffer_data(){
    int i;
    for(i=Partition->block_size-1; i>=0; i--){
        if(block_buffer[i]!=48) break; 
    }
    char *buffer = malloc((i+2)*sizeof(char));
    for(int j=0; j<i+1; j++) buffer[j]=block_buffer[j];
    buffer[i+1]=0;
    return buffer;
}

int is_open(char *filename){
    for(int i=0; i<5; i++){
        if(active_file_table[i]->fat_index==-1) continue;
        if(strcmp(fat[active_file_table[i]->fat_index]->filename, filename)==0) return active_file_table[i]->fat_index;
    }
    return -1;
}

void write_PARTITION_info(FILE *f){
    rewind(f);
    fwrite(&Partition->total_blocks, sizeof(int), 1, f);
    fwrite(&Partition->block_size, sizeof(int), 1, f);
    for(int i=0; i<20; i++) {
        fwrite(fat[i]->filename, 18*sizeof(char), 1, f);
        fwrite(&fat[i]->file_length, sizeof(int), 1, f);
        for(int j=0; j<10; j++){
            fwrite(&fat[i]->blockPtrs[j], sizeof(int), 1, f);
        }
        fwrite(&fat[i]->current_location, sizeof(int), 1, f);
    }
    if(detail()) printf("-> FAT written to disk\n");
}

void read_PARTITION_info(FILE *f){
    rewind(f);
    fread(&Partition->total_blocks, sizeof(int), 1, f);
    fread(&Partition->block_size, sizeof(int), 1, f);
    for(int i=0; i<20; i++) {
        fread(fat[i]->filename, 18*sizeof(char), 1, f);
        fread(&fat[i]->file_length, sizeof(int), 1, f);
        for(int j=0; j<10; j++){
            fread(&fat[i]->blockPtrs[j], sizeof(int), 1, f);
        }
        fread(&fat[i]->current_location, sizeof(int), 1, f);
    }
    if(detail()) printf("-> FATs from disk loaded to ram\n");
}

int file_exist(char *name){
    for(int i=0; i<20; i++){
        if(strcmp(fat[i]->filename, name)==0) return 1;
    }
    return 0;
}

void print_FAT(int index){
    printf("fat[%d]: filename: %s\n", index, fat[index]->filename);
    printf("        filelength: %d\n", fat[index]->file_length);
    printf("        Block pointers:");
    for(int i=0; i<10; i++) printf(" %d", fat[index]->blockPtrs[i]);
    printf("\n        current location: %d\n", fat[index]->current_location);
}

int int_size(int num){
    if(num==0) return 1;
    double k=num;
    int i=0;
    while(k>=1){
        k=k/10;
        i++;
    }
    return i;
}
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////// functions

void initIO(){
    Partition = malloc(sizeof(struct PARTITION));
    Partition->block_size=0;
    Partition->total_blocks=0;
    for(int i=0; i<20; i++) {
        fat[i] = malloc(sizeof(struct FAT));
        for(int j=0; j<10; j++) fat[i]->filename[j]=0;
        fat[i]->current_location=-1;
        fat[i]->file_length=0;
        for(int j=0; j<10; j++) fat[i]->blockPtrs[j]=-1;
    }
    
    for(int i=0; i<5; i++){
        active_file_table[i]=malloc(sizeof(struct fileTable));
        active_file_table[i]->file=NULL;
        active_file_table[i]->fat_index=-1;
    }
    block_buffer=NULL;
}

void clear_structs(){
    for(int i=0; i<5; i++){
        if(active_file_table[i]->fat_index!=-1) {
            fat[active_file_table[i]->fat_index]->current_location=0;
            write_PARTITION_info(active_file_table[i]->file);
            fclose(active_file_table[i]->file);
        }
        active_file_table[i]->file=NULL;
        active_file_table[i]->fat_index=-1;
    }
    Partition->block_size=0;
    Partition->total_blocks=0;
    for(int i=0; i<20; i++) {
        for(int j=0; j<10; j++) fat[i]->filename[j]=0;;//make it empty
        fat[i]->current_location=-1;
        fat[i]->file_length=0;
        for(int j=0; j<10; j++) fat[i]->blockPtrs[j]=-1;
    }
}

void destoryIO(){
    clear_structs();
    free(Partition);
    for(int i=0; i<5; i++){
        if(active_file_table[i]!=NULL) free(active_file_table[i]);
    }
    if(block_buffer!=NULL) free(block_buffer);
    for(int i=0; i<20; i++)  free(fat[i]);
}

int partition(char *name, int blocksize, int totalblocks){
    if(access("PARTITION", F_OK)==-1) {
        system("mkdir PARTITION"); 
        if(detail()) printf("-> create the directory\n");
    }
    char *filepath=malloc(12+strlen(name));
    filepath = strcpy(filepath, "PARTITION/");
    filepath = strcat(filepath, name);
    if(access(filepath, F_OK)!=-1) {//check if the file exists
        if(detail()) printf("-> Partition %s exists\n", filepath);
        if(detail()) printf("-> Calling mountFS to mount the partition\n");
        return mountFS(name);
    }//if the file already exists then we just call mountFS
    if(blocksize<=0 || totalblocks<=0){
        printf("Error: cannot create partition with size 0\n");
        return 0;
    }
    clear_structs();//reset all the information about partition and fat
    if(detail()) printf("-> Creating Partition %s\n", filepath);
    FILE *f=fopen(filepath, "w+");//create a new file if the file not exist
    if(f==NULL) return 0;
    Partition->name=strdup(filepath);
    Partition->total_blocks=totalblocks;
    Partition->block_size=blocksize;
    write_PARTITION_info(f);
    info_size=ftell(f);

    for(int i=0; i<Partition->block_size*Partition->total_blocks; i++) fputc(48, f);

    fclose(f);
    if(detail()) printf("-> Partition %s created, size: %d, block size: %d\n", filepath, Partition->total_blocks, Partition->block_size);
    free(filepath);
    if(block_buffer!=NULL) free(block_buffer);
    block_buffer=malloc(Partition->block_size*sizeof(char));
    for(int i=0; i<Partition->block_size; i++) block_buffer[i]=48;
    return 1;
}

int mountFS(char *name){
    char *filepath=malloc(12+strlen(name));
    int tmp=0;
    char c;
    filepath = strcpy(filepath, "PARTITION/");
    filepath = strcat(filepath, name);
    FILE *f=fopen(filepath, "r");
    if(f==NULL) return 0;
    clear_structs();//reset all the information about partition and fat
    Partition->name=strdup(filepath);
    read_PARTITION_info(f);
    info_size=ftell(f);
    if(block_buffer!=NULL) free(block_buffer);
    block_buffer=malloc(Partition->block_size*sizeof(char));
    for(int i=0; i<Partition->block_size; i++) block_buffer[i]=48;
    fclose(f);
    free(filepath);
    return 1;
}

int openfile(char *name){
    int i;
    int j;
    char c;
    if(detail()) printf("-> Open file with name %s\n", name);
    if(detail()) printf("-> Searching in fat\n");
    for(i=0; i<20; i++){
        if(fat[i]->filename[0]!=0 && strcmp(fat[i]->filename, name)==0) break;//find the file we want
    }
    int fat_index=i;

    if(i==20) {//in this case we need to create a new file
        int count=0;//this count the number of block used
        if(detail()) printf("-> File not exist in fat, creating a new file\n");
        for(j=0; j<20; j++) {
            count+=fat[j]->file_length;
            if(fat[j]->filename[0]==0) break;
        }
        if(count>=Partition->total_blocks){
            printf("Error: partition is full cannot create new file\n");
            return -1;
        }
        if(j==20) {//this means the fat is full
            printf("Error: fat is full, cannot create new file\n");
            return -1; //fat is full
        }
        if(strlen(name)>=17) {
            printf("Error: filename too long\n");
            return -1;
        }
        for(int k=0; k<strlen(name); k++){
            fat[j]->filename[k]=name[k];
        }
        fat[j]->filename[strlen(name)]=0;
        fat[j]->current_location=0;
        if(detail()) printf("-> New entry created for this new file\n");
        fat_index=j;
        //since a new file is created we need to update the fat into the disk
        FILE *f=fopen(Partition->name, "r+");
        write_PARTITION_info(f);
        fclose(f);
    }//if the file not in fat create a new entry

    for(i=0; i<5; i++){
        if(active_file_table[i]->file==NULL) break;
    }//find the first free entry
    if(i==5) {
        printf("Error: active file table is full, if a file is created, it will not open but still exists\n");
        return -1;//no available cells in the active file table
    }

    if(detail()) printf("-> Free entry found in active file table, index: %d\n", i);
    active_file_table[i]->file=fopen(Partition->name, "r+");
    active_file_table[i]->fat_index=fat_index;
    if(detail()) printf("-> New active file pointer created\n");
    if(fat[fat_index]->blockPtrs[0]!=-1) {
        to_block(active_file_table[i]->file, fat[fat_index]->blockPtrs[0]);//this is the first block of the file
    }
    return fat_index;
}

int f_rewind(char *name){
    int fat_index;
    int file_index;
    for(fat_index=0; fat_index<20; fat_index++){
        if(strcmp(fat[fat_index]->filename, name)==0) break;
    }
    if(fat_index==20) {
        printf("Error: file not exist\n");
        return -1;
    }
    for(file_index=0; file_index<5; file_index++){
        if(active_file_table[file_index]->fat_index==fat_index) break;
    }
    if(file_index==5){
        printf("Error: file is not open\n");
        return -1;
    }
    fat[fat_index]->current_location=0;
    write_PARTITION_info(active_file_table[file_index]->file);
    return 1;
}

int f_close(char *name){
    int fat_index;
    int file_index;
    for(fat_index=0; fat_index<20; fat_index++){
        if(strcmp(fat[fat_index]->filename, name)==0) break;
    }
    if(fat_index==20) {
        printf("Error: file not exist\n");
        return -1;
    }
    for(file_index=0; file_index<5; file_index++){
        if(active_file_table[file_index]->fat_index==fat_index) break;
    }
    if(file_index==5){
        printf("Error: file is not open\n");
        return -1;
    }
    fat[fat_index]->current_location=0;
    write_PARTITION_info(active_file_table[file_index]->file);
    fclose(active_file_table[file_index]->file);
    active_file_table[file_index]->file=NULL;
    active_file_table[file_index]->fat_index=-1;
    return 1;
}

int f_seek(char *name, int position){
    int fat_index;
    int file_index;
    for(fat_index=0; fat_index<20; fat_index++){
        if(strcmp(fat[fat_index]->filename, name)==0) break;
    }
    if(fat_index==20) {
        printf("Error: file not exist\n");
        return -1;
    }
    for(file_index=0; file_index<5; file_index++){
        if(active_file_table[file_index]->fat_index==fat_index) break;
    }
    if(file_index==5){
        printf("Error: file is not open\n");
        return -1;
    }
    if(position>fat[fat_index]->file_length) {
        printf("Error: the input is larger then file length\n");
        return -1;
    }
    if(position==fat[fat_index]->file_length){
        printf("Note that you are at the end of the file\n");
    }
    fat[fat_index]->current_location=position;
    write_PARTITION_info(active_file_table[file_index]->file);
    return 1;
}

int readBlock(int file){
    FILE *pointer=NULL;
    for(int i=0; i<5; i++) {
        if(active_file_table[i]->fat_index==file) pointer = active_file_table[i]->file;
    }
    if(pointer==NULL) {
        printf("ERROR: No valid file pointer in the active file table\n");
        return -1;  
    }
    int block_index = fat[file]->blockPtrs[fat[file]->current_location];
    if(block_index==-1) {
        printf("Warning: End of file, NULL\n");
        return -1;
    }
    if(fat[file]->current_location==fat[file]->file_length) {
        printf("Warning: End of file, NULL\n");
        return -1;
    }
    char *block = malloc(Partition->block_size*sizeof(char));
    copyBlock(pointer, block_index, block);
    if(block==NULL) return -1; //fail for end of Partition file

    if(detail()) printf("-> Copying block into the block_buffer\n");
    for(int i=0; i<Partition->block_size; i++) block_buffer[i]=block[i];
    free(block);

    fat[file]->current_location++;
    //update the fat table in the Partition
    return 1;
}

int writeBlock(int file, char *data){
    FILE *pointer;
    for(int i=0; i<5; i++) {
        if(active_file_table[i]->fat_index==file) pointer = active_file_table[i]->file;
    }
    if(pointer==NULL) {
        printf("Error: No valid file pointer in the active file table");
        return -1;  
    }

    if(detail()) printf("-> File pointer found in the active file table\n");
    int block_index = fat[file]->blockPtrs[fat[file]->current_location];
    if(detail()) printf("-> Current location is at block %d\n", block_index);

    char block[Partition->block_size];
    if(block_index==-1){
        if(detail()) printf("-> Invalid block, searching for a free block\n");
        //we need to find a free block to write the file
        for(block_index=0; block_index<Partition->total_blocks; block_index++){
            //check all the blocks
            copyBlock(pointer, block_index, block);
            int i;
            for(i=0; i<Partition->block_size; i++){
                if(block[i]!=48) break; 
            }
            if(i==Partition->block_size) break;
        }
        if(block_index==Partition->total_blocks) {
            printf("Error: No free block in this partition\n");
            printf("       Content: %s\n        and anything after it are not written\n", data);
            return -1; //there is no free block
        }
        if(detail()) printf("-> Free block found, index: %d\n", block_index);
        fat[file]->blockPtrs[fat[file]->current_location]=block_index;//update the fat
        fat[file]->file_length++;
    }//after this the block_index equals the block we want to write

    to_block(pointer, block_index);
    if(detail()) printf("-> Moving the pointer to target block\n");
    for(int i=0; i<Partition->block_size && i<strlen(data); i++){
        fputc(data[i], pointer);
    }
    if(detail()) printf("-> Block overwritten\n");
    fat[file]->current_location++;
    write_PARTITION_info(pointer);
    return 1;
}

int info(){
    if(Partition->block_size==0){
        printf("Error: No partition has been mounted, please mount first\n");
        return -1;
    }
    printf("Partition name: %s\n", Partition->name);
    printf("This partition has %d blocks, each has size %d\n", Partition->total_blocks, Partition->block_size);
    int count=0;
    for(int i=0; i<20; i++){
        if(fat[i]->filename[0]==0) continue;
        count+=fat[i]->file_length;
        if(i==0) printf("   FILENAME     LENGTH     CUR_POSITION     STATUS     BLOCKS\n");
        for(int j=0; j<3; j++) printf(" ");
        printf("%s", fat[i]->filename);
        for(int j=0; j<15-strlen(fat[i]->filename); j++) printf(" ");
        printf("%d", fat[i]->file_length);
        for(int j=0; j<15-int_size(fat[i]->file_length); j++) printf(" ");
        printf("%d", fat[i]->current_location);
        for(int j=0; j<11-int_size(fat[i]->current_location); j++) printf(" ");
        printf("%s", (is_open(fat[i]->filename)<0)? "closed":"open");
        for(int j=0; j<11-((is_open(fat[i]->filename)<0)? 6:4); j++) printf(" ");
        for(int j=0; j<fat[i]->file_length;j++) printf("%d ", fat[i]->blockPtrs[j]);
        printf("\n");
    }
    printf("%d blocks used and %d blocks free\n", count, Partition->total_blocks-count);
    return 1;
}
/////////////////////////////////////////////////////////////////// end
