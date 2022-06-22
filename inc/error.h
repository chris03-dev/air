// Exception codes

// File access exceptions
#define EXCEPTION_FILE_NO_CREATE                	0x0010
#define EXCEPTION_FILE_NO_ACCESS                	0x0011

// Compiler flags exceptions
#define EXCEPTION_FLAG_INVALID                  	0x0020
#define EXCEPTION_FLAG_NO_VALUE                 	0x0021
#define EXCEPTION_FLAG_INVALID_VALUE            	0x0022

// Node process exceptions
#define EXCEPTION_TOKEN_INVALID_OPERATOR        	0x0100
#define EXCEPTION_TOKEN_UNDEFINED               	0x0101

// Exceeding limits exceptions
#define EXCEPTION_LIMIT_NESTING                 	0x0200
#define EXCEPTION_LIMIT_PASS_COUNT              	0x0201
#define EXCEPTION_LIMIT_SIZE_TYPE               	0x0202

// Assembly-related
#define EXCEPTION_ASSEMBLY_INVALID_OPERAND_SIZE 	0x1000
#define EXCEPTION_ASSEMBLY_INVALID_OPERAND_TYPE 	0x1001
#define EXCEPTION_ASSEMBLY_INVALID_REGISTER     	0x1002

// Miscellaneous
#define EXCEPTION_DEVELOPER_ERROR               	0x8000
#define EXCEPTION_EXECUTION_ERROR               	0x8001

void throw_exception(int, int *, ...);