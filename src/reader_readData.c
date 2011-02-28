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
#include <stdio.h>

int lemonReaderReadData(void *dest, MPI_Offset *nbytes, LemonReader *reader)
{
  MPI_Status status;
  int err;
  int read;

  if ((reader == (LemonReader*)NULL) || (dest == NULL))
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderReadData:\n"
                    "        NULL pointer or uninitialized reader provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  err = MPI_File_read_at_all(*reader->fp, reader->off + reader->pos, dest, *nbytes, MPI_BYTE, &status);
  MPI_Barrier(reader->cartesian);

  if (err != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderReadData:\n"
                    "        MPI_File_read_at_all returned error code %d.\n", reader->my_rank, err);
    return LEMON_ERR_READ;
  }

  MPI_Get_count(&status, MPI_BYTE, &read);
  *nbytes = (uint64_t)read;
  reader->pos += *nbytes;

  return LEMON_SUCCESS;
}
