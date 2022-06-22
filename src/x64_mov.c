#include <stdio.h>
#include <string.h>

#include "../inc/macro.h"
#include "../inc/cdata.h"
#include "../inc/parse.h"
#include "../inc/error.h"

#include "../inc/x64.h"


/*
	R - Register    reg
	I - Immediate   imm
	P - Pointer     [reg]
	
	Y - ArraY       [reg + imm * off]
	M - Memory      [reg * off]
	C - Char array  [reg + imm]
	A - Address     [imm]
	
	MAD - Multiply-Add [imm + (reg * off)]
*/

// mov reg, reg
int x64_mov_rr(struct compiledata *cdata, FILE *fout, const char *args, char type, unsigned char size)
{
	// Initialize tokens
	char token[2][STRING_LENGTH_TOKEN];
	
	for (unsigned int i = 0; i < 2; ++i)
		if (strstrip(strltok(token[i], args, NULL, ",", i)) == NULL)
			return 1;

	// Initialize register values
	int 
	regv[2] = {0},  	// Register values
	f_rex[2] = {0}, 	// REX flags
	f_seg[2] = {0}, 	// SEG flags
	f_crx[2] = {0}; 	// CRX flags
	
	// Check for registers
	for (unsigned int i = 0; i < 2; ++i)
	{
		regv[i] = x64_registerid(token[i]);
		
		if (regv[i] == -1)
		{
			fprintf(stderr, "\nError (Line %u): Invalid register '%s'.\n---\n", cdata->c_linenum, token[i]);
			return 1;
		}
		
		// Determine if CRX/SEG/REX flagged
		switch (regv[i] / 8)
		{
			// REX check
			// CRX check
			case REGISTER_X64_GROUP_CRX:
				f_crx[i] = 1;
				regv[i] %= 8;
				break;
			
			// SEG check
			case REGISTER_X64_GROUP_SEG:
				f_seg[i] = 1;
				regv[i] %= 8;
				break;
			
			case REGISTER_X64_GROUP_REX:
				f_rex[i] = 1;
				regv[i] %= 8;
				break;
		}
	}
	
	// Check if valid operands
	{
		// Segment registers only load from 16-bit 
		if ((size != 2) && f_seg[0])
		{
			fprintf(stderr, "\nError (Line %u): x86 segment registers can only load 16 bits of data.\n---\n", cdata->c_linenum);
			return 1;
		}
		
		// 8-bit registers cannot load from segment registers
		if ((size == 1) && f_seg[1])
		{
			fprintf(stderr, "\nError (Line %u): x86 segment registers cannot store to 8 bits of data.\n---\n", cdata->c_linenum);
			return 1;
		}
		
		// You may not store to CS register
		if (regv[0] == REGISTER_X64_CS)
		{
			fprintf(stderr, "\nError (Line %u): You cannot use 'mov' to load the CS register. Use 'jmp', 'call', or 'ret' instead.\n---\n", cdata->c_linenum);
			return 1;
		}
		
		// 64-bit REX registers is incompatible with CRX registers
		if ((size < 8) && (f_rex[0] || f_rex[1]) && (f_crx[0] || f_crx[1]))
		{
			fprintf(stderr, "\nError (Line %u): x86 REX registers are incompatible with control registers less than 64 bits.\n---\n", cdata->c_linenum);
			return 1;
		}
	}
	
	// Generate binary instructions
	switch (size)
	{
		case 8:
			switch (type)
			{
				case 'i':
				case 'u':
					// REX prefix
					if (f_rex[0] || f_rex[1])
					{
						if (f_crx[0] || f_crx[1])
							fputc(0x41 + (f_seg[1] * 8), fout);
						else fputc(0x48 + f_rex[0] + (f_rex[1] * 4), fout);
					}
					
					else if (!(f_crx[0] || f_crx[1]))
						fputc(0x48 + f_rex[0] + (f_rex[1] * 4), fout);
					
					// CRX prefix
					if (f_crx[0] || f_crx[1]) fputc(0x0f, fout);
					
					// Opcode
					if (f_seg[1]) fputc(0x8c, fout); else
					if (f_crx[0]) fputc(0x22, fout); else
					if (f_crx[1]) fputc(0x20, fout);
					else          fputc(0x89, fout);
					
					// Operands
					if (f_seg[0] || f_seg[1]) fputc(0xc0 + regv[1] + (regv[0] * 8), fout); // Unconfirmed
					else                      fputc(0xc0 + regv[0] + (regv[1] * 8), fout);
					
					break;
				
				case 'p':
					break;
			}
			break;
		
		case 4:
			switch (type)
			{
				case 'i':
				case 'u':
					
					// REX prefix
					if (f_rex[0] || f_rex[1]) 
						fputc(0x40 + f_rex[0] + (f_rex[1] * 4) + (f_seg[1] * 8), fout);
					
					// Other prefixes
					if (f_crx[0] || f_crx[1]) fputc(0x0f, fout);
					
					// Opcode
					if (f_seg[1]) fputc(0x8c, fout); else
					if (f_crx[0]) fputc(0x22, fout); else
					if (f_crx[1]) fputc(0x20, fout);
					else          fputc(0x89, fout);
					
					// Operands
					fputc(0xc0 + regv[0] + (regv[1] * 8), fout);
					
					break;
				
				case 'p': 
					break;
			}
			break;
		
		case 2:
			break;
	}
	return 0;
}

// mov reg, imm
int x64_mov_ri(struct compiledata *cdata, FILE *fout, const char *args, char type, unsigned char size)
{
	//char token[2][STRING_LENGTH_TOKEN];
	
	//switch (type)
	return 0;
}

// mov reg, [reg]
int x64_mov_rp(struct compiledata *cdata, FILE *fout, const char *args, char type, unsigned char size)
{
	return 0;
}

// mov reg, [reg+imm]
int x64_mov_ry(struct compiledata *cdata, FILE *fout, const char *args, char type, unsigned char size)
{
	return 0;
}

// mov reg, [reg*off]
int x64_mov_rm(struct compiledata *cdata, FILE *fout, const char *args, char type, unsigned char size)
{
	return 0;
}

// mov reg, [reg*off+imm]
int x64_mov_rc(struct compiledata *cdata, FILE *fout, const char *args, char type, unsigned char size)
{
	return 0;
}

// mov reg, [imm]
int x64_mov_ra(struct compiledata *cdata, FILE *fout, const char *args, char type, unsigned char size)
{
	return 0;
}

int x64_mov_main(struct compiledata *cdata, FILE *fout, const char *args, char type, char sizec, char *conf)
{
	// Setup size
	unsigned char size;
	
	// Check if 4th character is valid
	if (sizec - '0' <= 9)
	{
		// Check if power of 2
		if (powcheck(sizec - '0', 2)) size = sizec - '0';
		else
		{
			throw_exception
			(
				EXCEPTION_ASSEMBLY_INVALID_OPERAND_SIZE,
				NULL, cdata->c_linenum, sizec
			);
			
			//fprintf(stderr, "\nError (Line %u): Invalid definition of values '%c%c'\n---\n", cdata->c_linenum, type, sizec);
			return 1;
		}
	}
	else switch (sizec)
	{
		case 'x': size = 16; break;
		case 'y': size = 32; break;
		case 'z': size = 64; break;
		default: 
			fprintf(stderr, "\nError (Line %u): Invalid definition of values '%c%c'\n---\n", cdata->c_linenum, type, sizec);
			break;
	}
	
	// Determine argument format
	int status = 0;
	
	if (!strcmp(conf, "rr")) status = x64_mov_rr(cdata, fout, args, type, size); else
	if (!strcmp(conf, "ri")) status = x64_mov_ri(cdata, fout, args, type, size);
	else
	{
		status = 1;
		fprintf(stderr, "\nError: Operand configuration '%s' not recognized in instruction '%s'.\n---\n", conf, "mov");
	}
	return status;
}