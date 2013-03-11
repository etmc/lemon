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

#include "internal_padding.static"

int lemonReaderCloseRecord(LemonReader *reader)
{
  if (reader == (LemonReader*)NULL)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderCloseRecord:\n"
                    "        NULL pointer provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  if (reader->is_awaiting_header)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderCloseRecord:\n"
                    "        reader is not initialized.\n", reader->my_rank);
    return LEMON_ERR_CLOSE;
  }

  if (reader->is_busy)
    lemonFinishReading(reader);

  MPI_Barrier(reader->cartesian);
  reader->off += reader->curr_header->data_length + lemon_padding(reader->curr_header->data_length);
  reader->pos = 0;
  reader->is_awaiting_header = 1;

  return LEMON_SUCCESS;
}
