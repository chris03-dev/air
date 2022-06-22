#ifndef VERSE_AIR_HEADER_CPU_X64
#define VERSE_AIR_HEADER_CPU_X64

#include <stdio.h>

// Register ID

// General purpose registers
#define REGISTER_X64_AX 	0
#define REGISTER_X64_CX 	1
#define REGISTER_X64_DX 	2
#define REGISTER_X64_BX 	3
#define REGISTER_X64_SP 	4
#define REGISTER_X64_BP 	5
#define REGISTER_X64_SI 	6
#define REGISTER_X64_DI 	7

// REX-Extended general purpose registers
#define REGISTER_X64_R8 	8
#define REGISTER_X64_R9 	9
#define REGISTER_X64_RA 	10
#define REGISTER_X64_RB 	11
#define REGISTER_X64_RC 	12
#define REGISTER_X64_RD 	13
#define REGISTER_X64_RE 	14
#define REGISTER_X64_RF 	15

// Executable sections
#define REGISTER_X64_ES		16
#define REGISTER_X64_CS 	17
#define REGISTER_X64_SS 	18
#define REGISTER_X64_DS		19
#define REGISTER_X64_FS 	20
#define REGISTER_X64_GS 	21

// Control registers
#define REGISTER_X64_CR0	24
#define REGISTER_X64_CR1	25
#define REGISTER_X64_CR2	26
#define REGISTER_X64_CR3	27
#define REGISTER_X64_CR4	28
#define REGISTER_X64_CR5	29
#define REGISTER_X64_CR6	30
#define REGISTER_X64_CR7	31
#define REGISTER_X64_CR8	32


// SSE
#define REGISTER_X64_F0 	64
#define REGISTER_X64_F1 	65
#define REGISTER_X64_F2 	66
#define REGISTER_X64_F3 	67
#define REGISTER_X64_F4 	68
#define REGISTER_X64_F5 	69
#define REGISTER_X64_F6 	70
#define REGISTER_X64_F7 	71

#define REGISTER_X64_F8 	72
#define REGISTER_X64_F9 	73
#define REGISTER_X64_FA 	74
#define REGISTER_X64_FB 	75
#define REGISTER_X64_FC 	76
#define REGISTER_X64_FD 	77
#define REGISTER_X64_FE 	78
#define REGISTER_X64_FF 	79

// Register ID Groups
#define REGISTER_X64_GROUP_IGP  	0
#define REGISTER_X64_GROUP_REX  	1
#define REGISTER_X64_GROUP_SEG  	2
#define REGISTER_X64_GROUP_CRX  	3
#define REGISTER_X64_GROUP_CR8  	4
#define REGISTER_X64_GROUP_FGP  	8
#define REGISTER_X64_GROUP_FGPX 	9

// Instruction functions
// Load/store
int x64_mov_main(struct compiledata *, FILE *, const char *, char, char, char *);
int x64_get_main(struct compiledata *, FILE *, const char *, char, char, char *);

// Arithmetic
//x86add


// x86 register identification
int x64_registerid(const char *);

// x86 main function
int x64_proc_main(struct compiledata *, FILE *, char *);

#endif