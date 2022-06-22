#ifndef VERSE_HEADER_VDATA
#define VERSE_HEADER_VDATA

#include "macro.h"

struct vlist
{
	unsigned int length;
	struct vdata *start, *end;
};

struct vdata
{
	char string[STRING_LENGTH_TOKEN];
	long long int value;
	
	struct vdata *prev, *next;
	struct vlist *list;
};

struct vdata *vdatainit(struct vlist *, char *, long long int);
struct vdata *vdatafind(struct vlist *, const char *);
int           vdatadest(struct vlist *, const char *);
int        vdatadestall(struct vlist *);

#endif