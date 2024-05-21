#ifndef vm_h
#define vm_h

#include "types.h"
#define ZERO_KEY 0
#define ONE_KEY 1
#define TWO_KEY 2
#define THREE_KEY 3
#define FOUR_KEY 4
#define FIVE_KEY 5
#define SIX_KEY 6
#define SEVEN_KEY 7
#define EIGHT_KEY 8
#define NINE_KEY 9
#define A_KEY 10
#define B_KEY 11
#define C_KEY 12
#define D_KEY 13
#define E_KEY 14
#define F_KEY 15
#define NO_KEY 16

int init_vm(const char *program, char *display);
void free_vm();
void step(uint8 key_pressed);
#endif // !vm_h
