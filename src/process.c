#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "macro.h"
#include <stdlib.h>

extern FILE *Include (FILE *);
extern void Macro (FILE *, char *);
extern void Mend (void);
extern int WordCheck (char *);

/* process input files */

void
Process0 (FILE * in)
{
  char *gc, *kc;
  char *lbuff;
  /*
   * in the first pass, we collect all the macro text and store it.
   */
  lbuff = calloc (1, 80);
  if (lbuff == 0)
    {
      printf ("Memory alloc issue\n");
      exit (1);
    }
  while (1)
    {
      if (!fgets (lbuff, 80, in))
	{
	  if (nest >= 1)
	    {
	      nest--;
	      fclose (in);
	    }
	  return;		/* EOF */
	}
      switch (lbuff[0])
	{
	case ';':		/* comment */
	  goto next_line;
	  break;
	case '%':		/* replace text */
	case '&':		/* location word */
	  break;
	case '#':		/* include file */
/* no you can't nest include files */
	  {
	    FILE *save = infile;
	    buff = lbuff;
	    Process0 (Include (in));
	    infile = save;
	    goto next_line;
	  }
	  break;
	}

      /* process for key words */

      memset (keywordbuffer, 0, 80);
      kc = keywordbuffer;
      gc = lbuff;
      switch (*gc)
	{
	case '%':		/* replace text */
	case '&':		/* location word */
	  break;
	case '\t':
	case ' ':		/* nothing there */
	  while (isblank (*gc))
	    gc++;
	  /* after skip ahead to non blank */
	  /* copy work to keywordbuffer and trim it */
	  while (*gc)
	    {
	      if (*gc == '\n')
		break;
	      if (*gc == '\t')
		break;
	      if (*gc == '\0')
		break;
	      if (*gc != ' ')
		*kc++ = *gc++;
	      else
		gc++;
	    }
	  break;
	default:
	  break;
	}
      if (keywordbuffer[0] != '\0')
	{
	  switch (Find (keywordbuffer))
	    {
	    case MACRO:
	      Macro (in, gc);
	      break;
	    case MEND:
	      Mend ();
	      break;
	    default:
	      break;
	    }
	}
    next_line:
      linecount++;
    }
  ETRACE ();
}

void
Process1 (FILE * in)
{
  char *gc, *kc, *pc;

  /* process macro expansion */
  /* input file is reset */
  /* no need to open includes */
  buff = calloc (1, 80);
  TRACE ();
  while (fgets (buff, 80, in))
    {
      /*
       * look up keywords in column 2 and find macro invocations
       */
      if (buff[0] == '#')
	{
	  expandstate |= LISTASCOM;
	  goto list_line;
	}
      memset (keywordbuffer, 0, 80);
      memset (parameterbuffer, 0, 80);
      kc = keywordbuffer;
      pc = parameterbuffer;
      gc = buff;
      switch (*gc)
	{
	case ';':
	  expandstate = 0;
	  goto list_line;
	  break;
	case '%':		/* replace text */
	case '&':		/* location word */
	  while (!(isblank (*gc)))
	    gc++;
	  while (isblank (*gc))
	    gc++;
	  while (!isblank (*gc))
	    if (*gc != '\n')
	      *kc++ = *gc++;
	    else
	      break;
	  break;
	case '\t':
	case ' ':		/* nothing there */
	  while (isblank (*gc))
	    gc++;
	  while (!isblank (*gc))
	    if (*gc != '\n')
	      *kc++ = *gc++;
	    else
	      break;
	  break;
	case '#':
	  *gc = ':';		/* special test case for cc8080 */
	  break;
	default:
	  while (!isblank (*gc))
	    gc++;
	  while (isblank (*gc))
	    gc++;
	  while (!isblank (*gc))
	    if (*gc != '\n')
	      *kc++ = *gc++;
	    else
	      break;
	  break;
	}
      /*
       * gc may be pointing at parameters to collect up
       */
      while (isblank (*gc))
	gc++;
      strcpy (pc, gc);

      if (keywordbuffer[0] != '\0')
	{
/* special wackadoodle */
	  {
	    char *z;
	    if ((z = strchr (keywordbuffer, '\r')) != 0)
	      *z = '\0';
	  }
	  switch (Find (keywordbuffer))
	    {
	    case MACRO:
	      Macro (in, gc);
	      break;
	    case MEND:
	      Mend ();
	      ListOut (';');
	      break;
	    default:
	      if (!(expandstate & INSIDE))
		{
		  if (WordCheck (keywordbuffer))
		    {
		      ExpandMacro (keywordbuffer);
		      expandstate |= DONTLIST;
		    }
		}
	      break;
	    }
	}
    list_line:

      /*
       * if the INSIDE flag is set, then we list as a comment
       */

      if (expandstate & INSIDE)
	{
	  ListOut (';');
	  LineOut (buff);
	}
      else
	{

	  /*
	   * DONTLIST is used to hide text in the listing
	   * LISTASCOM is used to make a comment line in the
	   * output
	   */

	  if (!(expandstate & DONTLIST))
	    {
	      if (expandstate & LISTASCOM)
		{
		  ListOut (';');
		  expandstate &= ~LISTASCOM;
		}
	      LineOut (buff);
	    }
	  expandstate &= ~DONTLIST;
	}
    }
  ETRACE ();
}
