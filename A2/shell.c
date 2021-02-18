#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include"interpreter.h"
#include"shellmemory.h"

int check(int i){printf("checkpoint %d\n", i);}
int shellUI(){
    printf("Welcome to the Jiaxuan Chen shell!\n"
           "Version 1.0 Created January 2020\n");

    shell_memory_initialize();
    int status=0;
     while (!feof(stdin)){
        if(isatty(STDIN_FILENO)) printf("$ ");
        fflush(stdout);
        char *line = NULL;
        size_t linecap = 0;
        if (getline(&line, &linecap, stdin) == -1) {
            if(!isatty(STDIN_FILENO)){
                FILE *tty;
                 tty = freopen("/dev/tty", "r", stdin);
                 continue;
            }
            break;
        }
        if(!isatty(STDIN_FILENO)) {
            if(line[0]=='\n') printf("$ \n");
            else {
                if(line[strlen(line)-1]!='\n') printf("$ %s\n", line);
                else printf("$ %s", line);
            }
        }
        if(line[0]=='\n') continue;

        status=interpreter(line);
        free(line);
        if(status==999) break;
    }
    shell_memory_destory();
    return 0;
}