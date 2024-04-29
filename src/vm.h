#ifndef vm_h
#define vm_h

#include <sys/types.h>

int init_vm(const char *program, char *display);
void free_vm();
void step();
#endif // !vm_h
