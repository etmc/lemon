/*****************************************************************************
 * LEMON v1.1                                                                *
 *                                                                           *
 * This file is part of the LEMON implementation of the SCIDAC LIME format.  *
 *                                                                           *
 * It is based directly upon the original c-lime implementation,             *
 * as maintained by C. deTar for the USQCD Collaboration,                    *
 * and inherits its license model and parts of its original code.            *
 *                                                                           *
 * LEMON is free software: you can redistribute it and/or modify             *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * LEMON is distributed in the hope that it will be useful,                  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details. You should have received     *
 * a copy of the GNU General Public License along with LEMON. If not,        *
 * see <http://www.gnu.org/licenses/>.                                       *
 *                                                                           *
 * LEMON was written for the European Twisted Mass Collaboration.            *
 * For support requests or bug reports, please contact                       *
 *    A. Deuzeman (deuzeman@itp.unibe.ch)                                    *
 *****************************************************************************/

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
