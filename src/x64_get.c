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
int x64_get_rr(struct compiledata *cdata, FILE *fout, const char *args, char type, unsigned char size)
{
	// Initialize tokens
	char token[2][STRING_LENGTH_TOKEN];
	
	
	for (unsigned int i = 0; i < 2; ++i)
		if (strstrip(strltok(token[i], args, NULL, ",", i)) == NULL)
			return 1;

	// Initialize register values
	int 
	regv [2] = {0}, 	// Register values
	f_fpt[2] = {0}, 	// Floating point flags
	f_rex[2] = {0}; 	// REX flags
	
	// Check for registers
	for (unsigned int i = 0; i < 2; ++i)
	{
		regv[i] = x64_registerid(token[i]);
		
		if (regv[i] == -1)
		{
			throw_exception(EXCEPTION_ASSEMBLY_INVALID_REGISTER, NULL, cdata->c_linenum, token[i]);
			return 1;
		}
		
		// Determine if REX extended
		if ((regv[i] / 8) == REGISTER_X64_GROUP_REX)
		{
			f_rex[i] = 1;
			regv[i] %= REGISTER_X64_GROUP_REX * 8;
		}
		
		// Determine if float
		if ((regv[i] / 8) == REGISTER_X64_GROUP_FGP)
		{
			f_fpt[i] = 1;
			regv[i] %= REGISTER_X64_GROUP_FGP * 8;
		}
		
		// Determine if float and REX extended
		if ((regv[i] / 8) == REGISTER_X64_GROUP_FGPX)
		{
			f_fpt[i] = 1;
			f_rex[i] = 1;
			regv[i] %= REGISTER_X64_GROUP_FGPX * 8;
		}
	}
	
	// Check if valid operands
	{
		// Check if type mismatch between float defined type and integer operand
		if (((type == 'p') && !f_fpt[0])
		||  ((type == 'i') &&  f_fpt[0]))
		{
			throw_exception(EXCEPTION_ASSEMBLY_INVALID_OPERAND_TYPE, NULL, cdata->c_linenum, type);
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
					
					// Check if float source operand
					if (f_fpt[1])
					{
						// Opcode 1
						fputc(0xf3, fout);
						
						// REX infix
						fputc(0x48 + f_rex[1] + (f_rex[0] * 4), fout);
						
						// Opcode 2
						fputc(0x0f, fout);
						fputc(0x2d, fout);
						
						// Operands
						fputc(0xc0 + (regv[0] * 8) + regv[1], fout);
					}
					else
					{
						// REX prefix
						fputc(0x48 + f_rex[0] + (f_rex[1] * 4), fout);
						
						// Opcode
						fputc(0x89, fout);
						
						// Operands
						fputc(0xc0 + regv[0] + (regv[1] * 8), fout);
					}
					
					
					break;
				
				case 'p':
					if (f_fpt[0] && f_fpt[1])
					{
						// Opcode 1
						fputc(0xf2, fout);
						
						// REX infix
						if (f_rex[0] || f_rex[1])
							fputc(0x40 + (f_rex[0] * 4) + f_rex[1], fout);
						
						// Opcode 2
						fputc(0x0f, fout);
						fputc(0x10, fout);
						
						// Operands
						fputc(0xc0 + (regv[0] * 8) + regv[1], fout);
					}
					else if (f_fpt[0])
					{
						// Opcode 1
						fputc(0xf2, fout);
						
						// REX infix
						fputc(0x48 + (f_rex[0] * 4) + f_rex[1], fout);
						
						// Opcode 2
						fputc(0x0f, fout);
						fputc(0x2a, fout);
						
						// Operands
						fputc(0xc0 + (regv[0] * 8) + regv[1], fout);
					}
					
					break;
			}
			break;
		
		case 4:
			switch (type)
			{
				case 'i':
				case 'u':
					
					// Check if float source operand
					if (f_fpt[1])
					{
						// Opcode 1
						fputc(0xf3, fout);
						
						// REX infix
						if (f_rex[0] || f_rex[1])
							fputc(0x40 + (f_rex[0] * 4) + f_rex[1], fout);
						
						// Opcode 2
						fputc(0x0f, fout);
						fputc(0x2d, fout);
						
						// Operands
						fputc(0xc0 + (regv[0] * 8) + regv[1], fout);
					}
					else
					{
						// REX prefix
						if (f_rex[0] || f_rex[1]) 
							fputc(0x40 + f_rex[0] + (f_rex[1] * 4), fout);
						
						// Opcode
						fputc(0x89, fout);
						
						// Operands
						fputc(0xc0 + regv[0] + (regv[1] * 8), fout);
					}
					
					break;
				
				case 'p': 
					if (f_fpt[0] && f_fpt[1])
					{
						// Opcode 1
						fputc(0xf3, fout);
						
						// REX infix
						if (f_rex[0] || f_rex[1])
							fputc(0x40 + (f_rex[0] * 4) + f_rex[1], fout);
						
						// Opcode 2
						fputc(0x0f, fout);
						fputc(0x10, fout);
						
						// Operands
						fputc(0xc0 + regv[1] + (regv[0] * 8), fout);
					}
					else if (f_fpt[0])
					{
						// Opcode 1
						fputc(0xf3, fout);
						
						// REX infix
						if (f_rex[0] || f_rex[1])
							fputc(0x40 + (f_rex[0] * 4) + f_rex[1], fout);
						
						// Opcode 2
						fputc(0x0f, fout);
						fputc(0x2a, fout);
						
						// Operands
						fputc(0xc0 + (regv[0] * 8) + regv[1], fout);
					}
					
					break;
			}
			break;
		
		case 2:
			break;
	}
	return 0;
}

// mov reg, imm
int x64_get_ri(struct compiledata *cdata, FILE *fout, const char *args, char type, unsigned char size)
{
	//char token[2][STRING_LENGTH_TOKEN];
	
	//switch (type)
	return 0;
}

// mov reg, [reg]
int x64_get_rp(struct compiledata *cdata, FILE *fout, const char *args, char type, unsigned char size)
{
	return 0;
}

// mov reg, [reg+imm]
int x64_get_ry(struct compiledata *cdata, FILE *fout, const char *args, char type, unsigned char size)
{
	return 0;
}

// mov reg, [reg*off]
int x64_get_rm(struct compiledata *cdata, FILE *fout, const char *args, char type, unsigned char size)
{
	return 0;
}

// mov reg, [reg*off+imm]
int x64_get_rc(struct compiledata *cdata, FILE *fout, const char *args, char type, unsigned char size)
{
	return 0;
}

// mov reg, [imm]
int x64_get_ra(struct compiledata *cdata, FILE *fout, const char *args, char type, unsigned char size)
{
	return 0;
}

int x64_get_main(struct compiledata *cdata, FILE *fout, const char *args, char type, char sizec, char *conf)
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
	
	if (!strcmp(conf, "rr")) status = x64_get_rr(cdata, fout, args, type, size); else
	if (!strcmp(conf, "ri")) status = x64_get_ri(cdata, fout, args, type, size);
	else
	{
		status = 1;
		fprintf(stderr, "\nError: Operand configuration '%s' not recognized in instruction '%s'.\n---\n", conf, "get");
	}
	return status;
}