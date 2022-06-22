#ifndef VERSE_HEADER_MAIN
#define VERSE_HEADER_MAIN

#include <stdio.h>
#include <stdint.h>

#include "macro.h"
#include "cdata.h"
#include "node.h"

// Function processes
int           varproc(struct compiledata *cdata, struct node *root);
struct node* nodeproc(struct compiledata *cdata, struct node *dest, char *args, int autofree);
int         kwordproc(struct compiledata *, FILE *, char *, int);

// File compilation processes
int fileproc(struct compiledata *, FILE *, char *);
int fcompile(struct compiledata *, FILE *);

// Main processes
int cdataconf(struct compiledata *, char *);
int      main(int, char **);

#endif