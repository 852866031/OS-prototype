#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H
int shell_memory_set(const char *key, const char *value);
const char *shell_memory_get(const char *key);
void shell_memory_initialize();
void shell_memory_destory();
#endif