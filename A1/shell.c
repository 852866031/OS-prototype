#include<stdio.h>
#include<stdlib.h>
#include<string.h>
//#include<unistd.h>
#include"interpreter.h"

int parse(char *string, char **tokens) {
    char buffer[1000];
    int c=0, b=0, t=0;
    while(string[c]!=0){
        while(string[c]==32) c++;
        //skip all the space
        if(string[c]==0) break;
        b=0;
        while(string[c]!=32 && string[c]!=0) {
            buffer[b]=string[c];
            b++;
            c++;
        }
        buffer[b]=0;
        tokens[t]=strdup(buffer);
        t++;
    }
    return t;
}

int get_line(char *line){
    char c=fgetc(stdin);
    int i=0;
    while(c!=10){
        if(c==EOF) return 0;
        line[i]=c;
        c=fgetc(stdin);
        i++;
    }
    line[i]=0;
    return 1;
}

int main() {
    char *tokens[100];
    int status=1;
    int len=0;
    printf("\n");
    printf("Welcome to the Jiaxuan Chen shell!\n");
    printf("Version 1.0 Created January 2020\n");
    char line[1024];
    while(status){
        //if(isatty(STDIN_FILENO)) printf("$ ");
        printf("$ ");//delete this line in linux 
        /*if(get_line(line)==0) {
            FILE *tty;
            tty = freopen("/dev/tty", "r", stdin);
            continue;
        };*/
        get_line(line);//delete this line in linux 
        //if(!isatty(STDIN_FILENO)) {printf("$ %s\n", line);}
        len=parse(line, tokens);
        status=interpreter(tokens, len);
        if(status==2) status=1;
    }
    printf("Bye\n");
    return 0;
}