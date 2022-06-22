#ifndef VERSE_HEADER_CDATA
#define VERSE_HEADER_CDATA

#include <stdio.h>

#include "macro.h"
#include "vdata.h"

#define CPU_RV64 	0
#define CPU_X86  	1
#define CPU_X64  	2
#define CPU_ARM64	3

#define OS_ONYX 	0
#define OS_LINUX 	1
#define OS_MACOS 	2
#define OS_WIN32 	3

// Assembler data struct
struct compiledata
{
	// Flags
	unsigned char
	f_cpu,                          	// Set CPU architecture target
	//f_bsz,                        	// Set byte size of CPU architecture target
	f_abi,                          	// Set platform target
	f_mlcom,                        	// Check if line is inside multi-line comment
	f_verbose,                      	// Show detailed program operations
	f_nodelimm,                      	// Set if immediate files should be removed
	f_needpass,                     	// Check if there are macros undefined
	f_lastpass,                     	// Check if last pass
	f_condisls[MAX_NESTLEVEL_CODE]; 	// Check whether previous condition is the last condition
	
	// Counters
	unsigned int
	c_condmaj[MAX_NESTLEVEL_CODE],  	// Condition counter (major)
	c_condmin[MAX_NESTLEVEL_CODE],  	// Condition counter (minor)
	c_linenum,                      	// Line counter
	c_passnum,                      	// Pass counter
	c_stacklv,                      	// Reserved stack counter for stack variables
	c_condlv,                       	// Condition nest counter (i.e. condition counter index)
	c_fconstnum,                    	// Float constant counter (i.e. __F1:)
	c_dconstnum,                    	// Double constant counter (i.e. __D1:)
	c_sconstnum;                    	// String constant counter (i.e. __S1:)
	
	// Parent address label
	char
	//*s_file_input,
	*s_file_output,
	*s_label_parent,
	*s_class_parent;
	
	// Address offsets
	long long unsigned int
	c_rel_base,     	// Base address
	c_rel_offset,   	// Relative address offset
	c_abs_offset;   	// Absolute address offset
	
	// Variable lists
	struct vlist
	*l_vars,    	// List of variables/relative addresses
	*l_addr,    	// List of absolute addresses
	*l_objt,    	// List of object templates
	*l_undef;   	// List of undefined macros
	
	// Function macro lists
	//struct mlist *l_macros;
};//cdata;

#endif