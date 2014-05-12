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

void chip8_cycle()
{
	int i;

	// Fetch an opcode
	opcode = (mem[reg_pc] << 8) | mem[reg_pc + 1];
	
	// Decode and Execute
	switch (opcode & 0xF000)
	{
		case 0x0000:
			switch (opcode & 0x000F)
			{
				// 0x00E0 - clear the screen
				case 0x0000:
					for (i=0; i<SCREEN_SIZE; ++i)
						gfx[i] = 0;
					draw_flag = 1;
					reg_pc += 2;

				// 0x00EE - return from subroutine
				case 0x000E:
					--reg_sp;
					reg_pc = stack[reg_sp];
					reg_pc += 2; // PC must still be incremented!

				default:
					printf("Unknown opcode 0x%x\n", opcode);
			}
			break;

		// 0x1NNN - jumps to address NNN
		case 0x1000:
			reg_pc = opcode & 0x0FFF;	
			break;

		// 0x2NNN - call subroutine at address NNN
		case 0x2000:
			stack[reg_sp] = reg_pc;
			++reg_sp;
			reg_pc = opcode & 0x0FFF;
			break;

		// 0x3XNN - skip next instruction if VX = NN
		case 0x3000:
			if ( reg_v[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF) )	
				reg_pc += 4;
			else
				reg_pc += 2;
			break;

		// 0x4XNN - skip next instruction if VX != NN
		case 0x4000:
			if ( reg_v[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF) )	
				reg_pc += 4;
			else
				reg_pc += 2;
			break;

		// 0x5XY0 - skip next instruction if VX = VY
		case 0x5000:
			if ( reg_v[(opcode & 0x0F00) >> 8] == reg_v[(opcode & 0x00F0) >> 4] )
				reg_pc += 4;
			else
				reg_pc += 2;
			break;

		// 0x6XNN - set VX to NN
		case 0x6000:
			reg_v[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
			reg_pc += 2;
			break;

		// 0x7XNN - add NN to VX
		case 0x7000:
			reg_v[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);	
			reg_pc += 2;
			break;

		case 0x8000:
			switch (opcode & 0x000F)
			{
				// 0x8XY0 - set VX to the value of VY
				case 0x0000:
					reg_v[(opcode & 0x0F00) >> 8] = reg_v[(opcode & 0x00F0) >> 4];
					reg_pc += 2;
					break;

				// 0x8XY1 - set VX to (VX OR VY)
				case 0x0001:
					reg_v[(opcode & 0x0F00) >> 8] |= reg_v[(opcode & 0x00F0) >> 4];
					reg_pc += 2;
					break;

				// 0x8XY2 - set VX to (VX AND VY)
				case 0x0002:
					reg_v[(opcode & 0x0F00) >> 8] &= reg_v[(opcode & 0x00F0) >> 4];
					reg_pc += 2;
					break;

				// 0x8XY3 - set VX to (VX XOR VY)
				case 0x0003:
					reg_v[(opcode & 0x0F00) >> 8] ^= reg_v[(opcode & 0x00F0) >> 4];
					reg_pc += 2;
					break;

				// 0x8XY4 - add VY to VX, set VF to 1 if there's a carry else to 0
				case 0x0004:
					if (reg_v[(opcode & 0x00F0) >> 4] > (0xFF - reg_v[(opcode & 0x0F00) >> 8]) )	
						reg_v[0xF] = 1;
					else 
						reg_v[0xF] = 0;
					reg_v[(opcode & 0x0F00) >> 8] += reg_v[(opcode & 0x00F0) >> 4];
					reg_pc += 2;
					break;

				case 0x0005:
					
					break;

				case 0x0006:
					
					break;

				case 0x0007:
					
					break;

				case 0x000E:
					
					break;

				default:

					break;
			}
			break;

		case 0x9000:
			
			break;

		case 0xA000:
			
			break;

		case 0xB000:
			
			break;

		case 0xC000:
			
			break;

		case 0xD000:
			
			break;

		case 0xE000:
			
			break;

		case 0xF000:
			
			break;

	}


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
