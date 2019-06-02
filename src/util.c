#include <stdio.h>
#include <ctype.h>
#include <string.h>

/*
macro preprocessor utility code.
*/

/* copy string until a blank,eol, null is found */
char *
strcpyblk (char *txt, char *in)
{
  char *out = txt;

  while (*in)
    {				/*until null */
      if (isblank (*in))
	break;
      if (*in == '\n')
	break;
      *txt++ = *in++;
    }
  return out;
}

/* terminat input text lines */
char *
trimline (char *in)
{
  char *lc;
  lc = strchr (in, '\n');
  if (lc)
    *lc = '\0';
  return lc;
}

/* find a non blank in string */
char *
strnonblk (char *in)
{
  while (*in)			/* null */
    {
      if (*in == '\n')
	break;
      if (isblank (*in))
	in++;
      else
	break;
    }
  return in;
}

/* find a null or blank */
char *
strblk (char *in)
{
  while (*in)			/* null */
    {
      if (isblank (*in))
	return in;
      in++;
    }
  return in;
}
