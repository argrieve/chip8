#include "chip8.h"
#include "time.h"

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

				// 0x8XY5 - subtract VY from VX, set VF to 0 if there's a borrow, else 1
				case 0x0005:
					if (reg_v[(opcode & 0x00F0) >> 4] > reg_v[(opcode & 0x0F00) >> 8] )
						reg_v[0xF] = 0; // a borrow ocurred
					else
						reg_v[0xF] = 1;
					reg_v[(opcode & 0x0F00) >> 8] -= reg_v[(opcode & 0x00F0) >> 4];
					reg_pc += 2;
					break;

				// 0x8XY6 - shift VX right 1 bit, VF is set to LSB of VX before the shift
				case 0x0006:
					reg_v[0xF] = reg_v[(opcode & 0x0F00) >> 8] & 0x1;
					reg_v[(opcode & 0x0F00) >> 8] >>= 1;
					reg_pc += 2;
					break;

				// 0x8XY7 - set VX to VY-VX, set VF to 0 if there's a borrow, else 1
				case 0x0007:
					if (reg_v[(opcode & 0x0F00) >> 8] > reg_v[(opcode & 0x00F0) >> 4] )
						reg_v[0xF] = 0;
					else
						reg_v[0xF] = 1;
					reg_v[(opcode & 0x0F00) >> 8] = reg_v[(opcode & 0x00F0) >> 4] - reg_v[(opcode & 0x0F00) >> 8];
					reg_pc += 2;
					break;

				// 0x8XYE - shift VX left by 1 bit, VF is set to MSB of VX before the shift
				case 0x000E:
					reg_v[0xF] = reg_v[(opcode & 0x0F00) >> 8] >> 7;
					reg_v[(opcode & 0x0F00) >> 8] <<= 1;
					reg_pc += 2;
					break;

				default:
					printf("Unknown opcode 0x%x\n", opcode);
					break;
			}
			break;

		// 0x9XY0 - skip next instruction if VX != VY
		case 0x9000:
			if (reg_v[(opcode & 0x0F00) >> 8] != reg_v[(opcode & 0x00F0) >> 4] )
				reg_pc += 4;
			else
				reg_pc += 2;
			break;

		// 0xANNN - sets register I to address NNN
		case 0xA000:
			reg_i = opcode & 0x0FFF;
			reg_pc += 2;
			break;

		// 0xBNNN - jump to address NNN + V0
		case 0xB000:
			reg_pc = (opcode & 0x0FFF) + reg_v[0];	
			break;

		// 0xCXNN - sets VX to a random number AND NN
		case 0xC000:
			reg_v[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
			reg_pc += 2;
			break;

		// 0xDXYN - draw sprite at position VX, VY - this op is screwy, see comments below
		case 0xD000:
			{
				uint8_t x = reg_v[(opcode & 0x0F00) >> 8]; // starting x position
				uint8_t y = reg_v[(opcode & 0x00F0) >> 4]; // starting y position
				uint8_t height = opcode & 0x000F; // height of the sprite (number of rows)
				uint8_t pixel;
				// read N bytes from memory starting at location I
				// each byte read represents 8 pixels (each bit is a pixel)
				// set VF to 1 if any pixels get flipped
				reg_v[0xF] = 0;
				int xline, yline;
				for (yline = 0; yline < height; ++yline)
				{
					pixel = mem[reg_i + yline];
					for (xline = 0; xline < 8; ++xline)
					{
						if ((pixel & (0x80 >> xline)) != 0) 
						{
							int index = x + xline + ((y + yline) * SCREEN_WIDTH);
							if (gfx[index] == 1) reg_v[0xF] = 1;
							gfx[index] ^= 1;
						}
					}
				}
				draw_flag = 0;
				reg_pc += 2;
			}
			break;

		case 0xE000:
			switch (opcode & 0x0001)
			{
				// 0xEX9E - skip next instruction if key stored in VX is pressd 
				case 0x0000:
					if (keys[reg_v[(opcode & 0x0F00) >> 8]] != 0)
						reg_pc += 4;
					else
						reg_pc += 2;
					break;

				// 0xEXA1 - skip next instruction if key stored in VX isn't pressed
				case 0x0001:
					if (keys[reg_v[(opcode & 0x0F00) >> 8]] == 0)
						reg_pc += 4;
					else
						reg_pc += 2;
					break;

				default:
					printf("Unknown opcode 0x%x\n", opcode);
					break;
			}
			break;

		case 0xF000:
			switch (opcode & 0x00FF)
			{
				// 0xFX07 - set VX to the value of the delay timer
				case 0x0007:
					reg_v[(opcode & 0x0F00) >> 8] = timer_delay;
					reg_pc += 2;
					break;
				
				// 0xFX0A - a key press is awaited, then stored in VX
				case 0x000A:
					{
						// this is a hack, but should work
						uint8_t key_pressed = 0;
						int i;
						for (i = 0; i < NUM_KEYS; ++i)
						{
							if (keys[i] != 0)
							{
								reg_v[(opcode & 0x0F00) >> 8] = i;
								key_pressed = 1;
							}
						}
						if (!key_pressed) return; // we will try again on the next cycle
						reg_pc += 2;
					}	
					break;

				// 0xFX15 - sets the delay timer to VX
				case 0x0015:
					timer_delay = reg_v[(opcode & 0x0F00) >> 8];
					reg_pc += 2;
					break;

				// 0xFX18 - sets the sound timer to VX
				case 0x0018:
					timer_sound = reg_v[(opcode & 0x0F00) >> 8];
					reg_pc += 2;
					break;

				// 0xFX1E - add VX to I
				case 0x001E:
					// be sure to check for overflow!
					if ((reg_i + reg_v[(opcode & 0x0F00) >> 8]) > 0xFFF)
						reg_v[0xF] = 1;
					else
						reg_v[0xF] = 0;
					reg_i += reg_v[(opcode & 0x0F00) >> 8];
					reg_pc += 2;
					break;

				// 0xFX29 - set I to the location of the sprite for the character in VX. Characters 0-F (HEX) are represented by a 4x5 font 
				case 0x0029:
					reg_i = reg_v[(opcode & 0x0F00) >> 8] * 0x5;
					reg_pc += 2;
					break;

				// 0xFX33 - weird opcode. Take the decimal representation of VX and put the hundreds digit at I, tens digit at I+1, and ones digit at I+2
				case 0x0033:
					mem[reg_i] = reg_v[(opcode & 0x0F00) >> 8] / 100;
					mem[reg_i + 1] = (reg_v[(opcode & 0x0F00) >> 8] / 10) % 10;
					mem[reg_i + 2] = (reg_v[(opcode & 0x0F00) >> 8] % 100) % 10;
					reg_pc += 2;
					break;

				// 0xFX55 - store V0 to VX in memory starting at address I
				case 0x0055:
				{
					uint8_t end = (opcode & 0x0F00) >> 8;
					uint8_t i;
					for (i = 0; i <= end; ++i)
						mem[reg_i + i] = reg_v[i];

					// according to the wiki, the original interpreter sets I = I + X + 1
					reg_i += ((opcode & 0x0F00) >> 8) + 1;
					reg_pc += 2;
				}
				break;
				
				// 0xFX65 - fill V0 to VX with values starting at memory address I
				case 0x0065:
				{
					uint8_t end = (opcode & 0xF00) >> 8;
					uint8_t i;
					for (i = 0; i<= end; ++i)
						reg_v[i] = mem[reg_i + i];

					// according to the wiki, the original interpreter sets I = I + X + 1
					reg_i += ((opcode & 0x0F00) >> 8) + 1;
					reg_pc += 2;
				}
				break;

				default:
					printf("Unknown opcode 0x%x\n", opcode);
					break;
			}	
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
	
	// Reseed rand() function for a couple instructions
	srand(time(NULL));
}

int main()
{
	chip8_load_rom("games/tetris.c8");
	return 0;
}
