#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"shellmemory.h"
#include"interpreter.h"
#include"kernel.h"
#include"pcb.h"
#include"cpu.h"
#include"memorymanager.h"
#include"DISK_driver.h"

char **tokenize(char *str) {
    size_t num_tokens = 1;
    int flag = 0;
    for (size_t i = 0; i < strlen(str); i++){
        if (flag == 0 && str[i] == ' '){
            num_tokens = num_tokens + 1;
            flag = 1;
        }
        if (str[i] != ' '){
            flag = 0;
        }
    }
    char **ret_arr = (char **)malloc(sizeof(char *) * (num_tokens + 1));
    if (ret_arr == NULL){
        perror("malloc");
        return NULL;
    }
    flag = 0;
    int ignore_flag = 0;
    char *modified_str = (char *)str;
    size_t counter = 0;
    const size_t length_str = strlen(str);
    for (size_t i = 0; i < length_str; i++){
        if (modified_str[i] == '\n' || modified_str[i] == '\r') modified_str[i] = ' ';
        if (modified_str[i] == '"') ignore_flag = ignore_flag ^ 0x1;
        if (flag == 0 && modified_str[i] != ' '){
            ret_arr[counter] = &(modified_str[i]);
            counter = counter + 1;
            flag = 1;
        }
        if (modified_str[i] == '\\' && modified_str[i + 1] == ' '){ i++; continue;}
        if (flag == 1 && modified_str[i] == ' ' && ignore_flag == 0){
            modified_str[i] = '\0';
            flag = 0;
            continue;
        }
    }
    ret_arr[counter] = NULL;
    for (size_t i = 0; i < counter; ++i){
        if (ret_arr[i][0] == '\"' && ret_arr[i][strlen(ret_arr[i] - 1)] == '\"'){
            ret_arr[i][strlen(ret_arr[i]) - 1] = '\0';
            ret_arr[i] = ret_arr[i] + 1;
        }
    }
    return ret_arr;
}

int help() {
    printf("COMMAND                            DESCRIPTION\n");
    printf("help                               Displays all the commands\n");
    printf("quit                               Exits / terminates the shell\n");
    printf("set VAR STRING                     Assigns a value to shell memory\n");
    printf("print VAR                          Displays the STRING assigned to VAR\n");
    printf("run SCRIPT.TXT                     Executes the file SCRIPT.TXT\n");
    printf("exec prog1 prog2 prog3             Executes up to 3 concurrent programs provided as arguments\n");
    printf("detail.on                          Turn on detail mode for exec\n");
    printf("detail.off                         Turn off detail mode for exec\n");
    printf("mount name number size             If partition with name not exist create a new one and initilize it\n");
    printf("                                   if exist, it will open that partition\n\n");
    printf("write filename [contents]          write contents into the file with name filename\n");
    printf("                                   if the file does not exist, it will create a new file\n\n");
    printf("read filename variable             load a block from filename into variable, file must exist\n");
    printf("rewind filename                    Rewind the file pointer to the beginning, file must open\n");
    printf("close filename                     close the file, its pointer is put to the beginning, file must open\n");
    printf("seek filename position             jump file 'filename' to location 'position'\n");
    printf("echo STATEMENT                     print STATEMENT\n");
    printf("info                               show the information of the current mount\n\n");
    return 0;
}

int quit() { 
    printf("Bye\n");
    return 999;
}

int set(const char *var, const char *value) {
    int error=shell_memory_set(var, value);
    if(error!=0) printf("set: unable to set shell memory");
    return error;
}

int print(const char *var){
    const char *value=shell_memory_get(var);
    if(value==NULL) {printf("print: Undefined value: %s\n", var); return 1;}
    printf("%s\n", value);
    return 0;
}

int runScript(const char *path) {
    if(cpu->cur_PCB!=NULL) {return 0; }
    FILE *f=fopen(path, "r");
    if(f==NULL) {printf("Script not found\n"); return 1;}
    char c;
    char line[100];
    int status=0;
    while(1){
        c=fgetc(f);
        if(c==EOF) break;
        int i=0;
        while(c!=10 && c!=EOF){
            line[i]=c;
            c=fgetc(f);
            i++;
            if(i>=100) break;
        }
        line[i]=0;
        if(line[0]==0) continue;
        status=interpreter(line);
        if(status!=0) break;
    }
    fclose(f);
    return 0;
}


int exec(char *prog1, char *prog2, char *prog3){
    if(prog1!=NULL) {
        if(launcher(fopen(prog1, "r"))<0) return -1;
    }
    if(prog2!=NULL) {
        if(launcher(fopen(prog2, "r"))<0) return -1;
    }
    if(prog3!=NULL) {
        if(launcher(fopen(prog3, "r"))<0) return -1;
    }
    int status = scheduler();
    return status;
}

int mount(char *partitionName, int number_of_blocks, int block_size){
    return partition(partitionName, block_size, number_of_blocks);
}

int write(char *name, char *data){
    int status=0;
    int block_size = get_block_size();
    if(block_size==0){
        printf("Error: No partition has been mounted, please mount first\n");
        return -1;
    }
    int datalen = strlen(data);
    int fat_index=is_open(name);
    if(fat_index==-1) fat_index=openfile(name);
    if(fat_index==-1) return -1;
    float tmp=(float) datalen/block_size;
    int number = datalen/block_size;
    if(tmp-number>0) number++; 
    if(detail()) printf("-> %d blocks will be overwritten\n",  number);
    char *buffer = malloc(block_size*sizeof(char));
    for(int i=0; i<block_size; i++) buffer[i]=0;
    int index=0;
    for(int i=0; i<number; i++){
        for(int j=0; j<block_size; j++){
            if(index<datalen){
                buffer[j]=data[index];
                index++;
            }
            else buffer[j]=48;
        }
        if(detail()) {
            printf("-> write block with content: ");
            for(int k=0; k<block_size; k++){
                printf("%c", buffer[k]);
            }
            printf("\n");
        }
        status=writeBlock(fat_index, buffer);
        if(status<0) break;
    }
    free(buffer);
    free(data);
    return status;
}

int read(char *name, char *variable){
    int status=0;
    char *buffer;
    int size = get_block_size();
    if(size==0){
        printf("Error: No partition has been mounted, please mount first\n");
        return -1;
    }
    if(!file_exist(name)){
        printf("Error: file does not exist\n");
        return -1;
    }
    int fat_index=is_open(name);
    if(fat_index==-1) fat_index=openfile(name);
    if(fat_index==-1) return -1;
    status=readBlock(fat_index);
    if(status<0) status=set(variable, "NULL");
    else {
        buffer=get_buffer_data();
        status=set(variable, buffer);
        free(buffer);
    }
    return status;
}

int rewind_file(char *name){
    int size = get_block_size();
    if(size==0){
        printf("Error: No partition has been mounted, please mount first\n");
        return -1;
    }
    return f_rewind(name);
}

int close_file(char *name){
    int size = get_block_size();
    if(size==0){
        printf("Error: No partition has been mounted, please mount first\n");
        return -1;
    }
    return f_close(name);
}

int seek_file(char *name, int position){
    int size = get_block_size();
    if(size==0){
        printf("Error: No partition has been mounted, please mount first\n");
        return -1;
    }
    return f_seek(name, position);
}

int interpreter(char *line){
    if(line[0]=='\n') return 0;
    char **tokens=tokenize(line);
    char *cmd=tokens[0];
    int status=0; 
    if(strcmp(cmd, "help")==0 && tokens[1]==NULL) status=help();
    else if(strcmp(cmd, "quit")==0 && tokens[1]==NULL) status=quit();
    else if(strcmp(cmd, "set")==0 && tokens[1]!=NULL 
    && tokens[2]!=NULL && tokens[3]==NULL) status=set(tokens[1], tokens[2]);
    else if(strcmp(cmd, "print")==0 && tokens[1]!=NULL
    && tokens[2]==NULL) status=print(tokens[1]);
    else if(strcmp(cmd, "run")==0 && tokens[1]!=NULL
    && tokens[2]==NULL) status=runScript(tokens[1]);
    else if(strcmp(cmd, "detail.on")==0 && tokens[1]==NULL) status=detailOn();
    else if(strcmp(cmd, "detail.off")==0 && tokens[1]==NULL) status=detailOff();
    else if(strcmp(cmd, "exec")==0 && tokens[1]!=NULL && tokens[2]==NULL){
        status=exec(tokens[1], NULL, NULL);
    }
    else if(strcmp(cmd, "exec")==0 && tokens[1]!=NULL && tokens[3]==NULL){
        status=exec(tokens[1], tokens[2], NULL);
    }
    else if(strcmp(cmd, "exec")==0 && tokens[1]!=NULL && tokens[4]==NULL){
        status=exec(tokens[1], tokens[2], tokens[3]);
    }
    else if(strcmp(cmd, "mount")==0 && tokens[1]!=NULL && tokens[2]!=NULL && tokens[3]!=NULL && tokens[4]==NULL){
        //check if tokens[3] and tokens[2] is number
        for(int i=0; i<strlen(tokens[2]); i++){
            if(tokens[2][i]>57 || tokens[2][i]<48){
                printf("Error: number_of_blocks should be an integer\n");
                return 1;
            }
        }
        for(int i=0; i<strlen(tokens[3]); i++){
            if(tokens[3][i]>57 || tokens[3][i]<48){
                printf("Error: block_size should be an integer\n");
                return 1;
            }
        }
        status=mount(tokens[1], atoi(tokens[2]), atoi(tokens[3]));
    }
    else if(strcmp(cmd, "write")==0 && tokens[1]!=NULL && tokens[2]!=NULL){
        int count = 0;
        int size = 0;
        char *data;
        while(tokens[count+2]!=NULL) {
            size+=strlen(tokens[count+2]);//size is the size of the input data (minus the [ ])
            count++;//count is the num of tokens
        }
        if(tokens[2][0]!=91 || tokens[count+2-1][strlen(tokens[count+2-1])-1]!=93){
            printf("Error: [ ] expected\n");
            return 1;
        }
        if(size-2<=0) {
            printf("Error: Invalid input\n");
            return 1;
        }
        data=malloc((size+count)*sizeof(char));
        int index=0;
        if(count==1){
            for(int i=1; i<strlen(tokens[2])-1; i++) {
                data[index]=tokens[2][i];
                index++;
            }
            data[index]=0;
        }
        else{
            for(int i=1; i<strlen(tokens[2]); i++) {
                data[index]=tokens[2][i];
                index++;
            }
            for(int i=3; i<2+count-1; i++){
                data[index]=32;
                index++;
                for(int j=0; j<strlen(tokens[i]); j++) {
                    data[index]=tokens[i][j];
                    index++;
                }
            }
            data[index]=32;
            index++;
            for(int i=0; i<strlen(tokens[count-1+2])-1; i++){
                data[index]=tokens[count-1+2][i];
                index++;
            }
            data[index]=0;
        }
        status=write(tokens[1], data);
    }
    else if(strcmp(cmd, "read")==0 && tokens[1]!=NULL && tokens[2]!=NULL && tokens[3]==NULL){
        status=read(tokens[1], tokens[2]);
    }
    else if(strcmp(cmd, "rewind")==0 && tokens[1]!=NULL && tokens[2]==NULL){
        rewind_file(tokens[1]);
    }
    else if(strcmp(cmd, "info")==0 && tokens[1]==NULL) status=info();
    else if(strcmp(cmd, "close")==0 && tokens[1]!=NULL && tokens[2]==NULL){
        status=close_file(tokens[1]);
    }
    else if(strcmp(cmd, "echo")==0){
        printf("\n");
        int i=1;
        while(tokens[i]!=NULL){
            printf("%s ", tokens[i]);
            i++;
        }
        printf("\n\n");
    }
    else if(strcmp(cmd, "seek")==0 && tokens[1]!=NULL && tokens[2]!=NULL && tokens[3]==NULL){
        for(int i=0; i<strlen(tokens[2]); i++){
            if(tokens[2][i]>57 || tokens[2][i]<48){
                printf("Error: position must be an integer\n");
                return 1;
            }
        }
        status=seek_file(tokens[1], atoi(tokens[2]));
    }
    else if(strcmp(cmd, "printAll")==0 && tokens[1]==NULL){
        int size = get_block_size();
        if(size==0){
            printf("Error: No partition has been mounted, please mount first\n");
            return -1;
        }
        print_all_blocks();
    }
    else {
        printf("Unknown command with its arguments: ");
        int i=0;
        while(tokens[i]!=NULL){
            printf("%s ", tokens[i]);
            i++;
        } 
        printf("\nLength parsed as: %d\n", i-1);
        status=1;
    }
    free(tokens);
    return status;
}
