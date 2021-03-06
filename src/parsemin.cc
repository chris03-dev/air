#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "../inc/cdata.h"
#include "../inc/parse.h"

int strchrcount(const char *s, char c)
{
	unsigned int c_occ = 0; 	// Occurences of character
	
	for (unsigned int i = 0, l = strlen(s); i < l; ++i)
	{
		// Check if first run
		if (i == 0)
		{
			if (s[i] == c) c_occ++;
		}
		else if ((s[i] == c) && (s[i - 1] != '\\'))
		{
			if (c == '\\') c_occ--;
			else           c_occ++;
		}
	}
	
	return c_occ;
}

int strnchrcount(const char *s, char c, size_t n)
{
	unsigned int c_occ = 0; 	// Occurences of character
	
	for (unsigned int i = 0; i < n; ++i)
	{
		// Check if first run
		if (i == 0)
		{
			if (s[i] == c) c_occ++;
		}
		else if ((s[i] == c) && (s[i - 1] != '\\'))
		{
			if (c == '\\') c_occ--;
			else           c_occ++;
		}
	}
	
	return c_occ;
}

// Get line of token
int strctok(char *out, const char *s, const char *fdelim, const char *delim, size_t ti)
{
	int c = 0;	// Counter against number of tokens
	
	unsigned int 
	is = 0, 	// Saved index
	nl = 0; 	// Nest level
	
	// Saved limiter as pointer to s
	char *flm = NULL;
	
	// Check if char pointer arguments are null
	if (fdelim == NULL) fdelim = "";
	if (delim  == NULL)  delim = "";
	
	for (unsigned int i = 0, l = strlen(s); i < l; ++i)
	{
		// Check if not delimiter
		if ((strchr(delim, s[i]) == NULL))
		{
			// Check if counter is less than limit
			if (c <= ti)
			{
				// Get token
				for (is = i; i < l; ++i)
				{
					// Check if limiter is assigned
					if (flm != NULL)
					{
						//printf("STRING: %s\n", out);
						// Check if end of field limiter (limiter assigned)
						if (s[i] == flm[1] && (s[i - 1] != '\\'))
						{
							if (nl > 0)
							{
								nl--;
								out[i - is] = s[i];
							}
							else if (s[i - 1] != '\\')
							{
								flm = NULL;
								break;
							}
						}
						
						// Regular character
						else
						{
							if ((s[i] == flm[0]) && (s[i - 1] != '\\')) nl++;
							out[i - is] = s[i];
						}
					}
					
					// Check if start of field limiter (limiter not assigned)
					else if (strchr(fdelim, s[i]) != NULL)
					{
						flm = (char *) strchr(fdelim, s[i]);
						if (i - is == 0) c--;
						break;
					}
					// Check if delimiter
					else if (strchr(delim, s[i]) != NULL)
						break;
					
					// Regular character
					else out[i - is] = s[i];
				}
				
				// Save index for null-termination
				is = i - is;
				
				// Increment counter
				c++;
			}
			
			// Exit loop
			else break;
		}
	}
	
	// Null-terminate token
	out[is] = '\0';
	
	// Exit code
	if (flm != NULL) return 2;
	if (c <= ti)     return 0;
	else             return 1;
}

// Get line of token, including field delimiters
int strcltok(char *out, const char *s, const char *suffix, const char *fdelim, const char *delim, size_t ti)
{
	int c = 0;	// Counter against number of tokens
	
	unsigned int
	is = 0, 	// Saved index
	nl = 0; 	// Nest level
	
	// Saved limiter as pointer to s
	char *flm = NULL;
	//suffix = NULL;
	
	// Check if char pointer arguments are null
	if (suffix == NULL) suffix = "";
	if (fdelim == NULL) fdelim = "";
	if (delim  == NULL)  delim = "";
	
	for (unsigned int i = 0, l = strlen(s); i < l; ++i)
	{
		// Check if not delimiter
		if ((strchr(delim, s[i]) == NULL))
		{
			// Check if counter is less than limit
			if (c <= ti)
			{
				// Check if field limiter set
				if (flm != NULL) out[0] = (char) *flm;
				
				// Get token
				for (is = i - (flm != NULL); i < l; ++i)
				{
					// Check if limiter is assigned
					if (flm != NULL)
					{
						// Check if end of field limiter (limiter assigned)
						if (s[i] == flm[1] && (s[i - 1] != '\\'))
						{
							if (nl > 0)
							{
								nl--;
								out[i - is] = s[i];
							}
							else if (s[i - 1] != '\\')
							{
								out[i - is] = *(flm + 1);
								flm = NULL;
								
								// Check if next character is part of inclusion characters
								if ((s[i + 1] == '\0') || (strchr(suffix, s[i + 1]) == NULL))
								{
									is--;
									break;
								}
							}
						}
						
						// Regular character
						else
						{
							if ((s[i] == flm[0]) && (s[i - 1] != '\\')) nl++;
							out[i - is] = s[i];
						}
					}
					
					// Check if start of field limiter (limiter not assigned)
					else if (strchr(fdelim, s[i]) != NULL)
					{
						flm = (char *) strchr(fdelim, s[i]);
						if (i - is == 0) c--;
						break;
					}
					// Check if delimiter
					else if ((strchr(delim,  s[i]) != NULL)
					&&       (strchr(suffix, s[i]) == NULL))
						break;
					
					// Regular character
					else out[i - is] = s[i];
				}
				
				// Save index for null-termination
				is = i - is;
				
				// Increment counter
				c++;
			}
			
			// Exit loop
			else break;
		}
	}
	
	// Null-terminate token
	out[is] = '\0';
	
	// Exit code
	if (flm != NULL) return 2;
	if (c <= ti)     return 0;
	else             return 1;
}

// Get index at beginning of token
char *stritok(const char *s, const char *fdelim, const char *delim, size_t ti)
{
	int c = 0; 	// Counter against number of tokens
	
	unsigned int 
	i = 0,  	// String index
	l = 0,  	// String length
	is = 0, 	// Saved index
	nl = 0; 	// Nest level
	
	// Saved limiter as pointer to s
	char *flm = NULL;
	
	// Check if char pointer arguments are null
	if (fdelim == NULL) fdelim = "";
	if (delim  == NULL)  delim = "";
	
	for (i = 0, l = strlen(s); (i < l); ++i)
	{
		// Check if not delimiter
		if ((strchr(delim, s[i]) == NULL))
		{
			// Check if counter is less than limit
			if (c < ti)
			{
				// Get token
				for (is = i; i < l; ++i)
				{
					// Check if limiter is assigned
					if (flm != NULL)
					{
						// Check if end of field limiter (limiter assigned)
						if ((s[i] == flm[1]) && (s[i - 1] != '\\'))
						{
							if (nl > 0) nl--;
							else
							{
								flm = NULL;
								break;
							}
						}
						else if ((s[i] == flm[0]) && (s[i - 1] != '\\')) nl++;
					}
					
					// Check if start of field limiter (limiter not assigned)
					else if (strchr(fdelim, s[i]) != NULL)
					{
						flm = (char *) strchr(fdelim, s[i]);
						if (i - is == 0) c--;
						break;
					}
					// Check if delimiter
					else if (strchr(delim, s[i]) != NULL)
						break;
				}
				// Increment counter
				c++;
			}
			
			// Exit loop
			else break;
		}
	}
	
	// Exit code
	if (i >= l) return NULL;
	else        return (char *) s + i;
}

// Get index at beginning of token, including field delimiters
char *striltok(const char *s, const char *suffix, const char *fdelim, const char *delim, size_t ti)
{
	unsigned int 
	c = 0,  	// Counter against number of tokens
	i = 0,  	// String index
	l = 0,  	// String length
	is = 0, 	// Saved index
	nl = 0; 	// Nest level
	
	// Saved limiter as pointer to s
	char *flm = NULL;
	
	// Check if char pointer arguments are null
	if (suffix == NULL) suffix = "";
	if (fdelim == NULL) fdelim = "";
	if (delim  == NULL)  delim = "";
	
	for (i = 0, l = strlen(s); (i < l); ++i)
	{
		// Check if not delimiter
		if ((strchr(delim, s[i]) == NULL))
		{
			// Check if counter is less than limit
			if (c < ti)
			{
				// Get token
				for (is = i - (flm != NULL); i < l; ++i)
				{
					// Check if limiter is assigned
					if (flm != NULL)
					{
						// Check if end of field limiter (limiter assigned)
						if ((s[i] == flm[1]) && (s[i - 1] != '\\'))
						{
							if (nl > 0) nl--;
							else
							{
								flm = NULL;
								
								// Check if next character is part of inclusion characters
								if ((s[i + 1] == '\0') || (strchr(suffix, s[i + 1]) == NULL))
									break;
							}
						}
						else if ((s[i] == flm[0]) && (s[i - 1] != '\\')) nl++;
					}
					
					// Check if start of field limiter (limiter not assigned)
					else if (strchr(fdelim, s[i]) != NULL)
					{
						flm = (char *) strchr(fdelim, s[i]);
						if (i - is == 0) c--;
						break;
					}
					// Check if delimiter
					else if ((strchr(delim,  s[i]) != NULL)
					&&       (strchr(suffix, s[i]) == NULL))
						break;
				}
				// Increment counter
				c++;
			}
			
			// Exit loop
			else break;
		}
	}
	
	// Exit code
	if (i >= l) return NULL;
	else        return (char *) s + i;
}

// Check if string can be converted to integer number
int isinteger(const char *s)
{
	// Flags for symbols at beginning of string
	unsigned int
	ishex = (strstr(s, "0x") == s),
	isbin = (strstr(s, "0b") == s),
	isneg = (strchr(s, '-') == s);
	
	// Check if null pointer
	if (s == NULL) return 0;
	
	// Check for length of string
	if (!strlen(s)) return 0;
	
	// Check if boolean values
	if ((!strcmp(s, "true")) || (!strcmp(s, "false")))
		return 1;
	
	// Check if character
	else if ((strlen(s) == 3)
	&&       (s[0] == '\'')
	&&       (s[1] != '\\')
	&&       (s[2] == '\''))
		return 1;
	
	// Check if character 2: electric boogaloo
	else if ((strlen(s) == 4)
	&&       (s[0] == '\'')
	&&       (s[1] == '\\')
	&&       (s[3] == '\''))
	{
		switch (s[2])
		{
			case 'a':
			case 'b':
			case 't':
			case 'n':
			case 'r':
				return 1;
			default:
				return 0;
		}
	}
	
	// Loop characters to check if string can be converted to number
	else for (unsigned int i = (2 * (ishex || isbin)) + isneg, l = strlen(s); i < l; ++i)
	{
		if ((i == 0) && (s[0] == '-'))
			continue;
		
		// Hexadecimal
		if (ishex)
		{
			if (!isxdigit(s[i]))
				return 0;
		}
		// Binary
		else if (isbin)
		{
			switch (s[i])
			{
				case '1':
				case '0':
					break;
				default:
					return 0;
			}
		}
		// Number
		else if (!isdigit(s[i]))
			return 0;
	}
	
	// Sucess
	return 1;
}

// Check if string can be converted to presicion number
int isprecise(const char *s)
{
	// Flags for symbols at beginning of string
	unsigned int
	isneg = (strchr(s, '-') == s),
	issuf = (strchr(s, 'f') == s + strlen(s) - 1) || (strchr(s, 'd') == s + strlen(s) - 1);
	
	// Check if null pointer
	if (s == NULL) return 0;
	
	// Check for length of string
	if (!strlen(s)) return 0;
	
	// Check for decimal dot
	else if (!issuf && (strchr(s, '.') == NULL))
		return 0;
	
	// Check for position of decimal point
	else if (s[strlen(s) - 1] == '.')
		return 0;
	
	// Check if there are more than one point symbol
	else if (strchrcount(s, '.') > 1)
		return 0;
	
	// Loop characters to check if string can be converted to number
	else for (unsigned int i = isneg, l = strlen(s); i < l - issuf; ++i)
	{
		// Check for digit
		if (isdigit(s[i]))
			continue;
		
		// Check for decimal point
		else if (s[i] == '.')
			continue;
		
		// Check for type suffix
		else if (i == strlen(s) - 1)
		{
			switch (s[strlen(s) - 1])
			{
				case 'f':
				case 'd':
					break;
				default:
					return 0;
			}
		}
		
		else return 0;
	}
	
	// Return success
	return 1;
}

// Convert string into integer
long long int stoi(const char *s)
{
	long long int value = 0;
	
	// Flags for symbols at beginning of string
	unsigned int
	ishex = (strstr(s, "0x") == s),
	isbin = (strstr(s, "0b") == s),
	isneg = (strchr(s, '-') == s);
	
	// Check for length of string
	if (!strlen(s))
		return 0;
	
	// Boolean values
	else if (!strcmp(s, "true"))  return 1;
	else if (!strcmp(s, "false")) return 0;
	
	// Return as number
	else if (strlen(s) == 1)
		return s[0] - '0';
	
	// Return as character
	// Check if character
	else if ((strlen(s) == 3)
	&&       (s[0] == '\'')
	&&       (s[1] != '\\')
	&&       (s[2] == '\''))
		return (int) s[1];
	
	// Check if character 2: electric boogaloo
	else if ((strlen(s) == 4)
	&&       (s[0] == '\'')
	&&       (s[1] == '\\')
	&&       (s[3] == '\''))
	{
		switch (s[2])
		{
			case 'a': return 7;
			case 'b': return 8;
			case 't': return 9;
			case 'n': return 10;
			case 'r': return 13;
			default: return s[2];
		}
	}
	
	// Check if hex character
	else if ((strlen(s) == 6)
	&&       (s[0] == '\'')
	&&       (s[1] == '\\')
	&&       (s[5] == '\''))
	{
		if (s[2] == 'x')
		{
			char shex[4];
			
			strcpy(shex, "0x");
			strncat(shex, s + 3, 2);
			return stoi(shex);
		}
	}
	
	// Loop characters to convert string to number
	else for (unsigned int i = (2 * (ishex || isbin)) + isneg, l = strlen(s); i < l; ++i)
	{
		// Hexadecimal
		if (ishex)
		{
			// Note: Switch-cases may not be portable
			switch (s[i])
			{
				// Numerical
				case '0': case '1':
				case '2': case '3':
				case '4': case '5':
				case '6': case '7':
				case '8': case '9':
					value += (s[i] - '0') << ((l - i - 1) * 4);
					break;
				
				// Alphabetical
				case 'a': case 'b':
				case 'c': case 'd':
				case 'e': case 'f':
					value += (10 + s[i] - 'a') << ((l - i - 1) * 4);
					break;
				case 'A': case 'B':
				case 'C': case 'D':
				case 'E': case 'F':
					value += (10 + s[i] - 'A') << ((l - i - 1) * 4);
					break;
			}
		}
		// Binary
		else if (isbin)
		{
			// Note: Switch-cases may not be portable
			switch (s[i])
			{
				case '1':
				case '0':
					value += (s[i] - '0') << (l - i - 1);
					break;
			}
		}
		// Number
		else
			value += (s[i] - '0') * (int) pow(10, l - i - 1);
	}
	
	// Success
	return (isneg) ? -value : value;
}

// Convert string into double number
double stod(const char *s)
{
	double value = 0;
	
	// Flags for symbols at string
	unsigned int
	isneg = (strchr(s, '-') == s),
	issuf = (strchr(s, 'f') == s + strlen(s) - 1) || (strchr(s, 'd') == s + strlen(s) - 1);
	
	// Digit-based exponent
	double xp = (strchr(s, '.') != NULL) ? pow(10, strchr(s, '.') - s - 1 - isneg) : pow(10, strlen(s) - 1 - isneg - issuf);
	
	// Check for length of string
	if (!strlen(s)) return 0;
	
	// Check for position of decimal point
	else if (s[strlen(s) - 1] == '.')
		return 0;
	
	// Check if there are more than one point symbol
	else if (strchrcount(s, '.') > 1)
		return 0;
	
	// Loop characters to check if string can be converted to number
	else for (unsigned int i = isneg, l = strlen(s); i < l; ++i)
	{
		// Check for digit
		if (isdigit(s[i]))
			value += (s[i] - '0') * xp;
		
		// Check for decimal point
		else if (s[i] == '.')
			continue;
		
		// Check for type suffix
		else if (i == strlen(s) - 1)
		{
			switch (s[strlen(s) - 1])
			{
				case 'f':
				case 'd':
					continue;
				default:
					return 0;
			}
		}
		
		// Failure 
		else return 0;
		
		// Divide digit exponent by 10
		xp /= 10;
	}
	
	// Return success
	return (isneg) ? -value : value;
}

// Remove whitespace at string ends
int strip(char *s)
{
	size_t offset = 0;
	
	// Remove whitespace from end of string
	while (isspace(s[strlen(s) - 1]))
		s[strlen(s) - 1] = '\0';
	
	// Remove whitespace from start of string
	while (isspace(s[offset]))
		offset++;
	
	// Shift characters by copying string to lower index
	for (unsigned int i = 0, l = strlen(s); i + offset - 1 < l; i++)
		s[i] = s[i + offset];
	
	return 0;
}

// Remove parts of string through comments
int strcom(char *s, struct compiledata *cdata)
{
	// If multiline remains
	if ((cdata->f_mlcom) && (strstr(s, ">_") == NULL))
		s[0] = '\0';
	
	// Check for multiline comment
	else 
	while (((strstr(s, "_<") != NULL) && (strnchrcount(s, '"', strstr(s, "_<") - s) % 2 == 0) && (strnchrcount(s, '\'', strstr(s, "_<") - s) % 2 == 0) && (!cdata->f_mlcom))
	||     ((strchr(s, '#')  != NULL) && (strnchrcount(s, '"', strchr(s,  '#') - s) % 2 == 0) && (strnchrcount(s, '\'', strchr(s,  '#') - s) % 2 == 0) && (!cdata->f_mlcom))
	||     ((strstr(s, ">_") != NULL) && (cdata->f_mlcom)))
	{
		// Check for end of multi-line comment
		if (cdata->f_mlcom)
		{
			if (strstr(s, ">_") != NULL)
			{
				strcpy(s, strstr(s, ">_") + 2);
				cdata->f_mlcom = 0;
			}
			else s[0] = '\0';
		}
		
		// Set single line character to null
		// (If not inside multi-line comment)
		if ((!cdata->f_mlcom)
		&& (strchr(s, '#') != NULL))
			strchr(s, '#')[0] = '\0';
		
		// Check for start of multi-line comment
		if (!cdata->f_mlcom)
			if (strstr(s, "_<") != NULL)
			{
				if (strstr(s, ">_") != NULL)
					strcpy(strstr(s, "_<"), strstr(s, ">_") + 2);
				else 
				{
					strstr(s, "_<")[0] = '\0';
					cdata->f_mlcom = 1;
				}
			}
	}
	
	return 0;
}
