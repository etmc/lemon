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

int lemonFinishWriting(LemonWriter *writer)
{
  int written;
  int size;
  MPI_Status status;

  if (!writer->is_busy)
    return LEMON_SUCCESS;

  if (writer->is_collective)
    MPI_File_write_at_all_end(*writer->fp, writer->buffer, &status);
  else if (writer->my_rank == 0)
    MPI_Wait(&writer->request, &status);

  writer->pos += writer->bytes_wanted;
  MPI_File_set_view(*writer->fp, writer->off, MPI_BYTE, MPI_BYTE, "native", MPI_INFO_NULL);
  MPI_File_seek(*writer->fp, writer->pos, MPI_SEEK_SET);

  MPI_Get_count(&status, MPI_BYTE, &written);

  if (writer->is_collective)
  {
    MPI_Comm_size(writer->cartesian, &size);
    if (written != writer->bytes_wanted / size)
    {
      fprintf(stderr, "[LEMON] Node %d reports in lemonFinishWriting:\n"
                      "        Could not write the required amount of data.\n", writer->my_rank);
      fprintf(stderr, "        needed: %d, written: %d\n", writer->bytes_wanted / size , written);
      return LEMON_ERR_WRITE;
    }
  }
  else
  {
    MPI_Bcast(&written, 1, MPI_INT, 0, writer->cartesian);
    if (written != writer->bytes_wanted)
    {
      fprintf(stderr, "[LEMON] Node %d reports in lemonFinishWriting:\n"
                      "        Could not write the required amount of data.\n", writer->my_rank);
      fprintf(stderr, "        needed: %d, written: %d\n", writer->bytes_wanted / size , written);
      return LEMON_ERR_WRITE;
    }
  }

  writer->data_length = 0;
  writer->buffer = NULL;
  writer->is_busy = 0;
  writer->is_collective = 0;

  return LEMON_SUCCESS;
}
