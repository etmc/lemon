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

#include <config.h>
#include <lemon.h>
#include <stdio.h>

int lemonWriteRecordData(void *source, MPI_Offset *nbytes, LemonWriter* writer)
{
  MPI_Status status;
  int        bytesRead;
  int        err;

  if (source == NULL || nbytes == (MPI_Offset*)NULL || *nbytes == 0 || writer == (LemonWriter*)NULL)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteRecordData:\n"
                    "        NULL pointer or uninitialized writer provided.\n", writer->my_rank);
    return LEMON_ERR_PARAM;
  }

  if (writer->is_busy)
    lemonFinishWriting(writer);

  if (writer->my_rank == 0)
  {
    err = MPI_File_write_at(*writer->fp, writer->off + writer->pos, source, *nbytes, MPI_BYTE, &status);
    MPI_Get_count(&status, MPI_BYTE, &bytesRead);
    *nbytes = (uint64_t)bytesRead;
  }

  MPI_File_sync(*writer->fp);
  MPI_Bcast(&err, 1, MPI_INT, 0, writer->cartesian);
  MPI_Bcast(nbytes, sizeof(MPI_Offset), MPI_BYTE, 0, writer->cartesian);
  MPI_Barrier(writer->cartesian);
  writer->pos += *nbytes;

  if (err != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteRecordData:\n"
                    "        MPI_File_write_at returned error %d.\n", writer->my_rank, err);
    return LEMON_ERR_WRITE;
  }
  return LEMON_SUCCESS;
}
