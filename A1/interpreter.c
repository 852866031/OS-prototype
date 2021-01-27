#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"shellmemory.h"
#include"shell.h"
#include"interpreter.h"

void help() {
    printf("COMMAND          DESCRIPTION\n");
    printf("help             Displays all the commands\n");
    printf("quit             Exits / terminates the shell\n");
    printf("set VAR STRING   Assigns a value to shell memory\n");
    printf("print VAR        Displays the STRING assigned to VAR\n");
    printf("run SCRIPT.TXT   Executes the file SCRIPT.TXT\n");
}

int quit() { return 0; }

void set(char *var, char *value) {saveToMEM(var, value);}

void print(char *var){(printf("%s\n",findValue(var)));}

int run(char *name){
    FILE *f;
    char c;
    f=fopen(name, "r");
    if(f==NULL) {printf("Script not found\n"); return 2;}
    char line[100];
    char *tokens[100];
    int args=0;
    int status=1;
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
        args=parse(line,tokens);
        status=interpreter(tokens, args);
        if(status==0 || status==2) break;
    }
    fclose(f);
    return 1;
}

int interpreter(char **tokens, int len){
    char *cmd=tokens[0];
    int status=1; //1 for normal, 0 for exit and 2 for error
    if(strcmp(cmd, "help")==0 && len==1) help();
    else if(strcmp(cmd, "quit")==0 && len==1) status=quit();
    else if(strcmp(cmd, "set")==0 && len==3) set(tokens[1], tokens[2]);
    else if(strcmp(cmd, "print")==0 && len==2) print(tokens[1]);
    else if(strcmp(cmd, "run")==0 && len==2) status=run(tokens[1]);
    else if(len==0) status=1;
    else {
        printf("Unknown command with its arguments: ");
        for(int i=0; i<len; i++){
            printf("%s ", tokens[i]);
        } 
        printf("\nFull length parsed as: %d\n", len);
        status=2;
    }
    return status;
}
