#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct MEM {
    char *var;
    char *value;
};
struct MEM **memArray = NULL;
int len = 0;

void printMEM(){
    for(int i=0; i<len; i++){
        printf("var: %s, value: %s\n", memArray[i]->var, memArray[i]->value);
    }
} 


int saveToMEM(char *v, char *va) {
    if(memArray==NULL) {  memArray = malloc(1000* sizeof(struct MEM)); } //initialize the array
    for(int i=0; i<len; i++) { 
        if(strcmp(memArray[i]->var, v)==0) {
            memArray[i]->value = va; 
            return 0;
        }
    }
    //if the var already exists, update its value
    len++;
    struct MEM *new = malloc(sizeof(struct MEM));
    new->var = v;
    new->value = va;
    memArray[len-1]=new;//add new element into the array
    return 0;
}

char *findValue(char *v){
    if(memArray==NULL) return "Variable does not exist due to no var exists";
    for(int i=0; i<len; i++) { 
        if(strcmp(memArray[i]->var, v)==0) return memArray[i]->value; 
    }
    return "Variable does not exist";
}


