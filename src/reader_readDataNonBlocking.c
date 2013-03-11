/*****************************************************************************
 * LEMON v1.01                                                               *
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

#include <config.h>
#include <lemon.h>
#include <stdio.h>

int lemonReaderReadDataNonBlocking(void *dest, MPI_Offset const *nbytes, LemonReader *reader)
{
  int err;

  if ((reader == (LemonReader*)NULL) || (dest == NULL))
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderReadDataNonBlocking:\n"
                    "        NULL pointer or uninitialized reader provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  err = MPI_File_read_at_all_begin(*reader->fp, reader->off + reader->pos, dest, *nbytes, MPI_BYTE);

  if (err != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderReadDataNonBlocking:\n"
                    "        MPI_File_read_at_all_begin returned error code %d.\n", reader->my_rank, err);
    return LEMON_ERR_READ;
  }
  reader->is_busy = 1;
  reader->is_striped = 0;
  reader->buffer = dest;
  reader->bytes_wanted = *nbytes;

  return LEMON_SUCCESS;
}
