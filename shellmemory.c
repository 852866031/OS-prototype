#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "shellmemory.h"
typedef struct{
    char *key;
    char *value;
} mem_t;

#define MEM_LENGTH 100
mem_t memory[MEM_LENGTH];

void shell_memory_initialize(){
    for (size_t i=0; i < MEM_LENGTH; ++i){
        memory[i].key = NULL;
        memory[i].value = NULL;
    }
}

void shell_memory_destory(){
    for (size_t i = 0; i < MEM_LENGTH; ++i){
        if (memory[i].key != NULL) free(memory[i].key);
        if (memory[i].value != NULL) free(memory[i].value);
    }
}

const char *shell_memory_get(const char *key){
    for (size_t i = 0; i < MEM_LENGTH; ++i){
        if (memory[i].key == NULL) continue;
        if (strcmp(memory[i].key, key) == 0) return memory[i].value;
    }
    return NULL;
}

int shell_memory_set(const char *key, const char *value) {
    for (size_t i = 0; i < MEM_LENGTH; ++i) {
        if (memory[i].key == NULL) continue;
        if (strcmp(memory[i].key, key) == 0){
            free(memory[i].value);
            memory[i].value = strdup(value);
            return 0;
        }
    }
    size_t possible_slot = MEM_LENGTH;
    for(size_t i = 0; i < MEM_LENGTH; ++i) {
        if (memory[i].key == NULL && memory[i].value == NULL) {
            possible_slot = i;
            break;
        }
    }
    if (possible_slot == MEM_LENGTH){ return -1;}
    else{
        memory[possible_slot].key = strdup(key);
        memory[possible_slot].value = strdup(value);
        return 0;
    }
}


