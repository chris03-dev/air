#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/macro.h"
#include "../inc/main.h"

#include "../inc/cdata.h"
#include "../inc/parse.h"
#include "../inc/node.h"
#include "../inc/error.h"

#include "../inc/x64.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

// Setup data in process tree before creating assembly code
int varproc(struct compiledata *cdata, struct node *root)
{
	// Recursion calls
	if (root->west != NULL) if (varproc(cdata, root->west)) return 1;
	if (root->east != NULL) if (varproc(cdata, root->east)) return 1;
	
	// Check for relative/absolute symbols
	unsigned char
	isrel = root->string[0] == '$',
	isabs = root->string[0] == '@';
	
	if (isrel || isabs)
	switch (strlen(root->string))
	{
		// Check if symbols
		case 2:
			if (root->string[1] == '$') snprintf(root->string, STRING_LENGTH_TOKEN, "%llu", cdata->c_rel_base);
		case 1:
			if (isrel) snprintf(root->string, STRING_LENGTH_TOKEN, "%llu", cdata->c_abs_offset + cdata->c_rel_offset);
			if (isabs) snprintf(root->string, STRING_LENGTH_TOKEN, "%llu", cdata->c_abs_offset);
			break;
			
		// Shift string to the left
		default:
			for (unsigned int i = 0, l = strlen(root->string); i < l - 1; ++i)
				root->string[i] = root->string[i + 1];
			break;
	}
	
	// Insert parent address label if member used
	// Note: Only for addresses, not objects
	else if (root->string[0] == '.')
	{
		char stmp[STRING_LENGTH_TOKEN];
		strncpy(stmp, root->string, STRING_LENGTH_TOKEN);
		
		strncpy(root->string, cdata->s_label_parent, STRING_LENGTH_TOKEN);
		strncpy(root->string, stmp, STRING_LENGTH_TOKEN);
	}
	
	// Check for variable/address label
	struct vdata
	*vdrel = vdatafind(cdata->l_vars, root->string),
	*vdabs = vdatafind(cdata->l_addr, root->string);
	
	if (vdrel != NULL)
	{
		// Check if variable or address
		
		// If there is an entry defined in address list, it is an address label.
		// If not, it is a variable.
		if (vdabs != NULL)
		{
			if (isrel) snprintf(root->string, STRING_LENGTH_TOKEN, "%llu", vdrel->value); else
			if (isabs) snprintf(root->string, STRING_LENGTH_TOKEN, "%llu", vdabs->value);
			else       snprintf(root->string, STRING_LENGTH_TOKEN, "%llu", vdabs->value + vdrel->value);
		}
		else snprintf(root->string, STRING_LENGTH_TOKEN, "%lli", vdrel->value);
		
		// Remove undefined entry from variable/address label, if any
		if (vdatafind(cdata->l_undef, root->string) != NULL)
			vdatadest(cdata->l_undef, root->string);
	}
	
	// Check for parenthesis
	else if ((root->string[0] == '(') && (root->root != NULL))
	{
		char name[STRING_LENGTH_VARNAME];
		
		// Check if nodes contain parenthesized equations
		strltok(name, root->string, "()", NULL, 0);
		strstrip(name);
		
		if (nodeproc(cdata, root, name, 0) == NULL)
			return 1;
	}
	
	// Inform assembler that another pass is required to find undefined name
	// Also cause error if name is still undefined after one pass
	else if ((nodelex(root) == 0) && !isinteger(root->string))
	{
		// Set flag to repeat pass
		cdata->f_needpass = 1;
		
		struct vdata *vdundef = vdatafind(cdata->l_undef, root->string);
		
		// Check if token is undefined from previous pass
		if ((vdundef != NULL) && (vdundef->value == 1))
		{
			throw_exception(EXCEPTION_TOKEN_UNDEFINED, NULL, cdata->c_linenum, root->string);
			return 1;
		}
		// Save token as undefined
		else vdatainit(cdata->l_undef, root->string, 0);
		
		strncpy(root->string, "0", STRING_LENGTH_TOKEN);
	}
	
	// Check if node is part of process tree
	if ((root->west != NULL) && (root->east != NULL))
	{
		// Values
		long long int
		value = 0,
		wvalue = stoi(root->west->string),
		evalue = stoi(root->east->string);
		
		// String for new value
		char vstring[STRING_LENGTH_NUMBER];
		// TODO: Use iswvar later
		int iswvar = (vdrel != NULL) && (vdabs == NULL);
		
		// Compute value from nodes
		switch (nodelex(root))
		{
			// Arithmetic operators
			case OP_MUL: value = wvalue * evalue; break;
			case OP_MOD: value = wvalue % evalue; break;
			case OP_DIV: value = wvalue / evalue; break;
			case OP_ADD: value = wvalue + evalue; break;
			case OP_SUB: value = wvalue - evalue; break;
			
			// Bitwise operators
			case OP_SHL: value = wvalue << evalue; break;
			case OP_SHR: value = wvalue >> evalue; break;
			
			// Equality operators
			case OP_MOV:
			case OP_MOVM:
			case OP_MOVD:
			case OP_MOVA:
			case OP_MOVS:
				fprintf(stderr, "\nError (Line %u): Whoever came up with the idea to use a constant value as the destination address should sip a bit of coffee.\n---\n", cdata->c_linenum);
				return 1;
			
			// Conditional operators
			case OP_GTHN: if (iswvar) value = wvalue > evalue;  break;
			case OP_LTHN: if (iswvar) value = wvalue < evalue;  break;
			case OP_GEQU: if (iswvar) value = wvalue >= evalue; break;
			case OP_LEQU: if (iswvar) value = wvalue <= evalue; break;
			case OP_EQU:  if (iswvar) value = wvalue == evalue; break;
			case OP_NEQU: if (iswvar) value = wvalue != evalue; break;
		}
		/*
		// Assign variable
		vdatainit(cdata->l_vars, , stoi(n->string));
		
		// (VERBOSE) Show value of variable
		if (cdata->f_verbose) printf("\nAssigned value of '%s' to %lli. (%s)\n\n", token[1], stoi(n->string), stritok(args, NULL, " \t", 2));
		*/
		// Apply new value to nodes
		snprintf(vstring, STRING_LENGTH_NUMBER, "%lli", value);
		strncpy(root->string, vstring, STRING_LENGTH_NUMBER);
		
		// Free memory of children nodes
		nodedest(root->west);
		nodedest(root->east);
		
		root->west = NULL;
		root->east = NULL;
	}
	
	// Check if node is the sole node
	else if (root->root == NULL)
	{
		// TODO: IDK
	}
	//if (root->root == NULL) printf("RESULT: %lli\n", stoi(root->string));
	
	return 0;
}

// Create process tree for assembly code
struct node* nodeproc(struct compiledata *cdata, struct node *dest, char *args, int autofree)
{
	char
	token[3][STRING_LENGTH_VARDATA] = {""},	// Tokens for function/variable names, and operator
	specl[2][STRING_LENGTH_VARDATA] = {""};	// Function arguments, if any
	
	unsigned int c_tok = 0;  	// Token counter
	
	// Nodes
	struct node
	*root  = NULL,  	// Node on top of everything else
	*focus = NULL,  	// Last node worked on in loop
	*droot = NULL;  	// Root of destination node
	
	// Check if null args
	if (args == NULL) return NULL;
	
	if (!strlen(args)) return (struct node *) 1;
	else for (unsigned int i = 0; strfltok(token[c_tok], args, "~:.", "[]()\"\"''", " \t", i); ++i, ++c_tok)
	switch (c_tok)
	{
		// Increment token counter
		case 0:
			// Cleanup special extension string
			strcpy(specl[0], "");
			break;
		
		case 1:
			break;
		
		// Process nodes
		case 2:
			// Cleanup special extension string
			strcpy(specl[1], "");
			
			// Setup process tree
			if (focus == NULL)
			{
				// Setup first 3 nodes
				struct node
				*n0 = nodeinit(token[0]),
				*n1 = nodeinit(token[1]),
				*n2 = nodeinit(token[2]);
				
				// Check if operator is valid
				if (nodelex(n1) == 0)
				{
					throw_exception(EXCEPTION_TOKEN_INVALID_OPERATOR, NULL, cdata->c_linenum, token[1]);
					if (autofree && (root != NULL)) nodedest(root);
					return NULL;
				}
				
				// Setup important nodes
				root = n1;
				focus = n1;
				
				// Setup node structure
				n1->west = n0;
				n1->east = n2;
				
				n0->root = n1;
				n2->root = n1;
				
				// Connect to destination node (if it exists)
				if (dest != NULL)
				{
					droot = dest->root;
					root->ext = dest->ext;
					dest->ext = NULL;
					
					//printf("OFFSET: %i %p\n", (dest->ext != NULL) ? dest->ext->offset : 0, (dest->ext != NULL) ? dest->ext->ext : NULL);
					
					if (droot != NULL)
					{
						root->root = droot;
						if (droot->west == dest)
						{
							nodedest(dest);
							droot->west = root;
						}
						if (droot->east == dest)
						{
							nodedest(dest);
							droot->east = root;
						}
					}
				}
			}
			
			// Compare nodes
			else
			{
				// Setup nodes
				struct node
				*n1 = nodeinit(token[1]),
				*n2 = nodeinit(token[2]),
				*comp = focus;
				
				// Check if operator is valid
				if (nodelex(n1) == 0)
				{
					throw_exception(EXCEPTION_TOKEN_INVALID_OPERATOR, NULL, cdata->c_linenum, token[1]);
					return NULL;
				}
				
				// Compare focus node operator importance with previous nodes' operators
				// Get closer to the process tree root
				while ((comp != droot) && ((nodelex(n1) / 8) <= (nodelex(comp) / 8)))
				{
					comp = comp->root;
				}
				
				// Setup basic connection between n1 and n2
				n1->root = comp;
				n1->east = n2;
				n2->root = n1;
				
				// Integrate nodes n1 and n2 to process tree
				if (comp != droot)
				{
					n1->west = comp->east;
					comp->east->root = n1;
					comp->east = n1;
				}
				// Set node n1 as root node
				else if (droot != NULL)
				{
					n1->root = droot;
					n1->west = root;
					
					if (droot->west == root) droot->west = n1;
					if (droot->east == root) droot->east = n1;
					
					// Transfer root extension node
					if (root->ext != NULL)
					{
						n1->ext = root->ext;
						root->ext = NULL;
					}
					
					root->root = n1;
					root = n1;
				}
				// Set node n1 as root node
				else
				{
					n1->west = root;
					
					// Transfer root extension node
					if (root->ext != NULL)
					{
						n1->ext = root->ext;
						root->ext = NULL;
					}
					
					root->root = n1;
					root = n1;
				}
				
				// Set new focus
				focus = n1;
			}
			
			//fprintf(stderr, "NODE: '%s%s %s %s%s'\n", focus->west->string, specl[0], focus->string, focus->east->string, specl[1]);
			//fprintf(stderr, "NODE: '%s %s %s'\n", focus->west->string, focus->string, focus->east->string);
			
			// Reset variables
			c_tok = 0;
			break;
		default:
			throw_exception(EXCEPTION_DEVELOPER_ERROR, NULL, "main.c", "nodeproc");
			if (autofree && (root != NULL)) nodedest(root);
			return NULL;
	}
	
	// Operate on process tree
	if (root != NULL)
	{
		// Calculate final value
		if (varproc(cdata, root))
		{
			// Destroy process tree
			if (autofree && (root != NULL)) nodedest(root);
			return NULL;
		}
	}
	else
	{
		char string[STRING_LENGTH_VARDATA];
		strfltok(string, args, "~:.", "[]()\"\"''", " \t", 0);
		
		root = nodeinit(string);
		
		// Calculate final value
		if (varproc(cdata, root))
		{
			// Destroy process tree
			if (autofree && (root != NULL)) nodedest(root);
			return NULL;
		}
	}
	
	// Destroy process tree
	if (autofree && (root != NULL)) nodedest(root);
	
	return root;
}

/*
	Note:
	If macro is already defined, return an error.
	Why? Because macros should only be defined before usage.
	
	e.g.
	
	z4 4
	z8 macro
	mov i4 rr ax, cx
	def macro $
	
	1ST PASS: macro = 7
	2ND PASS: macro = STILL 7 (11)
*/

// Check for keywords
int kwordproc(struct compiledata *cdata, FILE *fout, char *args, int finalpass)
{
	int status = 0;
	
	// Check for address labels
	char
	addrlbtok [STRING_LENGTH_TOKEN],
	addrlbtokf[STRING_LENGTH_TOKEN];
	
	if ((strltok(addrlbtok, args, NULL, " \t,", 0) != NULL)
	&&  (addrlbtok[strlen(addrlbtok) - 1] == ':'))
	{
		while ((strltok(addrlbtok, args, NULL, " \t,", 0) != NULL)
		&&     (addrlbtok[strlen(addrlbtok) - 1] == ':'))
		{
			// Null-terminate before colon
			addrlbtok[strlen(addrlbtok) - 1] = '\0';
			
			// Check if member of current parent address label
			if (addrlbtok[0] == '.')
			{
				// Combine parent and member address label by appending
				strcpy(addrlbtokf, cdata->s_label_parent);
				strcat(addrlbtokf, addrlbtok);
				
				// Delete existing label in order to update address label properly
				if (vdatafind(cdata->l_vars, addrlbtokf)) vdatadest(cdata->l_vars, addrlbtokf);
				if (vdatafind(cdata->l_addr, addrlbtokf)) vdatadest(cdata->l_addr, addrlbtokf);
				
				// Insert label to address list, and concatenate member address label to string
				vdatainit(cdata->l_vars, addrlbtokf, cdata->c_rel_offset);
				vdatainit(cdata->l_addr, addrlbtokf, cdata->c_abs_offset);
			}
			else
			{
				// Assign new parent address label
				strncpy(cdata->s_label_parent, addrlbtok, STRING_LENGTH_TOKEN);
				
				// Delete existing label in order to update address label properly
				if (vdatafind(cdata->l_vars, addrlbtok)) vdatadest(cdata->l_vars, addrlbtok);
				if (vdatafind(cdata->l_addr, addrlbtok)) vdatadest(cdata->l_addr, addrlbtok);
				
				// Insert label to address list
				vdatainit(cdata->l_vars, addrlbtok, cdata->c_rel_offset);
				vdatainit(cdata->l_addr, addrlbtok, cdata->c_abs_offset);
			}
			
			// Remove label from the line
			args = stritok(args, NULL, " \t,", 1);
		}
		
		// Add whitespace to verbose output
		if (cdata->f_verbose) puts("");
	}
	
	// Check if there's still something on the string
	if (strlen(args))
	{
		// Initialize tokens for keyword checking
		char token[2][STRING_LENGTH_TOKEN];
		strltok(token[0], args, "\"\"''", " \t,", 0),
		strltok(token[1], args, "\"\"''", " \t,", 1);
		
		// Generate assembly instructions
		switch (cdata->f_cpu)
		{
			case CPU_X64:   status = x64_proc_main(cdata, fout, args); break;
			case CPU_ARM64: break;
			default:
				throw_exception(EXCEPTION_DEVELOPER_ERROR, NULL, "main.c", "kwordproc");
				return 1;
		}
		
		// Check if assembly instructions are not used
		if (status > 0)
		{
			// Check if new struct object is to be defined
			if (vdatafind(cdata->l_objt, token[0]) != NULL)
			{
				
			}
			
			// Other keywords
			else if (strlen(token[0]) == 2)
			{
				// Define bytes
				if (strchr(token[0], 'd') == token[0])
				{
					char
					vtoken[STRING_LENGTH_TOKEN],
					*argsoff = stritok(args, NULL, " \t", 1);
					unsigned char size;
					
					// Check if 4th character is valid
					if (token[0][1] - '0' <= 9)
					{
						// Check if power of 2
						if (powcheck(token[0][1] - '0', 2)) size = token[0][1] - '0';
						else
						{
							fprintf(stderr, "\nError (Line %u): Invalid definition of values '%s'\n---\n", cdata->c_linenum, token[0]);
							return 1;
						}
					}
					else switch (token[0][1])
					{
						case 'x': size = 16; break;
						case 'y': size = 32; break;
						case 'z': size = 64; break;
						default:  fprintf(stderr, "\nError (Line %u): Invalid definition of values '%s'\n---\n", cdata->c_linenum, token[0]);
					}
					
					// (VERBOSE) Show values to be defined
					if (cdata->f_verbose) puts("Writing values...");
					
					// Loop to get values
					for (unsigned int i = 0; strstrip(strltok(vtoken, argsoff, NULL, ",", i)) != NULL; ++i)
					{
						// Check if token is valid value
						struct node *n = nodeproc(cdata, NULL, vtoken, 0);
						
						if (n == NULL)
						{
							if (n != NULL) free(n);
							return 1;
						}
						else
						{
							// Convert token to number
							unsigned long long int num = stoi(vtoken);
							
							// Check for overflow
							switch (size)
							{
								case 1:
									if (finalpass && (num > 0xff))
										throw_exception(EXCEPTION_LIMIT_SIZE_TYPE, NULL, cdata->c_linenum, num, size);
									num = num - (num >>  8 <<  8);
									break;
								
								case 2:
									if (finalpass && (num > 0xffff))
										throw_exception(EXCEPTION_LIMIT_SIZE_TYPE, NULL, cdata->c_linenum, num, size);
									num = num - (num >> 16 << 16);
									break;
								
								case 4:
									if (finalpass && (num > 0xffffffff))
										throw_exception(EXCEPTION_LIMIT_SIZE_TYPE, NULL, cdata->c_linenum, num, size);
									num = num - (num >> 32 << 32);
									break;
									
								case 8:
									break;
								
								default:
									throw_exception(EXCEPTION_DEVELOPER_ERROR, NULL, "main.c", "kwordproc");
									break;
							}
							
							// Output reserved numbers
							fwrite(&num, size, 1, fout);
							
							// Free node
							free(n);
							
							// (VERBOSE) Show values to be defined
							if (cdata->f_verbose) printf("%016llx\n", num);
						}
					}
					
					// (VERBOSE) Add whitspace to verbose output
					if (cdata->f_verbose) puts("");
				}
				
				// Reserve bytes
				if (strchr(token[0], 'z') == token[0])
				{
					unsigned int size;
					
					// Check if 4th character is valid
					if (token[0][1] - '0' <= 9)
					{
						// Check if power of 2
						if (powcheck(token[0][1] - '0', 2)) size = token[0][1] - '0';
						else
						{
							fprintf(stderr, "\nError (Line %u): Invalid definition of values '%s'\n---\n", cdata->c_linenum, token[0]);
							return 1;
						}
					}
					else switch (token[0][1])
					{
						case 'x': size = 16; break;
						case 'y': size = 32; break;
						case 'z': size = 64; break;
						default:  fprintf(stderr, "\nError (Line %u): Invalid definition of values '%s'\n---\n", cdata->c_linenum, token[0]);
					}
					
					// Check if 2nd token is valid number
					struct node *n = nodeproc(cdata, NULL, stritok(args, NULL, " \t", 1), 0);
					
					if (n == NULL)
					{
						if (n != NULL) free(n);
						return 1;
					}
					else
					{
						unsigned long long int nmemb = stoi(n->string);
						
						void *out;
						
						// This code ensures that nmemb does not go over 4096 bytes at a time
						while (nmemb > 0x10000)
						{
							// Allocate memory for output
							out = calloc(0x10000, size);
							
							// Output reserved numbers
							fwrite(out, size, nmemb, fout);
							
							// Free memory for output
							free(out);
							
							nmemb -= 0x10000;
						}
						
						// Allocate memory for output
						out = calloc(nmemb, size);
						
						if (out == NULL) fprintf(stderr, "(LINE %u, %llx) FIX THIS\n", cdata->c_linenum, nmemb);
						
						// Output reserved numbers
						fwrite(out, size, nmemb, fout);
						
						// Free memory for output
						free(out);
						free(n);
						
						if (cdata->f_verbose) printf("Wrote %llu empty bytes.\n\n", nmemb * size);
					}
					
				}
			
				// Align bytes (NOP)
				if (!strcmp(token[0], "al"))
				{
					struct node *n = nodeproc(cdata, NULL, stritok(args, NULL, " \t", 1), 0);
					
					// Check if equation results in valid number
					if (n == NULL) return 1;
					else
					{
						unsigned long long int
						aln = stoi(n->string),
						size = aln - (cdata->c_abs_offset % aln);
						
						// Output NOP instructions
						for (unsigned int i = 0; i < size; ++i)
							fputc(0x90, fout);
					}
				}
			}
			
			else if (strlen(token[0]) == 3)
			{
				// Set relative offset
				if (!strcmp(token[0], "org"))
				{
					struct node *n = nodeproc(cdata, NULL, stritok(args, NULL, " \t", 1), 0);
					
					// Check if equation results in valid number
					if (n == NULL) return 1;
					else
					{
						unsigned long long int rel_off = stoi(n->string);
						
						// Check if valid relative offset
						if (rel_off < cdata->c_abs_offset + cdata->c_rel_offset)
						{
							printf("\nError (Line %u): Base offset '%016llx' is less than the current offset '%016llx'\n---\n", cdata->c_linenum, rel_off, cdata->c_abs_offset + cdata->c_rel_offset);
							return 1;
						}
						else
						{
							// Assign new address base
							cdata->c_rel_offset = rel_off - cdata->c_abs_offset;
							cdata->c_rel_base = rel_off;
							
							// (VERBOSE) Show new address base, along with offsets after changes
							if (cdata->f_verbose) printf("\nAssigned new base address: %016llx\n\n", cdata->c_rel_base);
						}
					}
				}
				
				// Align bytes (Reserved)
				if (!strcmp(token[0], "pad"))
				{
					struct node *n = nodeproc(cdata, NULL, stritok(args, NULL, " \t", 1), 0);
					
					// Check if equation results in valid number
					if (n == NULL) return 1;
					else
					{
						unsigned long long int
						aln = stoi(n->string),
						size = aln - (cdata->c_abs_offset % aln);
						
						// Allocate memory for output
						void *out = calloc(size, 1);
						
						// Output reserved bytes
						fwrite(out, size, 1, fout);
						
						// Free memory for output
						free(out);
					}
				}
				
				// Define multi-line macro
				if (!strcmp(token[0], "mac"))
				{
					puts("MAC ATTACK 2: MAC STRIKES BACK");
				}
				
				// Define single-line macro
				if (!strcmp(token[0], "def"))
				{
					
				}
				
				// Define variable
				if (!strcmp(token[0], "var"))
				{
					struct node *n = nodeproc(cdata, NULL, stritok(args, NULL, " \t", 2), 0);
					
					// Check if equation results in valid number
					if (n == NULL)
					{
						throw_exception(EXCEPTION_DEVELOPER_ERROR, NULL, "main.c", "kwordproc");
						return 1;
					}
					else
					{
						// Assign variable
						vdatainit(cdata->l_vars, token[1], stoi(n->string));
						
						// (VERBOSE) Show value of variable
						if (cdata->f_verbose) printf("Assigned value of '%s' to %lli. (%s)\n\n", token[1], stoi(n->string), stritok(args, NULL, " \t", 2));
					}
				}
				
				// Output command on file
				if (!strcmp(token[0], "sys"))
				{
					char cmd[STRING_LENGTH_TOKEN];
					
					if (cdata->f_lastpass) 
					{
						strncpy(cmd, stritok(args, "\"\"''", " \t,", 1), STRING_LENGTH_TOKEN);
						
						// Add newline
						puts("");
						
						// Check for command error
						if (system(cmd) == -1)
						{
							throw_exception(EXCEPTION_EXECUTION_ERROR, NULL, cmd);
							return 1;
						}
						
						// Add newline
						puts("");
					}
				}
			}
			
			else if (strlen(token[0]) == 4)
			{
				// Print number as decimal
				if (!strcmp(token[0], "dspi"))
				{
					char
					vtoken[STRING_LENGTH_TOKEN],
					*argsoff = stritok(args, NULL, " \t", 1);
					struct node *n;
					
					// Check if number should be displayed (should be last pass)
					if (cdata->f_lastpass)
					{
						for (unsigned int i = 0; strstrip(strltok(vtoken, argsoff, NULL, ",", i)) != NULL; ++i)
						{
							n = nodeproc(cdata, NULL, vtoken, 0);
							
							// Check if equation results in valid number
							if (n == NULL)
							{
								// Check if input was valid in the first place
								if (i == 0) return 1;
								else break;
							}
							
							// Show value of variable
							else printf("DISPLAY: %lli (%s)\n", stoi(n->string), vtoken);
						}
						
						// (VERBOSE) Add newline
						if (cdata->f_verbose) puts("");
					}
				}
				
				// Print number as hexadecimal
				if (!strcmp(token[0], "dspx"))
				{
					char
					vtoken[STRING_LENGTH_TOKEN],
					*argsoff = stritok(args, NULL, " \t", 1);
					struct node *n;
					
					// Check if number should be displayed (should be last pass)
					if (cdata->f_lastpass)
					{
						for (unsigned int i = 0; strstrip(strltok(vtoken, argsoff, NULL, ",", i)) != NULL; ++i)
						{
							n = nodeproc(cdata, NULL, vtoken, 0);
							
							// Check if equation results in valid number
							if (n == NULL)
							{
								// Check if input was valid in the first place
								if (i == 0) return 1;
								else break;
							}
							
							// Show value of variable
							else printf("DISPLAY: 0x%llx (%s)\n", stoi(n->string), vtoken);
						}
						
						// (VERBOSE) Add newline
						if (cdata->f_verbose) puts("");
					}
				}
				
				// Output command on file (without output)
				if (!strcmp(token[0], "exec"))
				{
					char cmd[STRING_LENGTH_TOKEN];
					
					if (cdata->f_lastpass) 
					{
						strncpy(cmd, stritok(args, "\"\"''", " \t,", 1), STRING_LENGTH_TOKEN);
						
						// Output should not appear to terminal
						#if   defined __linux__
						strncat(cmd, " > /dev/null 2>&1", STRING_LENGTH_TOKEN);
						#elif defined _WIN32
						strncat(cmd, " > NUL", STRING_LENGTH_TOKEN);
						#endif
						
						// Check for command errors
						if (system(cmd) == -1)
						{
							throw_exception(EXCEPTION_EXECUTION_ERROR, NULL, cmd);
							return 1;
						}
						
						// (VERBOSE) Show command used in 'exec'
						else if (cdata->f_verbose)
							printf("\nExecuted command '%s'.\n", cmd);
						
						// Add newline
						if (cdata->f_verbose) puts("");
					}
				}
				
				// Force another pass
				if (!strcmp(token[0], "pass"))
				{
					cdata->f_needpass = 1;
					
					// (VERBOSE) Show that another pass is needed
					if (cdata->f_verbose) puts("\nAnother pass is needed.");
				}
			}
			
			// Failure
			else
			{
				throw_exception(EXCEPTION_TOKEN_UNDEFINED, NULL, cdata->c_linenum, token[0]);
				return 1;
			}
		}
		
		// Check if assembly generation failed
		if (status == -1)
		{
			fputs("\nAssembly generation failed.\n", stderr);
			return 1;
		}
		
	}
	
	// Update absolute address offset
	cdata->c_abs_offset = ftell(fout);
	
	return 0;
}

// Open input file, write to output file
int fileproc(struct compiledata *cdata, FILE *fout, char *args)
{
	char fcpyname[] = "__filecpy.txt";
	
	FILE 
	*fptr = fopen(args,     "r+"),
	*fcpy = fopen(fcpyname, "w+");
	
	// Bool for errors
	int status = 0;
	
	// Read line and process into output file
	if (fptr == NULL)
	{
		throw_exception(EXCEPTION_FILE_NO_ACCESS, NULL, args);
		return 1;
	}
	else if (fcpy == NULL)
	{
		throw_exception(EXCEPTION_FILE_NO_CREATE, NULL, fcpyname);
		return 1;
	}
	else
	{
		char line[STRING_LENGTH_LINE];
		int tmpvarvbs = 0;
		
		// Inform user of successful access to input file
		printf("Set '%s' as input file.\n", args);
		
		// Write to duplicate file (so original file is not overwritten)
		while (fgets(line, STRING_LENGTH_LINE, fptr))
		{
			cdata->c_linenum++;
			
			// Strip line
			strstrip(line);
			
			// Counter for newlines
			// Important for backslashes, which may include multiple lines
			unsigned int c_newline = 1;
			
			// Extend line if backslash found at end of line
			for (; line[strlen(line) - 1] == '\\'; ++c_newline)
			{
				if (fgets(line + strlen(line) - 1, STRING_LENGTH_LINE - strlen(line) - 1, fptr) != NULL)
				{
					cdata->c_linenum++;
					strstrip(line);
				}
				else break;
			}
			
			// Process line
			strstrip(line);
			strpreproc(line, &cdata->f_mlcom);
			strstrip(line);
			
			// Duplicate line to new file
			fwrite(line, sizeof(char), strlen(line), fcpy);
			
			// Add newlines after line
			for (; c_newline > 0; --c_newline)
				fputc('\n', fcpy);
		}
		
		// Setup duplicate file to read
		cdata->c_linenum = 0;
		fseek(fcpy, 0, SEEK_SET);
		
		// Disable verbose mode for now (if enabled)
		tmpvarvbs = cdata->f_verbose;
		cdata->f_verbose = 0;
		
		// Read lines of input file (macro and define pass loop)
		for (cdata->f_needpass = 1; cdata->f_needpass && cdata->c_passnum < MAX_PASSCOUNT_CODE; ++cdata->c_passnum)
		{
			// Set flag to 0 to determine if some function signifies need for another pass
			cdata->f_needpass = 0;
			
			// (VERBOSE) Show pass counter
			if (cdata->f_verbose) printf("PASS %u\n", cdata->c_passnum);
			
			while (fgets(line, STRING_LENGTH_LINE, fcpy) != NULL)
			{
				cdata->c_linenum++;
				
				// Strip line
				strstrip(line);
				
				// Parse line
				if (strlen(line)) 
				if (kwordproc(cdata, fout, line, 0))  	// Conditional statements
					status = 1;
			}
			
			// Setup duplicate file to read
			cdata->c_linenum = 0;
			cdata->c_rel_offset = 0;
			cdata->c_abs_offset = 0;
			fseek(fout, 0, SEEK_SET);
			fseek(fcpy, 0, SEEK_SET);
			
			// Set all undefined variables/address labels to force error in next pass
			for (struct vdata *vd = cdata->l_undef->start; vd != NULL; vd = vd->next)
				vd->value = 1;
			
			// Force return if failure before another pass begins
			if (status) return 1;
		}
		
		// Check if exceeded passes
		if (cdata->c_passnum >= MAX_PASSCOUNT_CODE)
		{
			throw_exception(EXCEPTION_LIMIT_PASS_COUNT, NULL, MAX_PASSCOUNT_CODE);
			status = 1;
		}
		
		// Enable verbose mode now (if enabled)
		cdata->f_verbose = tmpvarvbs;
		
		// Set last pass flag
		cdata->f_lastpass = 1;
		
		// Read lines of input file (final pass)
		while (fgets(line, STRING_LENGTH_LINE, fcpy) != NULL)
		{
			cdata->c_linenum++;
			
			// Process line
			strstrip(line);
			
			// Check if line should be parsed
			if (strlen(line))
			{
				// (VERBOSE) Show line
				if (cdata->f_verbose) printf("LINE %04u: '%s'\n", cdata->c_linenum, line);
				
				// Check for address label
				char addrlbtok[STRING_LENGTH_TOKEN];
				
				if ((strltok(addrlbtok, line, NULL, " \t,", 0) != NULL)
				&&  (addrlbtok[strlen(addrlbtok) - 1] == ':'))
				{
					while ((strltok(addrlbtok, line, NULL, " \t,", 0) != NULL)
					&&     (addrlbtok[strlen(addrlbtok) - 1] == ':'))
					{
						// Null-terminate before colon
						addrlbtok[strlen(addrlbtok) - 1] = '\0';
						
						// (VERBOSE) Show assigned address label
						if (cdata->f_verbose) printf("Assigned '%s' as address %016llx.\nREL: %016llx\nABS: %016llx\n", addrlbtok, cdata->c_abs_offset + cdata->c_rel_offset, cdata->c_rel_offset, cdata->c_abs_offset);
						
						// Remove label from the line
						char *index = stritok(line, NULL, " \t,", 1);
						if (index > line)
						for (unsigned int i = 0, l = strlen(line); i <= l; ++i)
							line[i] = index[i];
					}
					
					// Add whitespace to verbose output
					if (cdata->f_verbose) puts("");
				}
				
				// Parse the line
				if (kwordproc(cdata, fout, line, 1))  	// Conditional statements
					status = 1;
			}
		}
		
		printf("%u pass%s completed. Read finished.\n", cdata->c_passnum, (cdata->c_passnum > 1) ? "es" : "");
		
		// Close files
		fclose(fptr);
		fclose(fcpy);
		
		return status;
	}
}

// Configure compile data in program
int cdataconf(struct compiledata *cdata, char *args)
{
	char
	*flag =  stritok(args, NULL, "-=", 0),
	*value = stritok(args, NULL, "=",  1);
	
	// Search for valid flag
	switch (args[1])
	{
		case '-':
			if (strstr(flag, "help")     == args + 2) goto j_help;     else
			if (strstr(flag, "license")  == args + 2) goto j_license;  else
			if (strstr(flag, "platform") == args + 2) goto j_targetos; else
			if (strstr(flag, "nodelimm") == args + 2) goto j_nodelimm; else
			if (strstr(flag, "target")   == args + 2) goto j_targetos; else
			if (strstr(flag, "verbose")  == args + 2) goto j_verbose;
			else goto j_flag_invalid;
		
		// Help text
		case 'h':
		j_help:
			printf("Abstract Intermediate Representation (v%i.%i.%i)\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
			puts("Copyright (c) 2022, chris03-dev\n");
			
			puts("Usage: air <output file> <flag=value> ... <input file> ...\n");
			
			puts("Generates binary files from cross-platform AIRC (Assembly");
			puts("Intermediate Representation Code)-syntax files.");
			
			puts("Flags may be used anywhere in your console input after the");
			puts("the compiler executable name. Note that the flags are only");
			puts("in effect for file arguments succeeding such flags.\n");
			
			puts("Values are automatically ignored in flags that do not need");
			puts("them.\n");
			
			puts("Flag     Description                            Value");
			puts("-c       Set target CPU architecture            x64,rv64*");
			puts("-h       Read this whole text                   --");
			puts("-l       Read the license of the source code    --");
			puts("         used to build this executable");
			puts("-p -t    Set target platform for assembly       linux,win32*,osx*");
			puts("-s       Compile to assembly code only          --");
			puts("-v       Expose assembler data upon operation;  0 - 1");
			puts("         Includes boolean values, lines read,");
			puts("         tokens identified, etc.");
			
			break;
		
		// License text
		case 'l':
		j_license:
			puts("Mozilla Public License, version 2.0\n");
			
			puts("This Executable Form was built using its Source Code form that");
			puts("is subject to the terms of the Mozilla Public License, v. 2.0.");
			puts("If a copy of the MPL was not distributed with this Executable");
			puts("form, you can obtain one at:\n");
			
			puts("'https://mozilla.org/MPL/2.0/'\n");
			
			puts("If a copy of the Source Code form was not distributed with this");
			puts("Executable Form, you can obtain one at:\n");
			
			puts("'https://codeberg.org/chris03-dev/air'");
			puts("\n---\n");
			
			break;
		
		// Target platform
		case 'p':
		case 't':
		j_targetos:
			if (value != NULL)
			{
				if (!strcmp(value, "linux"))
				{
					cdata->f_abi = OS_LINUX;
					puts("Set platform target to Linux.");
				}
				else if (!strcmp(value, "win32"))
				{
					cdata->f_abi = OS_WIN32;
					puts("Set platform target to Windows.");
				}
				else if (!strcmp(value, "osx"))
				{
					cdata->f_abi = OS_MACOS;
					puts("Set platform target to Mac OS.");
				}
				else goto j_value_invalid;
			}
			else goto j_value_notfound;
			break;
		
		// Do not remove immediate files
		case 's':
		j_nodelimm:
			cdata->f_nodelimm = 1;
			break;
		
		// Verbose mode
		case 'v':
		j_verbose:
			if (isinteger(value))
			switch (stoi(value))
			{
				case 1:
					cdata->f_verbose = 1;
					puts("Enabled verbose mode.");
					break;
				case 0:
					cdata->f_verbose = 0;
					puts("Disabled verbose mode.");
					break;
				default:
					goto j_value_invalid;
			}
			else goto j_value_notfound;
			break;
		
		// Warnings and errors
		default:
		j_flag_invalid:
			throw_exception(EXCEPTION_FLAG_INVALID, NULL, args);
			return 1;
		j_value_notfound:
			throw_exception(EXCEPTION_FLAG_NO_VALUE, NULL, args);
			return 1;
		j_value_invalid:
			throw_exception(EXCEPTION_FLAG_INVALID_VALUE, NULL, value, args);
			return 1;
	}
	return 0;
}

// Main function
int main(int argc, char **argv)
{
	// FOR DEBUG PURPOSES ONLY
	setbuf(stdout, NULL);
	
	/*
	{
		char args[] = "dspx hex + 1, hex, 0x300";
		char out[256];
		
		puts(strstrip(strltok(out, args, NULL, ",", 0)));
		
		for (int i = 0; strstrip(strltok(out, stritok(args, NULL, " \t", 1), NULL, ",", i)); ++i)
		{
			puts(out);
		}
	}
	
	//"Hello Wor(ld\")\" HI!!!"
	
	char *c;//[256];
	for (int i = 0; (c = striftok("Hello Wor(ld\")\" H)I!!!", NULL, "\"\"()", " ", i)) != NULL; ++i)
		puts(c);
	*/
	
	// Compiler data
	struct compiledata cdata =
	{
		// CPU architecture
		#if   defined __x86_64__
		CPU_X64,
		#elif defined __arm__
		CPU_ARM64,
		#endif
		
		// OS platform target
		#if   defined __linux__
		OS_LINUX,
		#elif defined _WIN32
		OS_WIN32,
		#elif defined __APPLE__
		OS_MACOS,
		#endif
		
		0, 0, 0, 0, 0, {0}, {0},
		{0}, 0, 0, 0, 0, 0, 0, 0,
		
		calloc(STRING_LENGTH_TOKEN, sizeof(char)),
		calloc(STRING_LENGTH_TOKEN, sizeof(char)),
		calloc(STRING_LENGTH_TOKEN, sizeof(char)),
		
		0, 0, 0,
		calloc(1, sizeof(struct vlist)),
		calloc(1, sizeof(struct vlist)),
		calloc(1, sizeof(struct vlist)),
		calloc(1, sizeof(struct vlist))
	};
	
	// Setup files
	FILE *fout = NULL;
	
	// Loop arguments
	if (argc <= 1)
	{
		//printf("No input detected.\nInput '%s -h' for instructions.\n", argv[0]);
		cdataconf(NULL, "-h");
		return 1;
	}
	else for (unsigned int i = 1; i < argc; ++i) 
	{
		// Check for flags
		if (argv[i][0] == '-')
			cdataconf(&cdata, argv[i]);
		
		// Setup output file
		else if (fout == NULL)
		{
			fout = fopen(argv[i], "wb+");
			
			// Check if creating the output file works
			if (fout == NULL)
			{
				throw_exception(EXCEPTION_FILE_NO_CREATE, NULL, argv[0]);
				break;
			}
			else
			{
				printf("Set '%s' as output file.\n", argv[i]);
				cdata.s_file_output = argv[i];
			}
		}
		
		// Read input file
		else if (fileproc(&cdata, fout, argv[i]))
			break;
	}
	
	// Free lists
	if (fout != NULL)
	{
		/*
		fseek(fout, 0, SEEK_SET);
		fputc(0xff, fout);
		fputc(0xfe, fout);
		fputc(0xfd, fout);
		*/
		
		// Free lists
		vdatadestall(cdata.l_vars);
		vdatadestall(cdata.l_objt);
		
		// Close output file
		fclose(fout);
	}
	
	printf("'%s' finished execution.\n", argv[0]);
	
	return 0;
}