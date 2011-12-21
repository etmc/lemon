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
#include "internal_splitSize.static"

int lemonFinishReading(LemonReader *reader)
{
  int read;
  int size;
  int factor;
  MPI_Status status;
  char MPImode[] = "native";  
  MPI_Datatype factype;

  if (!reader->is_busy)
    return LEMON_SUCCESS;

  MPI_Comm_size(reader->cartesian, &size);

  MPI_File_read_at_all_end(*reader->fp, reader->buffer, &status);

  reader->pos += reader->bytes_wanted;
  MPI_File_set_view(*reader->fp, reader->off, MPI_BYTE, MPI_BYTE, MPImode, MPI_INFO_NULL);
  MPI_File_seek(*reader->fp, reader->pos, MPI_SEEK_SET);

  factor = lemonSplitSize(reader->bytes_wanted);
  if (factor == 0)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonFinishReading:\n"
                    "        Had issues in factorizing the total volume to fit integer dataype.\n", reader->my_rank);
    return LEMON_ERR_READ;
  }

  MPI_Type_contiguous(factor, MPI_BYTE, &factype);
  MPI_Type_commit(&factype);
  MPI_Get_count(&status, factype, &read);
  MPI_Type_free(&factype);

  /* Doing a data read should never get us to EOF, only header scanning */
  if (read != (reader->is_striped ? reader->bytes_wanted / (size * factor) : (reader->bytes_wanted / factor)))
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonFinishReading:\n"
                    "        Could not read the required amount of data.\n", reader->my_rank);
    return LEMON_ERR_READ;
  }

  reader->bytes_wanted = 0;
  reader->buffer = NULL;
  reader->is_busy = 0;
  reader->is_striped = 0;

  return LEMON_SUCCESS;
}
