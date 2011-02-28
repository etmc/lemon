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

int lemonWriterSeek(LemonWriter *writer, MPI_Offset offset, int whence)
{
  int err;

  if (writer == (LemonWriter*)NULL || writer->is_awaiting_header)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriterSeek:\n"
                    "        NULL pointer or uninitialized writer provided.\n", writer->my_rank);
    return LEMON_ERR_PARAM;
  }

  if (writer->is_busy)
    lemonFinishWriting(writer);

  if (whence == MPI_SEEK_CUR)
    writer->pos += offset;
  else if (whence == MPI_SEEK_SET)
    writer->pos = offset;
  else if (whence == MPI_SEEK_END)
    writer->pos = writer->data_length - offset;
  else
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriterSeek:\n"
                    "        Value passed for whence not recognized.\n", writer->my_rank);
    return LEMON_ERR_PARAM;
  }

  if ((writer->pos >= writer->data_length) || (writer->pos < 0))
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriterSeek:\n"
                    "        Value passed for offset brings file pointer outside of current record.\n", writer->my_rank);
    return LEMON_ERR_SEEK;
  }

  err = MPI_File_seek(*writer->fp, writer->off + writer->pos, MPI_SEEK_SET);
  MPI_Barrier(writer->cartesian);

  if (err != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriterSeek:\n"
                    "        MPI_File_seek returned error %d.\n", writer->my_rank, err);
    return LEMON_ERR_SEEK;
  }
  return LEMON_SUCCESS;
}
