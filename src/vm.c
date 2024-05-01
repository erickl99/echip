#include "vm.h"
#include "graphics.h"
#include "types.h"
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
#define PREFIX_8 8
#define PREFIX_9 9
#define PREFIX_A 10
#define PREFIX_B 11
#define PREFIX_C 12
#define PREFIX_D 13
#define PREFIX_E 14
#define PREFIX_F 15
#define SET_XY 0
#define BIN_OR 1
#define BIN_AND 2
#define BIN_XOR 3
#define ADD_XY 4
#define SUBTRACT_XY 5
#define RIGHT_SHIFT 6
#define SUBTRACT_YX 7
#define LEFT_SHIFT 14
#define CHECK_KEY 158
#define NOT_CHECK_KEY 161
#define CLEAR_SCREEN 0xE0
#define SET_XDELAY 7
#define SET_DELAYX 21
#define SET_SOUND 24
#define INDEX_ADD 30
#define GET_KEY 10
#define GET_FONT 41
#define BIN_TO_DEC 51
#define STORE_REG 85
#define LOAD_REG 101
#define LEFT_MASK 15
#define MAX_SPRITE_HEIGHT 15
#define CARRY_REGISTER 15
#define NO_KEY 16

#define MAX_ADDRESS 4096
#define RESERVED_SPACE 512

typedef struct {
  uint8 registers[16];
  mem_addr idx_reg;
  mem_addr pc;
  uint8 delay_timer;
  uint8 sound_timer;
  unsigned char key_pressed;
  mem_addr *stack;
  uint8 *memory;
  char *display;
} vm;

vm echip;

uint8 font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

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
  memcpy(echip.memory + 80, font, 80);
  echip.stack = (mem_addr *)(echip.memory + 160);
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
  uint8 left_byte = echip.memory[echip.pc];
  uint8 right_byte = echip.memory[echip.pc + 1];
  uint8 first_nibble = left_byte >> 4;
  uint8 second_nibble = left_byte & LEFT_MASK;
  printf("Left: %02x, Right: %02x, Full: %02x, Arg: %02x\n", first_nibble,
         second_nibble, left_byte, right_byte);
  switch (first_nibble) {
  case PREFIX_0: {
    if (right_byte == CLEAR_SCREEN) {
      printf("Clearing the screen\n");
      clear_screen(echip.display);
    } else {
      echip.pc = *echip.stack;
      echip.pc -= 2;
      echip.stack--;
      printf("We have a return instruction\n");
    }
    break;
  }
  case PREFIX_1: {
    mem_addr address = (second_nibble << 8) + right_byte;
    printf("Jumping to %03x\n", address);
    echip.pc = address;
    echip.pc -= 2;
    break;
  }
  case PREFIX_2: {
    mem_addr address = (second_nibble << 8) + right_byte;
    *echip.stack = echip.pc;
    echip.stack++;
    echip.pc = address;
    echip.pc -= 2;
    break;
  }
  case PREFIX_3: {
    if (right_byte == echip.registers[second_nibble]) {
      echip.pc += 2;
    }
    break;
  }
  case PREFIX_4: {
    if (right_byte != echip.registers[second_nibble]) {
      echip.pc += 2;
    }
    break;
  }
  case PREFIX_5: {
    uint8 third_nibble = right_byte >> 4;
    if (echip.registers[second_nibble] != echip.registers[third_nibble]) {
      echip.pc += 2;
    }
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
  case PREFIX_8: {
    uint8 third_nibble = right_byte >> 4;
    uint8 fourth_nibble = right_byte & LEFT_MASK;
    switch (fourth_nibble) {
    case SET_XY: {
      echip.registers[second_nibble] = echip.registers[third_nibble];
      break;
    }
    case BIN_OR: {
      echip.registers[second_nibble] |= echip.registers[third_nibble];
      break;
    }
    case BIN_AND: {
      echip.registers[second_nibble] &= echip.registers[third_nibble];
      break;
    }
    case BIN_XOR: {
      echip.registers[second_nibble] ^= echip.registers[third_nibble];
      break;
    }
    case ADD_XY: {
      if (255 - echip.registers[third_nibble] <
          echip.registers[second_nibble]) {
        echip.registers[CARRY_REGISTER] = 1;
      } else {
        echip.registers[CARRY_REGISTER] = 0;
      }
      echip.registers[second_nibble] += echip.registers[third_nibble];
      break;
    }
    case SUBTRACT_XY: {
      if (echip.registers[second_nibble] >= echip.registers[third_nibble]) {
        echip.registers[CARRY_REGISTER] = 1;
      } else {
        echip.registers[CARRY_REGISTER] = 0;
      }
      echip.registers[second_nibble] -= echip.registers[third_nibble];
      break;
    }
    // May need to provide option to configure behavior of this instruction
    case RIGHT_SHIFT: {
      uint8 value = echip.registers[third_nibble];
      echip.registers[CARRY_REGISTER] = value & 1;
      echip.registers[second_nibble] = value >> 1;
      break;
    }
    case SUBTRACT_YX: {
      if (echip.registers[third_nibble] >= echip.registers[second_nibble]) {
        echip.registers[CARRY_REGISTER] = 1;
      } else {
        echip.registers[CARRY_REGISTER] = 0;
      }
      echip.registers[second_nibble] =
          echip.registers[third_nibble] - echip.registers[second_nibble];
      break;
    }
    // May need to provide option to configure behavior of this instruction
    case LEFT_SHIFT: {
      uint8 value = echip.registers[third_nibble];
      echip.registers[CARRY_REGISTER] = value & 128;
      echip.registers[second_nibble] = value << 1;
      break;
    }
    }
    break;
  }
  case PREFIX_9: {
    uint8 third_nibble = right_byte >> 4;
    if (echip.registers[second_nibble] != echip.registers[third_nibble]) {
      echip.pc += 2;
    }
    break;
  }
  case PREFIX_A: {
    mem_addr address = (second_nibble << 8) + right_byte;
    printf("Setting index register to value %03x\n", address);
    echip.idx_reg = address;
    break;
  }
  // May need to provide option to configure behavior of this instruction
  case PREFIX_B: {
    mem_addr address = (second_nibble << 8) + right_byte + echip.registers[0];
    echip.pc = address;
    echip.pc -= 2;
    break;
  }
  case PREFIX_C: {
    uint8 value = random() % 255;
    echip.registers[second_nibble] = value & right_byte;
    break;
  }
  case PREFIX_D: {
    uint8 x = echip.registers[second_nibble];
    uint8 y = echip.registers[right_byte >> 4];
    uint8 height = right_byte & LEFT_MASK;
    uint8 sprite[MAX_SPRITE_HEIGHT];
    memcpy(sprite, echip.memory + echip.idx_reg, height);
    printf("Drawing sprite with height %d pixels at location (%d, %d)\n",
           height, x, y);
    draw_sprite(echip.display, x, y, sprite, height);
    break;
  }
  case PREFIX_E: {
    unsigned char key = echip.registers[second_nibble];
    if (right_byte == CHECK_KEY && key == echip.key_pressed) {
      echip.pc += 2;
    } else if (right_byte == NOT_CHECK_KEY && key != echip.key_pressed) {
      echip.pc += 2;
    }
    break;
  }
  case PREFIX_F: {
    switch (right_byte) {
    case SET_XDELAY:
      echip.registers[second_nibble] = echip.delay_timer;
      break;
    case SET_DELAYX:
      echip.delay_timer = echip.registers[second_nibble];
      break;
    case SET_SOUND:
      echip.sound_timer = echip.registers[second_nibble];
      break;
    case INDEX_ADD:
      echip.idx_reg += echip.registers[second_nibble];
      break;
    case GET_KEY:
      if (echip.key_pressed == NO_KEY) {
        echip.pc -= 2;
      } else {
        echip.registers[second_nibble] = echip.key_pressed;
      }
      break;
    case GET_FONT:
      echip.idx_reg = 80 + echip.registers[second_nibble];
      break;
    case BIN_TO_DEC: {
      uint8 value = echip.registers[second_nibble];
      uint8 ones = value % 10;
      echip.memory[echip.idx_reg + 2] = ones;
      value = (value - ones) / 10;
      uint8 tens = value % 10;
      echip.memory[echip.idx_reg + 1] = tens;
      value = (value - tens) / 10;
      uint8 hundreds = value % 10;
      echip.memory[echip.idx_reg] = hundreds;
      break;
    }
    // Some older games may rely on the index register being updated
    case STORE_REG:
      for (uint8 i = 0; i <= second_nibble; i++) {
        echip.memory[echip.idx_reg + i] = echip.registers[i];
      }
      break;
    // Some older games may rely on the index register being updated
    case LOAD_REG:
      for (uint8 i = 0; i <= second_nibble; i++) {
        echip.registers[i] = echip.memory[echip.idx_reg + i];
      }
      break;
    }
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
