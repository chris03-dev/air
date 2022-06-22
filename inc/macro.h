#ifndef VERSE_HEADER_MACRO
#define VERSE_HEADER_MACRO

// Limits
#define STRING_LENGTH_LINE    	2048
#define STRING_LENGTH_TOKEN   	256
#define STRING_LENGTH_VARDATA 	256
#define STRING_LENGTH_OFFSET  	128
#define STRING_LENGTH_REGISTER	92	// Why is this larger than the varname? Because register is sometimes substituted for the variable name + operator + offset
#define STRING_LENGTH_VARNAME 	64
#define STRING_LENGTH_NUMBER  	24
#define MAX_BYTESIZE_X64      	8
#define MAX_NESTLEVEL_CODE    	64
#define MAX_PASSCOUNT_CODE    	64

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#endif