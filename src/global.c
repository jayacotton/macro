#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "macro.h"
#include <stdlib.h>

/* globals */

OP_CODE Instructions[] = {
  {"MACRO", MACRO},
  {"MEND", MEND},
  {"AIF", AIF},
  {"SETB", SETB},
  {"GBLB", GBLB},
  {"GBLA", GBLA},
  {"GBLC", GBLC},
  {"ANOP", ANOP},
  {"LAB", LAB},
  {"NOLAB", NOLAB},
  {"SYSINDEX", SYSINDEX},
  {"LCLA", LCLA},
  {"SETA", SETA},
  {"AGO", AGO},
  {"MNOTE", MNOTE},
  {"MEXIT", MEXIT},
  {"LCLB", LCLB},
  {"LCLC", LCLC},
  {"SETC", SETC},
  {"EQ", EQ},
  {"NE", NEQ},
  {"NEQ", NEQ},
  {"INCLUDE", INCLUDE},
  {"BIF", BIF},
  {"CIF", CIF},
  {"GT", GT},
  {"LT", LT},
  {"", 0}
};

/* the labels are valid for the life of a macro */

SYMBOL *labels = NULL;

/* globales are valid for the entire text stream */

SYMBOL *globals = NULL;

/* locals are valid for the live of the macro */

SYMBOL *locals = NULL;

/* storage for the macro text */

MACRO_RECORD *macro_root = NULL;
int macro_count = 0;

FILE *infile;			/* input file */
FILE *outfile;			/* Output file */
FILE *listfile;			/* List file */
int nest = 0;
int include_count = 0;
char *stack[10];
int stack_count = 0;
char *buff;
char *lbuff;
int linecount = 0;
int PassState;			/* state 0 is collect macros, state 1 is
				 * expand macros */
int GetSource;			/* 0 is get from file, 1 is get from memory */
char *currenttext;
char labelbuffer[80];
char keywordbuffer[80];
char parameterbuffer[80];
unsigned int expandstate = 0;
MACRO_LINES *ln;		/* the current macro line we are processing */
MACRO_LINES *startline;		/* the first line of the macro */

/* prototypes */
void LineOut (char *);
char *GetWord (char *, char *);
int Find (char *);
void StartMacro (FILE *, char *);
void debug_expandflags (void);
SYMBOL *FindLabel (char *);
SYMBOL *FindLocal (char *);
SYMBOL *FindGlobal (char *);
void ClearLabel ();
void ClearLocal ();
void ListOut (char);
void ExpandMacro (char *);
void ExpandMacroT (char *, char *);
void CloseOut (void);

/* debug code */
int level;
