#include "vm.h"
#include "graphics.h"
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define PREFIX_0 0
#define PREFIX_1 1
#define PREFIX_2 2
#define PREFIX_3 3
#define PREFIX_4 4
#define PREFIX_5 5
#define PREFIX_6 6
#define PREFIX_7 7
#define PREFIX_9 9
#define PREFIX_A 10
#define PREFIX_B 11
#define PREFIX_C 12
#define PREFIX_D 13
#define PREFIX_E 14
#define PREFIX_F 15
#define CLEAR_SCREEN 0xE0
#define LEFT_MASK 15
#define MAX_SPRITE_HEIGHT 15

#define MAX_ADDRESS 4096
#define RESERVED_SPACE 512

typedef struct {
  unsigned char registers[16];
  unsigned short idx_reg;
  unsigned char delay_timer;
  unsigned char sound_timer;
  unsigned short pc;
  unsigned char *memory;
  char *display;
} vm;

vm echip;

int init_vm(const char *program, char *display) {
  FILE *fp = fopen(program, "rb");
  if (fp == NULL) {
    fprintf(stderr, "Program file %s not found\n", program);
    return -1;
  }
  struct stat stat_info;
  fstat(fileno(fp), &stat_info);
  size_t size = stat_info.st_size;
  if (size > MAX_ADDRESS - RESERVED_SPACE) {
    fprintf(stderr, "Program file %s is too large. Max size is %d bytes.\n",
            program, MAX_ADDRESS - RESERVED_SPACE);
    fclose(fp);
    return -1;
  }
  echip.memory = malloc(MAX_ADDRESS);
  size_t bytes_read = fread(echip.memory + RESERVED_SPACE, 1, size, fp);
  if (bytes_read != size) {
    free(echip.memory);
    fclose(fp);
    fprintf(stderr, "An error occurred while reading the program file.\n");
    return -1;
  }
  echip.pc = RESERVED_SPACE;
  echip.display = display;
  fclose(fp);
  return 0;
}

void free_vm() { free(echip.memory); }

void print_state() {
  for (int i = 0; i < 16; i++) {
    printf("Register V%x: %d\n", i, echip.registers[i]);
  }
  printf("Index Register: %x\n", echip.idx_reg);
  printf("Program Counter: %u\n", echip.pc);
}

void step() {
  unsigned char left_byte = echip.memory[echip.pc];
  unsigned char right_byte = echip.memory[echip.pc + 1];
  unsigned char first_nibble = left_byte >> 4;
  unsigned char second_nibble = left_byte & LEFT_MASK;
  printf("Left: %02x, Right: %02x, Full: %02x, Arg: %02x\n", first_nibble,
         second_nibble, left_byte, right_byte);
  switch (first_nibble) {
  case PREFIX_0: {
    if (right_byte == CLEAR_SCREEN) {
      printf("We have a clear screen instruction\n");
      clear_screen(echip.display);
    } else {
      printf("We have a return instruction\n");
    }
    break;
  }
  case PREFIX_1: {
    unsigned short address = (second_nibble << 8) + right_byte;
    printf("We need to jump to %03x\n", address);
    echip.pc = address;
    echip.pc -= 2;
    break;
  }
  case PREFIX_6: {
    printf("Setting register %d to value %02x\n", second_nibble, right_byte);
    echip.registers[second_nibble] = right_byte;
    break;
  }
  case PREFIX_7: {
    printf("Adding to register %d value %02x\n", second_nibble, right_byte);
    echip.registers[second_nibble] += right_byte;
    break;
  }
  case PREFIX_A: {
    unsigned short address = (second_nibble << 8) + right_byte;
    printf("Setting index register to value %03x\n", address);
    echip.idx_reg = address;
    break;
  }
  case PREFIX_D: {
    unsigned char x = echip.registers[second_nibble];
    unsigned char y = echip.registers[right_byte >> 4];
    unsigned char height = right_byte & LEFT_MASK;
    unsigned char sprite[MAX_SPRITE_HEIGHT];
    memcpy(sprite, echip.memory + echip.idx_reg, height);
    printf("Drawing sprite with height %d pixels at location (%d, %d)\n",
           height, x, y);
    draw_sprite(echip.display, x, y, sprite, height);
    break;
  }
  default: {
    printf("Unrecognized opcode\n");
    break;
  }
  }
  echip.pc += 2;
  print_state();
}
