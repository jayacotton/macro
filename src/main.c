#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "macro.h"
#include <stdlib.h>
#include <malloc.h>
//#include <mcheck.h>

extern void Process0 (FILE *);
extern void Process1 (FILE *);

#ifdef CPM
HEAPSIZE (10000);
#endif

void
print_version ()
{
#ifndef CPM
  printf ("macro version 1.0 %s %s\n", __DATE__, __TIME__);
#else
  printf ("macro version 1.0\n");
#endif
}

int
main (int argc, char *argv[])
{
  MACRO_RECORD *cc;
  int i;
  char listfilename[80];
  char outfilename[80];
  char filename[80];

#ifndef CPM
  extern int level;
  level = 0;
#else
  heapinit (10000);
	srand(8520474);
#endif
//mtrace();
  if (argc < 2)
    {
      print_version ();
      printf ("usage: macro file\n");
      exit (1);
    }

  if (argc > 2)
    print_version ();

  memset (filename, 0, 80);
  memset (outfilename, 0, 80);
  memset (listfilename, 0, 80);

  strcpy (filename, argv[1]);
  strcpy (outfilename, argv[1]);
  strcpy (listfilename, argv[1]);
  strcat (filename, Dmac);
  strcat (outfilename, Dout);
  strcat (listfilename, Dlst);

  /* for now skip all to good stuff */

  infile = fopen (filename, "r");

  if (!infile)
    {
      printf ("Can't open %s\n", filename);
      exit (2);
    }
  listfile = fopen (listfilename, "w");
  if (!listfile)
    {
      printf ("Can't make a list file\n");
      exit (2);
    }
  outfile = fopen (outfilename, "w");
  if (!outfile)
    {
      printf ("Can't make an output file\n");
      exit (2);
    }

  /*
   * pass 1, collect all the macro prototypes and store them in memory
   * list
   */

  macro_root = NULL;
  PassState = 0;		/* gather macros */
  GetSource = 0;		/* start reading from a file */
  Process0 (infile);
  /*
   * pass 2, read back through the file and expand inline all the
   * macros that are used
   */

  PassState = 1;		/* Now expand macros */
  GetSource = 0;		/* start with file again */
  rewind (infile);
  Process1 (infile);

  /* clean up and dump stats to list file */

  fprintf (listfile, "-------------------------------------------------\n");
  if (macro_count == 1)
    {
      fprintf (listfile, "Created %d macro\n", macro_count);
    }
  else
    {
      fprintf (listfile, "Created %d macros\n", macro_count);
    }
  cc = macro_root;
  for (i = 0; i < macro_count; i++)
    {
      fprintf (listfile, "%s\thas %d lines\n", cc->name, cc->count);
      cc = cc->next;
    }
  if (include_count == 1)
    {
      fprintf (listfile, "Processed %d include file\n", include_count);
    }
  else if (include_count >= 1)
    {
      fprintf (listfile, "Processed %d include files\n", include_count);
    }

  fprintf (listfile, "Processed %d lines of input text\n", linecount);

  fclose (infile);
  fclose (listfile);
  fclose (outfile);
}
