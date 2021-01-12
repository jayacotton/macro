/* structures */

typedef struct Symbol {
	char           *symbol;
	char           *string_value;
	char            char_value;
	int             int_value;
	int             symbol_type;
	void           *reference;	/* pointer that the symbol may
					 * reference */
	struct Symbol  *next;
}               SYMBOL;

typedef struct op_code {
	char           *OP;
	int             value;
}               OP_CODE;

typedef struct macro_lines {
	char           *line;
	struct macro_lines *next_line;
}               MACRO_LINES;

typedef struct macro_record {
	char           *name;	/* name of macro */
	MACRO_LINES    *list;	/* pointer to text */
	int             count;	/* nr lines in text */
	struct macro_record *next;	/* next record */
}               MACRO_RECORD;

/* defines */

#define MACRO 1
#define MEND 2
#define AIF 3
#define SETB 4
#define GBLB 5
#define ANOP 6
#define LAB 7
#define NOLAB 8
#define SYSINDEX 9
#define LCLA 10
#define SETA 11
#define AGO 12
#define MNOTE 13
#define MEXIT 14
#define LCLB 15
#define LCLC 16
#define SETC 17
#define EQ 18
#define NEQ 19
#define GT 26
#define LT 27
#define INCLUDE 20
#define GBLA 22
#define GBLC 23
#define BIF 24
#define CIF 25

#define LABEL 		0x001
#define PARAM 		0x002
#define ASCII 		0x004
#define BIN   		0x008
#define DEC   		0x010
#define INSIDE 		0x020
#define OUTSIDE 	0x040
#define DONTLIST 	0x080
#define ENDMACRO 	0x100
#define LISTASCOM 	0x200

#define STRING 		0x400
#define CHARACTER 	0x800
#define NUMBER 		0x1000
#define ARITHMATIC	0x2000

#ifdef CPM
#define Dmac	".MAC"
#define Dout	".ASM"
#define Dlst	".LIS"
#else
#define Dmac	".mac"
#define Dout	".asm"
#define Dlst	".list"
#endif

extern OP_CODE Instructions[];
extern SYMBOL  *labels;
extern SYMBOL  *globals;
extern SYMBOL  *locals;
extern MACRO_RECORD *macro_root;
extern int macro_count;
extern FILE  *infile;
extern FILE  *outfile;
extern FILE  *listfile;
extern int   nest;
extern int  include_count;
extern char *stack[];
extern int stack_count;
extern char	*buff;
extern char	*lbuff;
extern int      linecount;
extern int      PassState;	
extern int      GetSource;
extern char     *currenttext;
extern char     labelbuffer[];
extern char     keywordbuffer[];
extern char     parameterbuffer[];
extern unsigned int  expandstate;
extern MACRO_LINES    *ln;
extern MACRO_LINES    *startline;

extern void            LineOut(char *);
extern char           *GetWord(char *, char *);
extern int             Find(char *);
extern void            StartMacro(FILE *, char *);
extern void            debug_expandflags(void);
extern SYMBOL         *FindLabel(char *);
extern SYMBOL         *FindLocal(char *);
extern SYMBOL         *FindGlobal(char *);
extern void            ClearLabel();
extern void            ClearLocal();
extern void            ListOut(char);
extern MACRO_LINES *            Ago(char *);
extern void		ExpandMacro(char *);
extern void	ExpandMacroT(char *,char *);
extern void	CloseOut(void);

/* debugging code */
#ifdef DEBUG_TRACE
#define TRACE() { \
extern int level; \
int myi; \
level++;\
for(myi=1;myi<level;myi++) printf("."); \
printf("%s %s %d\n",__FUNCTION__,__FILE__,__LINE__); \
};
#define ETRACE() {extern int level;\
int myi; for(myi=1;myi<level;myi++)printf(".");printf("Leaving %s\n",__FUNCTION__);level--;};
#else
#define TRACE();
#define ETRACE();
#endif

extern char *strnonblk(char *);
extern char *strcpyblk(char *, char *);
extern char *trimline(char *);
extern char *strblk(char *);
