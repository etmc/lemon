#include <stdio.h>

static char out[32];

char const *humanForm(unsigned long long int filesize)
{
  static char const *units[] = {"kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"}; /* Optimisim ;) */
  double engFilesize;

  /* Using SI conventions, unambiguous since these are transmission speeds */
  if (filesize < 1000)
  {
    sprintf(out, "%u B", (unsigned int)filesize);
    return out;
  }

  size_t ucnt = 0;
  while ((ucnt < 7) && (filesize / 1000) > 1000 )
  {
    filesize /= 1000;
    ++ucnt;
  }
  engFilesize = filesize / 1000.0;

  filesize /= 1000;
  if (filesize >= 100)
    sprintf(out, "%5.0f %s", engFilesize, units[ucnt]);
  else if (filesize >= 10)
    sprintf(out, "%5.1f %s", engFilesize, units[ucnt]);
  else 
   sprintf(out, "%5.2f %s", engFilesize, units[ucnt]);

  return out;
}
