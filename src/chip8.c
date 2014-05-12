#include "chip8.h"

int chip8_load_rom(char *rom)
{
	chip8_reset();

	// Open the ROM file
	FILE *fp = fopen(rom, "r");
	if (fp == NULL) {
		printf("chip8_load_rom(): fopen() failed.\n");
		return 1;
	}

	// Check it's size, make sure it will fit in memory
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	rewind(fp);
	if (size > (MEM_SIZE-ROM_START_ADDR) ) {
		printf("chip8_load_rom(): ROM file too large.\n");
		return 1;
	}

	// Copy the ROM into memory
	uint8_t *ptr = mem;
	ptr += ROM_START_ADDR;
	int ret = fread(ptr, 1, size, fp);
	if (ret != size) {
		printf("chip8_load_rom(): fread() failed.\n");
		return 1;
	}
	
	return 0;
}

void chip8_reset()
{
	int i;
	reg_pc = ROM_START_ADDR; // Reset the program counter
	opcode = 0; // Clear the opcode
	reg_i = 0;	// Reset the Index register
	reg_sp = 0;	// Reset the stack pointer

	// Clear the graphics buffer
	for (i=0; i<SCREEN_SIZE; ++i)
		gfx[i] = 0;
	
	// Clear the stack
	for (i=0; i<STACK_SIZE; ++i) 
		stack[i] = 0;

	// Clear the keys
	for (i=0; i<NUM_KEYS; ++i)
		keys[i] = 0;

	// Clear the memory
	for (i=0; i<MEM_SIZE; ++i)
		mem[i] = 0;

	// Load the font set
	for (i=0; i<FONT_SIZE; ++i)
		mem[FONT_SIZE + i] = font_set[i];

	// Clear timers, draw flag
	timer_delay = 0;
	timer_sound = 0;
	draw_flag = 0;
}

int main()
{
	chip8_load_rom("games/tetris.c8");
	return 0;
}
