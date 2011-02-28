/*****************************************************************************
 * LEMON v1.00                                                               *
 *                                                                           *
 * This file is part of the LEMON implementation of the SCIDAC LEMON format. *
 *                                                                           *
 * It is based directly upon the original c-lemon implementation,            *
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

#include <config.h>
#include <lemon.h>
#include <string.h>

LemonRecordHeader *lemonCreateHeader(int MB_flag, int ME_flag, char const *type, MPI_Offset reclen)
{
  LemonRecordHeader *result;
  size_t type_length;

  result = (LemonRecordHeader*)malloc(sizeof(LemonRecordHeader));
  if (result == (LemonRecordHeader*)NULL)
    return NULL;

  type_length = strlen(type);
  result->type = (char*)malloc(sizeof(char) * (type_length + 1));
  if (result->type == (char*)NULL)
  {
    free(result);
    return NULL;
  }
  strcpy(result->type, type); /* Will be assumed to be null-terminated from now on */

  /* Fill out the rest of the fields */
  result->ME_flag = ME_flag;
  result->MB_flag = MB_flag;
  result->data_length = (uint64_t)reclen;
  result->lemon_version = LEMON_VERSION;

  return result;
}
