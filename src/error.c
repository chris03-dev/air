#include <stdio.h>

#include "../inc/cdata.h"
#include "../inc/error.h"

void throw_exception(int code, int *boolptr, ...)
{
	va_list args;
	va_start(args, boolptr);
	
	// Check for exception codes
	switch (code)
	{
		// File access exceptions
		case EXCEPTION_FILE_NO_CREATE:
			vfprintf(stderr, "\nError: '%s' cannot create files. Please check permissions for this program.\n---\n", args);
			break;
		case EXCEPTION_FILE_NO_ACCESS:
			vfprintf(stderr, "\nError: File '%s' cannot be read, or does not exist.\n---\n", args);
			break;
		
		// Compiler flag exceptions
		case EXCEPTION_FLAG_INVALID:
			vfprintf(stderr, "\nWarning: Invalid flag '%s'.\n---\n", args);
			break;
		case EXCEPTION_FLAG_NO_VALUE:
			vfprintf(stderr, "\nWarning: No value for flag '%s'.\n---\n", args);
			break;
		case EXCEPTION_FLAG_INVALID_VALUE:
			vfprintf(stderr, "\nWarning: Invalid value '%s' for flag '%s'.\n---\n", args);
			break;
		
		// Limits exceptions
		case EXCEPTION_LIMIT_SIZE_TYPE:
			vfprintf(stderr, "\nWarning (Line %u): Value '0x%llx' is larger than defined size of %i bytes.\n---\n", args);
			break;
		case EXCEPTION_LIMIT_PASS_COUNT:
			vfprintf(stderr, "\nError: Exceeded pass limit of assembler (maximum is %u).\n---\n", args);
			break;
		
		// Assembly opcode exceptions
		case EXCEPTION_ASSEMBLY_INVALID_OPERAND_TYPE:
			vfprintf(stderr, "\nError (Line %u): Invalid opcode type '%c'.\n---\n", args);
			break;
		case EXCEPTION_ASSEMBLY_INVALID_OPERAND_SIZE:
			vfprintf(stderr, "\nError (Line %u): Invalid opcode size '%c'.\n---\n", args);
			break;
		case EXCEPTION_ASSEMBLY_INVALID_REGISTER:
			vfprintf(stderr, "\nError (Line %u): Invalid register '%s'.\n---\n", args);
			break;
		
		// Node process exceptions
		case EXCEPTION_TOKEN_INVALID_OPERATOR:
			vfprintf(stderr, "\nError (Line %u): Invalid operator '%s'.\n---\n", args);
			break;
		case EXCEPTION_TOKEN_UNDEFINED:
			vfprintf(stderr, "\nError (Line %u): Invalid token '%s' (still undefined after pass).\n---\n", args);
			break;
		
		// Miscellaneous
		case EXCEPTION_EXECUTION_ERROR:
			vfprintf(stderr, "\nError: '%s' finished execution with errors.\n---\n", args);
		case EXCEPTION_DEVELOPER_ERROR:
			vfprintf(stderr, "\nError: You may have broken something in the source code.\nDebug '%s', in function '%s'.\n---\n", args);
			break;
		
		// The exception for exceptions
		default:
			vfprintf(stderr, "\nInvalid exception code (0x%x)\n\n", args);
			break;
	}
	
	// Flip boolean to 0 or 1
	if (boolptr != NULL)
		*boolptr = (*boolptr != 1);
}
