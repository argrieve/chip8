#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// 2 byte opcodes
uint16_t opcode;

// 4K memory (4096 bytes)
#define MEM_SIZE 4096
uint8_t mem[MEM_SIZE];
#define ROM_START_ADDR 0x200

// Stack can hold 16 addresses
#define STACK_SIZE 16
uint16_t stack[STACK_SIZE];
uint16_t reg_sp;

// CPU has 15 general purpose registers v0, v1, v2, ... vE
// Register 16 is the register holding the carry flag
#define NUM_REGS 16
uint8_t reg_v[NUM_REGS];

// The Index register and PC can have values from 0x000 - 0xFFF
uint16_t reg_i;
uint16_t reg_pc;

// Graphics buffer
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCREEN_SIZE 2048 // 64W x 32H
uint8_t gfx[SCREEN_SIZE];
uint8_t draw_flag;

// 8-bit hardware timers
// If their value is greater than 0, they count down at 60 Hz
uint8_t timer_delay;
uint8_t timer_sound;

// Hex keyboard with 16 keys, each with value from 0x0 - 0xF
#define NUM_KEYS 16
uint8_t keys[NUM_KEYS];

// Font set used with the CHIP-8
#define FONT_START_ADDR 0x50
#define FONT_SIZE 80
uint8_t font_set[FONT_SIZE] =
{ 
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};



/*
 * Public Functions
 */
int chip8_load_rom(char *rom);
void chip8_cycle();

/*
 * Private Functions
 */
void chip8_reset();
