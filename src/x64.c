#include <string.h>

#include "../inc/macro.h"
#include "../inc/cdata.h"
#include "../inc/parse.h"
#include "../inc/main.h"

#include "../inc/x64.h"

/*
	R - Register    reg
	I - Immediate   imm
	P - Pointer     [reg]
	A - Address     [imm]
	
	C - Char array  [reg + imm]
	M - Memory      [reg * off]
	Y - ArraY       [reg + imm * off]
	
	MAD - Multiply-Add [imm + (reg * off)]
*/

// Return assembly register ID
int x64_registerid(const char *s)
{
	int chval = 0;
	
	// Get characters to make compatible with switch-case
	for (unsigned int i = 0, l = strlen(s); i < l; ++i)
		chval += (s[l - i - 1] << (8 * i));
	
	// NOTE: Multichar constants generate warnings, even when casted to integers.
	// ...so instead, I used hexes for the case values. Not ideal for readability.
	switch (chval)
	{
		/*
			The comments below are guides to appropriate char -> hex conversions.
			DO NOT DELETE THIS COMMENT UNTIL THE CASE VALUES:
			- Are multi-char values (i.e. 'ax')
			- Do not generate any warnings
			
			30 --> 0
			61 --> a
			6a --> j
			71 --> q
		*/
		
		case 0x6178: return REGISTER_X64_AX;
		case 0x6378: return REGISTER_X64_CX;
		case 0x6478: return REGISTER_X64_DX;
		case 0x6278: return REGISTER_X64_BX;
		case 0x7370: return REGISTER_X64_SP;
		case 0x6270: return REGISTER_X64_BP;
		case 0x7369: return REGISTER_X64_SI;
		case 0x6469: return REGISTER_X64_DI;
		
		case 0x7238: return REGISTER_X64_R8;
		case 0x7239: return REGISTER_X64_R9;
		case 0x7261: return REGISTER_X64_RA;
		case 0x7262: return REGISTER_X64_RB;
		case 0x7263: return REGISTER_X64_RC;
		case 0x7264: return REGISTER_X64_RD;
		case 0x7265: return REGISTER_X64_RE;
		case 0x7266: return REGISTER_X64_RF;
		/*
		case 0x6573: return REGISTER_X64_ES;
		case 0x6373: return REGISTER_X64_CS;
		case 0x7373: return REGISTER_X64_SS;
		case 0x6473: return REGISTER_X64_DS;
		case 0x6673: return REGISTER_X64_FS;
		case 0x6773: return REGISTER_X64_GS;
		
		case 0x637230: return REGISTER_X64_CR0;
		case 0x637231: return REGISTER_X64_CR1;
		case 0x637232: return REGISTER_X64_CR2;
		case 0x637233: return REGISTER_X64_CR3;
		case 0x637234: return REGISTER_X64_CR4;
		*/
		
		case 0x6630: return REGISTER_X64_F0;
		case 0x6631: return REGISTER_X64_F1;
		case 0x6632: return REGISTER_X64_F2;
		case 0x6633: return REGISTER_X64_F3;
		case 0x6634: return REGISTER_X64_F4;
		case 0x6635: return REGISTER_X64_F5;
		case 0x6636: return REGISTER_X64_F6;
		case 0x6637: return REGISTER_X64_F7;
		
		case 0x6638: return REGISTER_X64_F8;
		case 0x6639: return REGISTER_X64_F9;
		case 0x6661: return REGISTER_X64_FA;
		case 0x6662: return REGISTER_X64_FB;
		case 0x6663: return REGISTER_X64_FC;
		case 0x6664: return REGISTER_X64_FD;
		case 0x6665: return REGISTER_X64_FE;
		case 0x6666: return REGISTER_X64_FF;
		
		
		// Return failure
		default: return -1;
	}
}

// Main function for x64 assembly
int x64_proc_main(struct compiledata *cdata, FILE *fout, char *line)
{
	// Initialize tokens for keyword checking
	char token[3][STRING_LENGTH_TOKEN];
	
	for (unsigned int i = 0; i < 3; ++i)
	if (strltok(token[i], line, NULL, " \t", i) == NULL)
		return 1;
	
	// Initialize char pointer for arguments
	line = stritok(line, NULL, " \t", 3);
	
	// Determine instruction to be generated
	int status = 0;
	
	if (!strcmp(token[0], "mov")) status = x64_mov_main(cdata, fout, line, token[1][0], token[1][1], token[2]); else
	if (!strcmp(token[0], "get")) status = x64_get_main(cdata, fout, line, token[1][0], token[1][1], token[2]); else
	
	return 1;
	
	/*
	
	Why the negation of the status value?
	This is because at this point, the function has determined the following:
	
		1. A valid instruction keyword is used
		2. The arguments used are invalid
	
	Returning 1 signals the 'kwordproc' function back in 'main.c' that no valid
	instruction keyword was found, so that it could then proceed with finding
	its own provided non-instruction keywords.
	
	If the negation wasn't used, the return value would either be 1 or 0.
	Since we know that a valid instruction keyword has already been found,
	returning 1 doesn't report an accurate status for the kwordproc function,
	and thus, would continue its operations, wasting away execution time.
	
	What should I do then? Perhaps you might think that I should return -1 for
	the 'no-instruction-found' status while zealously defending return 1 as the
	sacred, one-size-fits-all return value for errors, right?
	
	Wrong. If I were to return -1 for every single return code, I might either 
	deal with having an unnecessary mental overhead relating to return values,
	or worse, cultivate this habit of returning -1, out of convenience, leading
	to consistent usage of this bad practice (in my opinion).
	
	For this reason, if the operation fails, there should be another value that
	represents the error status.
	
	Returning -1, in that case, would mean that something went wrong in the
	process of determining the exact bytes to be written to the output file
	that accurately represents the instruction provided.
	
	TL;DR:
	return  1 = there is just no valid instruction found on the current line.
	return -1 = there is a valid instruction, but then an invalid argument was used.
	            therefore, there is an error.
	
	*/
	return -status;
}
