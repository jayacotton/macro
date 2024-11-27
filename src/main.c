#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "macro.h"
#include <stdlib.h>
#include <malloc.h>
#include <getopt.h>

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
  printf ("macro version 2.0 %s %s\n", __DATE__, __TIME__);
#else
  printf ("macro version 2.0\n");
#endif
}

void
print_usage ()
{
  printf ("usage\n");
  exit (0);
}

  char arch[80];
  char libroot[80];
int
main (int argc, char *argv[])
{
  MACRO_RECORD *cc;
  int i;
  int c;
  int digit_optind = 0;
  char listfilename[80];
  char outfilename[80];
  char filename[80];
  int flagi;
  int flago;
  int flagl;
  int flagLib;
  int flagarch;


#ifndef CPM
  extern int level;
  level = 0;
#else
  heapinit (10000);
  srand (8520474);
#endif
//mtrace();
  memset (filename, 0, 80);
  memset (outfilename, 0, 80);
  memset (listfilename, 0, 80);
  memset (arch, 0, 80);
  memset (libroot, 0, 80);

  flagi = flago = flagl = flagLib = flagarch = 0;

  while (1)
    {
      int this_option_optind = optind ? optind : 1;
      int option_index = 0;
      static struct option long_options[] = {
	{"in", required_argument, 0, 0},
	{"out", required_argument, 0, 0},
	{"list", required_argument, 0, 0},
	{"version", no_argument, 0, 0},
	{"library", required_argument, 0, 0},
	{"arch", required_argument, 0, 0}
      };

      c = getopt_long (argc, argv, "iolLva", long_options, &option_index);
      if (c == -1)
	break;

      switch (c)
	{
	case 'a':		// arch 8080/z80/pdp11/68000 etc.
	      strcpy (arch, argv[optind++]);
	      flagarch++;
	  break;
	case 'i':		// input file name
	      strcpy (filename, argv[optind++]);
	      flagi++;
	  break;
	case 'o':		// output file name
	      strcpy (outfilename, argv[optind++]);
	      flago++;
	  break;
	case 'l':		// list file name       
	      strcpy (listfilename, argv[optind++]);
	      flagl++;
	  break;
	case 'L':		// macro library root directory
	      strcpy (libroot, argv[optind++]);
	      flagLib++;
	  break;
	case 'v':		// version
	  print_version ();
	  break;
	default:
	  break;
	}
    }
  if (optind < argc)
    {
      printf (" non - option ARGV - elements:");
      while (optind < argc)
	printf ("%s", argv[optind++]);
      printf ("\n");
    }

// fill in the blanks 
  if (!flagi)
    print_usage ();

  if (!flago)
    strcpy (outfilename, filename);
  if (!flagl)
    strcpy (listfilename, filename);

  strcat (filename, Dmac);
  strcat (outfilename, Dout);
  strcat (listfilename, Dlst);

  /* for now skip all to good stuff */

  infile = fopen (filename, "r");

  if (!infile)
    {
      printf (" Can 't open %s\n", filename);
      exit (2);
    }
  listfile = fopen (listfilename, "w");
  if (!listfile)
    {
      printf ("Can' t make a list file \n ");
      exit (2);
    }
  outfile = fopen (outfilename, "w");
  if (!outfile)
    {
      printf (" Can 't make an output file\n");
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
fprintf(listfile,"Macro library root: %s\n",libroot);
fprintf(listfile,"Archatectur: %s\n",arch);
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
