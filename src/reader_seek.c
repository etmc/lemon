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

int lemonReaderSeek(LemonReader *reader, MPI_Offset offset, int whence)
{
  int err;
  if (reader == (LemonReader*)NULL || reader->is_awaiting_header)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderSeek:\n"
                    "        NULL pointer or uninitialized reader provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  if (reader->is_busy)
    lemonFinishReading(reader);

  if (whence == MPI_SEEK_CUR)
    reader->pos += offset;
  else if (whence == MPI_SEEK_SET)
    reader->pos = offset;
  else if (whence == MPI_SEEK_END)
    reader->pos = reader->curr_header->data_length - offset;
  else
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderSeek:\n"
                    "        Value passed for whence not recognized.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  if ((reader->pos >= reader->curr_header->data_length) || (reader->pos < 0))
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderSeek:\n"
                    "        Value passed for offset brings file pointer outside of current record.\n", reader->my_rank);
    return LEMON_ERR_SEEK;
  }

  err = MPI_File_seek(*reader->fp, reader->off + reader->pos, MPI_SEEK_SET);

  if (err != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderSeek:\n"
                    "        MPI_File_seek returned error code %d.\n", reader->my_rank, err);
    return LEMON_ERR_SEEK;
  }
  return LEMON_SUCCESS;
}
