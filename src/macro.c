/* macro preprocessor for assembly code. */

/*
 * This processor is designed to work like cal macro/32, since its my
 * favorite pre processor
 * 
 * The macro processor is esentialy a 2 pass assembler.  In pass one we collect
 * all the things call 'MACRO' into a memory table of text.  Along the way,
 * we do string substitution and replace all the things with %TEXT, with
 * formal parameter strings, supplied by the macro text.
 * 
 * 
 */


/*
 * general user stuff:
 * 
 * macro [-l] <infile[.mac]> [<outfile[.asm]>] [<outfile[.lst]>]
 * 
 */
/*
 * doc r0.0
 * 
 * Include directive:
 * 
 * This is an addition to the macro, it allows macro prototype files to be
 * included from a library directory by name.
 * 
 * #include <name[.mac]>
 * 
 * Name can be a complex text includeing direcories or later remote machines.
 * 
 * The macro text is included into the input stream and processed allong with
 * the primary source text.
 * 
 * Macro prototype:
 * 
 * The macro definition (after the keyword MACRO) is a prototype of the macro.
 * It has the form; <label> <name> <parameters>
 * 
 * During expansion, the label prototype is replaced with the invocation label
 * string.  The macro has to be constructed to account for the situation
 * where label is blank.
 * 
 * Parameters are expanded from the invocation into the prototype macro text as
 * needed.
 * 
 * blank	MACRO	<name>
 * 
 * Macro declares the start of a macro called <name>.  There can not be a label
 * filed for this command, if its present it will be lost.
 * 
 * <name> can be upper case or lower case or any mix.  The parser will find
 * <name> regardless of the string case in the search.  This implies that the
 * <name> must be completely unique for each macro.
 * 
 * The listing will preserve the case of the <name>.
 * 
 * Macro creates a temp storage for the macro called name, and prepares it for
 * latter use.  No particular processing is performed during the storage
 * process.
 * 
 * During the list output and file output, all macro text will be treated as a
 * comment line.  This is done to prevent errors in the assembler stage.
 * 
 * MEND
 * 
 * Mend declares the end of the macro text.  No labels or paramaters are
 * allowed.
 * 
 * AIF directive:
 * 
 * This command does an arithmatic compare between two variables and returns
 * true if equal and false if not equal.
 * 
 * The directive is used to control macro expansion flow through the text file.
 * In addition to the basic AIF command there will also be one of two
 * condition tests, EQ and NEQ.  Based on the result of the string compare,
 * the conditional after the test will be exicuted.
 * 
 * AIF('XYZ' EQ 'ABC') &NOLAB
 * 
 * In this example since XYZ is not equal to ABC then don't goto &NOLAB.
 * 
 * Global Storage
 * 
 * Create global arithmatic storage:
 * 
 * <blank> GBLA <list of parameters>
 * 
 * Create global binary storage:
 * 
 * <blank> GBLB <list of parameters>
 * 
 * Create global charater storage:
 * 
 * <blank> GBLC <list of parameters>
 * 
 * Allocate storage in the global space for the parameters.  All that happens is
 * the storage is created and named for the parameter.  The value is
 * initalized to null.
 * 
 * Local Storage
 * 
 * Create local arithmatic storage: <blank> LCLA	<list of parameters>
 * 
 * Create local binary storage: <blank> LCLB	<list of parameters>
 * 
 * Create local caracter storage: <blank> LCLC	<list of parameters>
 * 
 * Allocate storage in the local space for the parameters.  This is unlike
 * global storage in that the locals are cleared out after the macro closes.
 * 
 * 
 * Setting or assigning values to storage
 * 
 * <lab>	SETA	value  set lab to value string (should be arithmatic) <lab>
 * SETB	value  set lab to a binary value. <lab>	SETC	value  set lab to
 * caracter/string.
 * 
 */
/* headers */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "macro.h"
#include <stdlib.h>

/* because its not in the string.h file */
#ifndef CPM
char *strcasestr (const char *haystack, const char *needle);
#else
char *
strcasestr (const char *s, const char *find)
{
  char c, sc;
  size_t len;
  if ((c = *find++) != 0)
    {
      c = (char) tolower ((unsigned char) c);
      len = strlen (find);
      do
	{
	  do
	    {
	      if ((sc = *s++) == 0)
		return (NULL);
	    }
	  while ((char) tolower ((unsigned char) sc) != c);
	}
      while (strncasecmp (s, find, len) != 0);
      s--;
    }
  return ((char *) s);
}
#endif
/* functions */

void
StackUp (char *buffer)
{

  if (stack_count <= 10)
    {
      stack[stack_count++] = buff;
      buff = buffer;
    }
  else
    {
      printf ("stack overflow\n");
      CloseOut ();
    }

}

char *
StackDown ()
{

  if (stack_count >= 1)
    {
      return (stack[--stack_count]);
    }
  printf ("Stack under flow\n");
  CloseOut ();

  return NULL;
}

void
CloseOut ()
{

  fclose (outfile);
  fclose (listfile);
  fclose (infile);

  exit (99);
}

/*
 * we get here because we found '%text' in the macro prototype we don't care
 * if there is a text field for this symbol, just create the thing.
 */

void
AddLabel (char *symbol, char *text)
{
  SYMBOL *sym;

  if (labels == NULL)
    {
      labels = calloc (1, sizeof (SYMBOL));
      sym = labels;
    }
  else
    {
      sym = labels;
      while (sym->next)
	sym = sym->next;
      sym->next = calloc (1, sizeof (SYMBOL));
      sym = sym->next;
    }
  sym->symbol = calloc (1, strlen (symbol));
  strcpy (sym->symbol, symbol);
  if (text)
    {
      if (strlen (text))
	{
	  sym->string_value = calloc (1, strlen (text));
	  strcpy (sym->string_value, text);
	}
      else
	{
	  sym->string_value = NULL;
	}
    }
  sym->symbol_type = LABEL | STRING;
}

char *
FindLabText (char *symbol)
{
  SYMBOL *thing = FindLabel (symbol);

  if (thing)
    {
      return thing->string_value;
    }
  return NULL;
}

SYMBOL *
FindLabel (char *symbol)
{
  SYMBOL *sym;
  sym = labels;
  if (sym == NULL)
    {
      return NULL;
    }
  while (sym)
    {
      if (!strcmp (sym->symbol, symbol))
	{
	  return (sym);
	}
      sym = sym->next;
    }
  return NULL;
}

void
ClearLabel (void)
{
  SYMBOL *sym;
  SYMBOL *next;

  if (labels == NULL)
    {
      return;
    }
  sym = labels;
  while (sym->next)
    {
      next = sym->next;
      free (sym->symbol);
      if (sym->string_value)
	free (sym->string_value);
      free (sym);
      sym = next;
    }
  labels = NULL;
}

/*
 * so start collecting macro text in the current input file cc points to
 * the possible macro name
 */
void
Macro (FILE * in, char *cc)
{
  if (PassState)
    {
      expandstate |= (LISTASCOM | INSIDE);
    }
  else
    {
      cc = strnonblk (cc);
      StartMacro (in, cc);
    }
}

void
Mend (void)
{

  if (PassState == 0)
    {
      return;
    }

  /* we will never get in here in pass zero */

  expandstate &= ~INSIDE;
  expandstate &= ~LISTASCOM;
  ClearLabel ();		/* these labels are done */
  ClearLocal ();		/* these locals are doen */
}

/* set some values in global or local mem */

void
Seta (char *name, char *text)
{
  SYMBOL *sym;

  if (!(sym = FindLocal (name)))
    {
      if (!(sym = FindGlobal (name)))
	{
	  fprintf (listfile, "Can't find symbolic name %s\n", name);
	}
    }
  sym->string_value = calloc (1, strlen (text));
  strcpy (sym->string_value, text);
}

void
Setb (char *name, char *value)
{
  SYMBOL *sym;

  if (!(sym = FindLocal (name)))
    {
      if (!(sym = FindGlobal (name)))
	{
	  fprintf (listfile, "Can't find symbolic name %s\n", name);
	}
    }
  sym->string_value = calloc (1, strlen (value));
  strcpy (sym->string_value, value);
}

void
Setc (char *name, char *c)
{
  SYMBOL *sym;

  if (!(sym = FindLocal (name)))
    {
      if (!(sym = FindGlobal (name)))
	{
	  fprintf (listfile, "Can't find symbolic name %s\n", name);
	}
    }
  sym->string_value = calloc (1, strlen (c));
  strcpy (sym->string_value, c);
}

/* make a global symbol */

void
Gbla (char *symbol)
{
  SYMBOL *sym;

  if (!globals)
    {
      globals = calloc (1, sizeof (SYMBOL));
      sym = globals;
    }
  else
    {
      sym = globals;
      while (sym->next)
	sym = sym->next;
      sym->next = calloc (1, sizeof (SYMBOL));
      sym = sym->next;
    }
  sym->symbol = calloc (1, strlen (symbol));
  strcpy (sym->symbol, symbol);
  sym->symbol_type = (LABEL | ARITHMATIC);
}

char *
FindGlobText (char *name)
{
  SYMBOL *sym;
  sym = FindGlobal (name);
  if (sym)
    {
      return sym->string_value;
    }
  return NULL;
}

SYMBOL *
FindGlobal (char *name)
{
  SYMBOL *sym;

  if ((sym = globals) == NULL)
    {
      return NULL;
    }
  while (sym)
    {
      if (!strcmp (sym->symbol, name))
	{
	  return (sym);
	}
      sym = sym->next;
    }
  return NULL;
}

void
Gblb (char *symbol)
{
  SYMBOL *sym;

  if (!globals)
    {
      globals = calloc (1, sizeof (SYMBOL));
      sym = globals;
    }
  else
    {
      sym = globals;
      while (sym->next)
	sym = sym->next;
      sym->next = calloc (1, sizeof (SYMBOL));
      sym = sym->next;
    }
  sym->symbol = calloc (1, strlen (symbol));
  strcpy (sym->symbol, symbol);
  sym->symbol_type = (LABEL | NUMBER);
}

void
Gblc (char *symbol)
{
  SYMBOL *sym;

  if (!globals)
    {
      globals = calloc (1, sizeof (SYMBOL));
      sym = globals;
    }
  else
    {
      sym = globals;
      while (sym->next)
	sym = sym->next;
      sym->next = calloc (1, sizeof (SYMBOL));
      sym = sym->next;
    }
  sym->symbol = calloc (1, strlen (symbol));
  strcpy (sym->symbol, symbol);
  sym->symbol_type = (LABEL | CHARACTER);
}

/* make a local symbol */

void
Lcla (char *symbol)
{
  SYMBOL *sym;

  if (!locals)
    {
      locals = calloc (1, sizeof (SYMBOL));
      sym = locals;
    }
  else
    {
      sym = locals;
      while (sym->next)
	sym = sym->next;
      sym->next = calloc (1, sizeof (SYMBOL));
      sym = sym->next;
    }
  sym->symbol = calloc (1, strlen (symbol));
  strcpy (sym->symbol, symbol);
  sym->symbol_type = (LABEL | ARITHMATIC);
}

void
Lclb (char *symbol)
{
  SYMBOL *sym;

  if (!locals)
    {
      locals = calloc (1, sizeof (SYMBOL));
      sym = locals;
    }
  else
    {
      sym = locals;
      while (sym->next)
	sym = sym->next;
      sym->next = calloc (1, sizeof (SYMBOL));
      sym = sym->next;
    }
  sym->symbol = calloc (1, strlen (symbol));
  strcpy (sym->symbol, symbol);
  sym->symbol_type = (LABEL | NUMBER);
}

void
Lclc (char *symbol)
{
  SYMBOL *sym;

  if (!locals)
    {
      locals = calloc (1, sizeof (SYMBOL));
      sym = locals;
    }
  else
    {
      sym = locals;
      while (sym->next)
	sym = sym->next;
      sym->next = calloc (1, sizeof (SYMBOL));
      sym = sym->next;
    }
  sym->symbol = calloc (1, strlen (symbol));
  strcpy (sym->symbol, symbol);
  sym->symbol_type = (LABEL | CHARACTER);
}

void
ClearLocal (void)
{
  SYMBOL *sym, *next;

  sym = locals;
  if (sym == NULL)
    {
      return;
    }

  while (sym)
    {
      next = sym->next;
      free (sym->symbol);
      if (sym->string_value)
	free (sym->string_value);
      free (sym);
      sym = next;
    }
}

SYMBOL *
FindLocal (char *name)
{
  SYMBOL *sym;

  sym = locals;
  if (sym == NULL)
    {
      return NULL;
    }

  while (sym)
    {
      if (!strcmp (sym->symbol, name))
	{
	  return (sym);
	}
      sym = sym->next;
    }
  return NULL;
}

char *
LookUpLocal (char *name)
{
  SYMBOL *sym = FindLocal (name);

  if (sym)
    {
      return (sym->string_value);
    }
  else
    {
      return NULL;
    }
}

/* aif is arithmatic compare a (comp) b */
/* conditions can be =,!=,<,> */
/* the form is <blank> AIF ('a' [cond] 'b') dest */
/* or          <blank> AIF ('a') dest */
int
Aif (char *name, char *string)
{
  char *a = NULL;
  char *aa = NULL;
  char *b = NULL;
  char *d = NULL;
  char cond[24];
  int binary = 0;
  int cd;
  char *cc;
  int res = 0;

  memset (cond, 0, 24);

  cc = string;
  cc = strchr (cc, '(');
  if (!cc)
    goto syntax;

  if (strchr (cc, '\''))
    {
      cc = strchr (cc, '\'');
      if (!cc)
	goto syntax;
      a = ++cc;

      cc = strchr (cc, '\'');
      if (!cc)
	goto syntax;
      *cc++ = '\0';
    }
  else
    {				/* could be testing a binary */
      cc = strchr (cc, '%');
      if (!cc)
	goto syntax;
      a = cc;
      cc = strchr (cc, ')');
      if (!cc)
	goto syntax;
      *cc++ = '\0';
      binary++;
    }

  /* first we will look at the label stack */

  aa = a;			/* save a copy of the pointer, we may need it
				 * soon */
  if (*a == '%')
    a = FindLabText (a);

  /* next we will look at the global stack */

  if (!a)
    {
      a = aa;			/* told ya */
      a = FindGlobText (a);
    }

  if (!binary)
    {
      if (!a)
	{			/* now we know that the label is missing */
	  /* now, this is a the same as ' ' blank */
	  a = "";
	}
    }
  cc = strnonblk (cc);
  if (binary)
    goto getdest1;
  b = cc;
  cc = GetWord (cond, cc);
  cd = Find (cond);

  if (strlen (cond) == 0)
    {
      if (strlen (a))
	{
	  cc = b;
	  b = NULL;
	  goto getdest;
	}
      else
	{
	  return (0);
	}
    }
  if (cd == 0)
    goto syntax;
  b = NULL;

  cc = strchr (cc, '\'');
  if (!cc)
    goto syntax;

  b = ++cc;
  cc = strchr (cc, '\'');
  if (!cc)
    goto syntax;

  *cc++ = '\0';
  if (*b == '%')
    b = FindLabText (b);


  /* get the destination name field and go there */

getdest:
  cc = strchr (cc, ')');
  if (!cc)
    goto syntax;
getdest1:
  cc = strchr (cc, '&');
  if (!cc)
    goto syntax;
  d = cc;

/* advance to a null or a blank 

	cc = strblk(cc);
	*cc = '\0';
*/

  if (d == 0)
    goto syntax;
  if (strlen (d) == 0)
    goto syntax;

  /* need to add more here */
  /* if binary op and its not set yet */
  /* then don't ago anywhare */

  if (binary)
    {
      if (!a)
	{
	  return 0;
	}
    }
  /* test logic and make a decision */

  if (b)
    {
      if (a)
	{
	  res = strcmp (a, b);
	}
      else
	goto syntax;
    }
  else
    {
      if (a)
	{
	  if (strlen (a))
	    {
	      Ago (d);
	    }
	  else
	    {
	      return (0);
	    }
	}
      else
	{
	  return (0);
	}

    }
  switch (cd)
    {
    case EQ:
      if (res == 0)
	Ago (d);
      break;
    case NEQ:
      if (res != 0)
	Ago (d);
      break;
    case GT:
      if (res >= 1)
	Ago (d);
      break;
    case LT:
      if (res <= 1)
	Ago (d);
      break;
    }
  return (res);
syntax:
  fprintf (listfile, "Syntax error AIF, %s\n", string);
  CloseOut ();

  return (0);
}

void
Bif ()
{


}

void
Cif ()
{


}

/* we can for foward or backward in the macro */

MACRO_LINES *
Ago (char *name)
{
  MACRO_LINES *savethis = ln;

  /* startline is set to the beginning of the macro prototype.
     ln is the pointer to the current line of the macro prototype.
     Advance ln until it points to a destination that lines up with
     name.  
   */
  ln = startline;

  /*
   * o.k. search down the macro and find a line where we can start
   * running again
   */

  while (ln)
    {
      if (ln != savethis)
	{			/* skip the current prototype line */
	  if (ln->line[0] == '&')
	    {
	      if (strstr (ln->line, name))
		{		/* bingo */
		  /* on return ln should be the destination one.... */
		  return ln;
		}
	    }
	}
      ln = ln->next_line;
    }
  fprintf (listfile, "Syntax error, missing destination, AGO \n");
  CloseOut ();
  return NULL;
}

void
Anop ()
{


}

void
Sysindex ()
{


}

void
Mnote (char *cc)
{
  char *lc;


  if (PassState)
    {
      while (isblank (*cc))
	cc++;
      if ((lc = strchr (cc, '\'')))
	cc = ++lc;
      if ((lc = strchr (cc, '\'')))
	*lc = '\0';

      fprintf (listfile, "%s\n", cc);
      fprintf (outfile, "%s\n", cc);
    }

}

void
Mexit (void)
{

  if (PassState)
    {
      fprintf (listfile, "Unrecoverable error state\n");
      CloseOut ();
    }

}

void
Eq ()
{


}

void
Neq ()
{


}

FILE *New;
FILE *
Include (FILE * in)
{
  char mylbuff[80];
  char *cc;
  char *gc;


  memset (mylbuff, 0, 80);
  cc = mylbuff;
  gc = &buff[1];

  /* varify that its an include command */

  while (*gc != ' ')
    *cc++ = *gc++;
  if (Find (mylbuff) != INCLUDE)
    {
    error:
      fprintf (listfile, "Bad include directive\n%s\n", buff);

      CloseOut ();
    }
  /* isolate the file name to open */

  memset (mylbuff, 0, 80);
  cc = mylbuff;
  while (*gc == ' ')
    gc++;
  while (*gc != '"')
    gc++;
  gc++;
  while (*gc != '\0')
    {
      if (*gc == '"')
	goto next;
      *cc++ = *gc++;
    }
  goto error;
  /* open the file, and return the handle */

next:

  New = fopen (mylbuff, "r");
  if (!New)
    {
      printf ("Can't open the file %s\n", mylbuff);
      goto error;
    }
  nest++;
  include_count++;

  return (New);
}

int
WordCheck (char *word)
{
  MACRO_RECORD *cc;


  if (!macro_count)
    {

      return 0;
    }
  if (!strlen (word))
    {

      return 0;
    }

  cc = macro_root;
  while (cc)
    {
      if (strcasestr (cc->name, word))
	{
	  if (strlen (cc->name) == strlen (word))
	    {

	      return 1;
	    }
	}
      cc = cc->next;
    }

  return 0;
}

/* return a pointer to a word terminated by a null.
 * find word.  Words are seperated by comma, and there are no non character
 * bytes in the return buffer. no trailing junk ether
 * 
 * A work can be terminated by a common or a null
 */

char *
FindWord (char *in, int index)
{
  char *cc = in;
  char *lc;


  if (*cc == '\"')
    {				/* quote at front of string */
      cc++;
      while (*cc)
	{			/* run down to end of quoted string */
	  if (*cc == '\"')
	    {
	      cc++;
	      *cc = '\0';
	      return cc;
	    }
	  cc++;
	}
      printf ("parseing error in FindWord %s\n", in);
      return 0;
    }
  cc = strnonblk (cc);
  if ((lc = strchr (cc, ',')))
    *lc = '\0';			/* break at comma */
  if ((lc = strchr (cc, '=')))
    *lc = '\0';			/* zap = */

  lc = strblk (cc);
  if (lc)
    {
      if (lc != cc)
	{
	  *lc = '\0';
	}
    }
  return cc;
}

/*
 * so in the macro prototype there may be formal parameters that need to be
 * parsed out, the variables are in buff
 */
/*

This is where we will be adding code to support multiple input parms for a 
macro 
*/

void
ParseParm (char *text, char *values)
{
  char *cc, *lp;
  char *vc, *lc, *lvc;
  char ltext[80];
  char vtext[80];
  int i;


  /* need to protect the input text */
  memset (ltext, 0, 80);
  memset (vtext, 0, 80);
  strcpy (ltext, text);
  strcpy (vtext, values);

  cc = ltext;
  vc = vtext;

/* the text can have a % at the beginning.  This is for a 
label prototype.  Someone else handles that marker so we 
can skip it */

  if (*cc == '%')
    cc++;			/* eat the first one */
/*  Now we can have quite a few input parameters here, we will need to 
work through them one at a time and match up each parameter with a value */
/* note: parameters start with a % and end with , or null.  values are
seperated by a , or end with null.  Its a string list */
/* note: we can have more parameters than values and vice versa.  So no craching
on extra data. */

  if ((cc = strchr (cc, '%')))
    {
      for (i = 0; *cc; i++)
	{
	  lp = FindWord (cc, i);
/* not sure what this is supposed to be doing... */
	  cc += strlen (lp) + 1;
/* cc now points to second arg. etc. */

	  if (*vc == '\"')
	    {			/* quoted string */
	      lvc = FindWord (vc, 0);
	      /* made it vc since findword returns pointer to end of string */
	      AddLabel (lp, lvc);
/* dont see this working at all */
	      vc += strlen (lvc) + 1;
	    }
	  if (*vc == '\'')
	    {
	      lc = vc;
	    }
	  else
	    {
	      lc = FindWord (vc, i);
	    }
	  vc += strlen (lc) + 1;
	  if (strlen (lc) == 0)
	    AddLabel (lp, NULL);
	  else
	    AddLabel (lp, lc);
	}
    }
}

void
ExpandText (MACRO_RECORD * record)
{
  char *cc, *ll, *pp;
  char word[80];
  int globin = 0;
  char sys[9];
  long s;
  char bigbuff[160];
  SYMBOL *lsymb;


#ifndef CPM
  s = random ();
#else
  /* need better way to get this number */
  s = 34567;
#endif
  snprintf (sys, 9, "%06d", (int) s);
  memset (bigbuff, 0, 160);
  lbuff = calloc (1, 80);

  /* ln and startline are globals */

  startline = ln = record->list;

  cc = ln->line;

  /* record points to start of macro, i.e. prototype */
  /* process prototype */

  memset (labelbuffer, 0, 80);
  memset (parameterbuffer, 0, 80);
  memset (word, 0, 80);


/* buff is the input text that starts a macro expansion.
Part of buff has all the input parameters for this expansion.
*/

  ll = buff;
  pp = &parameterbuffer[0];

  if (!isblank (*ll))
    ll = GetWord (labelbuffer, ll);	/* get the label text */

  while (isspace (*ll))
    ll++;

  /* now at macro name */

  ll = GetWord (word, ll);

  /* copy the input parameters to the buffer */

  if (*ll == '\t')
    ll++;

  strcpy (parameterbuffer, ll);
/* done with ll from this point on */


  pp = strchr (parameterbuffer, '\n');
  if (pp)
    *pp = '\0';

  if (parameterbuffer[0] != '\0')
    expandstate |= PARAM;

  /* we need to parse the parameterbuffer and store up the stuff */

  ParseParm (ln->line, parameterbuffer);

  /*
   * so now we have some possible parameter for the prototype so
   * collect them up and put them in the string stack
   */

  /* now skip over the first (prototype) text line */

  ln = ln->next_line;

  /*
   * fetch and process text we are now expanding the macro text from
   * memory, with the buff holding the original text and labelbuffer
   * holding label and paramterbuffer holding parameters
   */

  memset (word, 0, 80);
  cc = GetWord (word, cc);
  if (word[0] == '%')
    {
      if (FindGlobal (word))
	{
	  /* this label is part of a global binary variable */
	  globin++;
	}
      if (!FindLabel (word))
	AddLabel (word, labelbuffer);	/* Note the label symbol */
    }
  while (ln)
    {
      cc = ln->line;
      memset (lbuff, 0, 80);
      memset (bigbuff, 0, sizeof (bigbuff));
      memset (labelbuffer, 0, 80);
      memset (word, 0, 80);
      strcpy (lbuff, cc);
      cc = lbuff;
#ifdef BUGS
      if (strlen (cc) << 1)
	goto printline;
#endif
      /* do this 2 time because there may be 2 of them */

      if (strstr (lbuff, "$sysin"))
	{
	  strncpy (strstr (lbuff, "$sysin"), sys, 6);
	}
      if (strstr (lbuff, "$sysin"))
	{
	  strncpy (strstr (lbuff, "$sysin"), sys, 6);
	}
      memset (word, 0, 80);
      cc = GetWord (word, cc);

      /* save a destination label for later processing */

      if (word[0] == '&')
	{
	  AddLabel (word, NULL);
	  FindLabel (word)->reference = ln;	/* remember macro line */
	  goto nextline;
	}
      if (word[0] == '%')
	{
	  if (FindGlobal (word))
	    {
	      globin++;
	    }
	  if (globin == 0)
	    {
	      lsymb = FindLabel (word);
	      if (lsymb)
		{
		  if (lsymb->string_value)
		    {
		      fprintf (outfile, "%s", lsymb->string_value);
		      fprintf (listfile, "%s", lsymb->string_value);
		    }
#ifdef MOREINFO
		  else
		    {
		      fprintf (outfile, "internal error %d %s\n", __LINE__,
			       __FILE__);
		      fprintf (listfile, "internal error %d %s\n", __LINE__,
			       __FILE__);
		    }
#endif
		}
	      else
		{
		  fprintf (outfile, "%s", labelbuffer);
		  fprintf (listfile, "%s", labelbuffer);
		}
	      fprintf (outfile, "%s", cc);
	      fprintf (listfile, "%s", cc);
	      ListOut ('\n');
	      memset (labelbuffer, 0, 80);
	      goto nextline;
	    }
	}
      /* collect parameters */
      if (Find (word) == MEND)
	{
	  Mend ();
	  goto nextline;
	}
      else if (!cc)
	{
	  goto printline;
	}
      while (isblank (*cc))
	cc++;
      if (*cc)
	{
	  if (globin)
	    {
	      strcpy (labelbuffer, word);
	      cc = GetWord (word, cc);
	    }
	  if (!cc)
	    {
	      fprintf (outfile, "%s not found\n", word);
	      fprintf (listfile, "%s not found\n", word);
	      CloseOut ();
	    }
	  strcpy (parameterbuffer, cc);
	}
      /* lets see if this word is a macro */

      /* need to protect the buff line of text */
      /*
       * some sort of stack and each recursion needs its own buff
       */

      switch (Find (word))
	{
	case MACRO:
	  fprintf (listfile, "Error: can't create macro inside of macro\n");
	  CloseOut ();
	  break;
	case MEND:
	  Mend ();
	  break;
	case AIF:
	  Aif (labelbuffer, parameterbuffer);
	  expandstate |= DONTLIST;
	  break;
	case SETB:
	  Setb (labelbuffer, parameterbuffer);
	  expandstate |= DONTLIST;
	  globin = 0;
	  break;
	case GBLB:
	  Gblb (parameterbuffer);
	  expandstate |= DONTLIST;
	  break;
	case ANOP:
	  Anop ();
	  expandstate |= DONTLIST;
	  break;
	case SYSINDEX:
	  Sysindex ();
	  expandstate |= DONTLIST;
	  break;
	case LCLA:
	  Lcla (parameterbuffer);
	  expandstate |= DONTLIST;
	  break;
	case SETA:
	  Seta (labelbuffer, parameterbuffer);
	  expandstate |= DONTLIST;
	  break;
	case AGO:
	  ln = Ago (parameterbuffer);
	  expandstate |= DONTLIST;
	  break;
	case MNOTE:
	  if (!(expandstate & INSIDE))
	    Mnote (cc);
	  break;
	case MEXIT:
	  if (!(expandstate & INSIDE))
	    Mexit ();
	  break;
	case LCLB:
	  Lclb (parameterbuffer);
	  expandstate |= DONTLIST;
	  break;
	case LCLC:
	  Lclc (parameterbuffer);
	  expandstate |= DONTLIST;
	  break;
	case SETC:
	  Setc (labelbuffer, parameterbuffer);
	  expandstate |= DONTLIST;
	  break;
	case EQ:
	  Eq ();
	  break;
	case NEQ:
	  Neq ();
	  break;
	case GBLA:
	  Gbla (parameterbuffer);
	  expandstate |= DONTLIST;
	  break;
	case GBLC:
	  Gblc (parameterbuffer);
	  expandstate |= DONTLIST;
	  break;
	default:
	  {
	    char *lab;
	    char newlab[80];
	    char *work, *bc;
	    work = calloc (1, strlen (parameterbuffer));
	    bc = NULL;
	    strcpy (work, parameterbuffer);
	    memset (newlab, 0, 80);
	    cc = work;
	    if (strchr (cc, '\t'))
	      {
		*strchr (cc, '\t') = '\0';
	      }
	    if (strchr (cc, '%'))
	      {
		cc = strchr (cc, '%');
		if (strchr (cc, '\t'))
		  {
		    *(strchr (cc, '\t')) = '\0';
		  }
		if (strchr (cc, ' '))
		  {
		    bc = strchr (cc, ' ');
		    *(strchr (cc, ' ')) = '\0';
		  }

		lab = FindLabText (cc);
		strcpy (work, parameterbuffer);
		if (strchr (work, ','))
		  {
		    cc = strchr (work, ',');
		    cc++;
		    cc = strchr (work, '%');
		    if (cc)
		      {
			cc++;
			*cc++ = 's';
			*cc = '\0';
			snprintf (newlab, 80, work, lab);
		      }
		  }
		else
		  {
		    if (lab)
		      {
			strcpy (newlab, lab);
		      }
		  }
		/* bug  after finding lab, we need to check to see
		   if there is text after lab... probably need to worry
		   about befor lab also */
		if (bc)
		  {		/* might be text after the label */
		    while (*bc)
		      {
			switch (*bc)
			  {
			  case ' ':
			    bc++;
			    break;
			  case '\t':
			  case '\n':
			  case ';':
			    goto skipit;
			    break;
			  case '\"':	/* quoted string */
			    {
			      char *qc = bc;
			      bc++;
			      while (*bc)
				{
				  if (*bc == '\"')
				    {
				      bc++;
				      *bc = '\0';
				    }
				}
			      strcat (newlab, qc);
			      goto skipit;
			    }
			    break;
			  default:
			    if (strchr (bc, ' '))
			      *strchr (bc, ' ') = '\0';
			    if (strchr (bc, '\t'))
			      *strchr (bc, '\t') = '\0';
			    if (strchr (bc, ';'))
			      *strchr (bc, ';') = '\0';
			    strcat (newlab, bc);
			    goto skipit;
			    break;
			  }
		      }
		  }
	      skipit:snprintf (bigbuff, 160, "%s\t%s\t%s\t; %s",
			  labelbuffer, word, newlab,
			  ln->line);
		memset (newlab, 0, 80);
		LineOut (bigbuff);
		ListOut ('\n');
		expandstate |= DONTLIST;
	      }
	  }
	  break;
	}
    printline:
      /* last change to fix the imediate parameter text */
      if ((cc = strchr (lbuff, '%')))
	{
	  if ((cc = FindLabText (cc)))
	    {
	      strcpy (strchr (lbuff, '%'), cc);
	    }
	}
      if (!(expandstate & INSIDE))
	{
	}
      /* emit text globs */

      if (!(expandstate & DONTLIST))
	{
	  LineOut (lbuff);
	  ListOut ('\n');
	}
      expandstate &= ~DONTLIST;
    nextline:
      ln = ln->next_line;
    }
  free (lbuff);

}

void
ExpandMacro (char *word)
{
  MACRO_RECORD *cc;


  cc = macro_root;

  while (cc)
    {
      if (strcasestr (cc->name, word))
	{
	  ListOut (';');
	  LineOut (buff);
	  ExpandText (cc);
	  return;
	}
      cc = cc->next;
    }
}

/* Get next character from a file or from a macro storage */

char
GetChar (FILE * in)
{

  if (GetSource)
    {
    }
  else
    {

      return fgetc (in);
    }

  return '\0';
}

/* find a keyword in the key list */
/* keywords may now be in lower case or upper case */

int
Find (char *buff)
{
  int i;


  if (!buff)
    {
      printf ("Null pointer in Find\n");
      exit (3);
    }
  /* since the max instruction len is 7 */
  if (strlen (buff) >= 8)
    return 0;
  for (i = 0; Instructions[i].value; i++)
    {
      if (!strcasecmp (Instructions[i].OP, buff))
	{

	  return (Instructions[i].value);
	}
    }

  return (0);
}

MACRO_RECORD *
CreatSet (char *name)
{
  MACRO_RECORD *cc;


  if (!macro_root)
    {
      cc = macro_root = calloc (1, sizeof (MACRO_RECORD));
    }
  else
    {
      for (cc = macro_root; cc->next; cc = cc->next);
      cc->next = calloc (1, sizeof (MACRO_RECORD));
      cc = cc->next;
    }
  cc->name = name;

  return (cc);
}

/* the key word was macro, now make a macroblock with name */

void
StartMacro (FILE * in, char *gc)
{
  char *name;
  char *cc;
  MACRO_RECORD *record;
  MACRO_LINES *lines;

  /* name is name of macro */
  name = calloc (1, 80);
  cc = name;

  /* gc is raw text of macro */
  name = strcpyblk (cc, gc);
  /* collect name, and creat a storage block */

  record = CreatSet (name);
  macro_count++;

  /* read lines until we get to the end of the definition */

  lines = record->list = calloc (1, sizeof (MACRO_LINES));
  cc = lines->line = calloc (1, 80);
/*
process until EOF or "MEND" is found
*/
  while (1)
    {
      if (fgets (cc, 80, in) == NULL)
	return;

      trimline (cc);

      record->count++;
      if (strcasestr (lines->line, "MEND"))
	{
	  return;
	}
      lines->next_line = calloc (1, sizeof (MACRO_LINES));
      lines = lines->next_line;
      cc = lines->line = calloc (1, 80);
    }
}

/* print listing a output stuff */

void
LineOut (char *buf)
{
  if (buf)
    {
      fprintf (listfile, "%s", buf);
      fprintf (outfile, "%s", buf);
    }
}

/* for now, just print it out */
void
ListOut (char c)
{
  if (PassState)
    {
      fprintf (listfile, "%c", c);
      fprintf (outfile, "%c", c);
    }
}

/* print a comment line */

void
EmitComment (FILE * in)
{
  int c;
  ListOut (';');
  while ((c = GetChar (in)) != '\n')
    {
      ListOut (c);
    }
  ListOut (c);
}

char *
GetWord (char *out, char *in)
{
  char *cc;

  while (isspace (*in))
    in++;

  *out++ = *in++;		/* first non blank */
  for (cc = in; *cc; cc++)
    {
      switch (*cc)
	{
	case ' ':
	case '\t':
	case '\n':
	case '\0':
	  *out = '\0';
	  return (cc);
	}
      *out++ = *cc;
    }
  return NULL;
}

/* Pass 2 text parseing */

void
TextParse (FILE * in)
{
  char buffer[80];
  char *cc;
  char word[80];
  int macrostate = 0;

  /* read entire text file */
  while (1)
    {
    out1:
      memset (buffer, 0, 80);
      cc = fgets (buffer, 80, in);
      if (cc == NULL)
	{
	  return;
	}

      for (cc = buffer; *cc; cc++)
	{
	  if (!isblank (*cc))
	    {
	      if (*cc == ';')
		goto out;
	      memset (word, 0, 80);
	      cc = GetWord (word, cc);
	      /* now do stuff with word */
	      switch (Find (word))
		{
		case MACRO:
		  macrostate = 1;
		  break;
		case MEND:
		  macrostate = 0;
		  ListOut (';');
		  goto out;
		  break;
		default:
		  if (!macrostate)
		    {
		      if (WordCheck (word))
			{
			  ListOut (';');
			  LineOut (buffer);
			  /*
			   * get a copy of the
			   * current text we
			   * need this to
			   * expand the macro
			   */
			  currenttext = buffer;
			  ExpandMacro (word);
			  macrostate = 0;
			  goto out1;
			}
		    }
		  break;
		}
	    }
	}
      if (macrostate)
	ListOut (';');
    out:
      LineOut (buffer);
    }
}

/* parse the text for isolated words */

int
Parse (FILE * in, int c)
{
  int ch;
  int i;
  int ret;


  ch = c;
  memset (buff, 0, 80);
  buff[0] = ch;
  i = 1;

  switch (ch)
    {
    case ';':			/* text is a comment in the source file */
      /* each line starts with a ';' so no */
      /* nesting is allowed */
      while (GetChar (in) != '\n');

      return (0);
      break;
    case '#':
      {
      }
      break;
    default:
      while ((ch = GetChar (in)) != EOF)
	{
	  if (!isblank (ch))
	    {
	      buff[i++] = ch;
	    }
	  else
	    {
	      ret = Find (buff);

	      return (ret);
	    }
	}
      break;
    }


  return (-1);
}
