#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"shellmemory.h"
#include"interpreter.h"
#include"kernel.h"
#include"pcb.h"
#include"cpu.h"
#include"memorymanager.h"

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
