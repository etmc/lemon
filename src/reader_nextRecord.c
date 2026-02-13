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

#include <stdio.h>
#include <lemon.h>

#include "internal_readAndParseHeader.static"

int lemonReaderNextRecord(LemonReader *reader)
{
  int err = 0;

  if (reader == (LemonReader *)NULL)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderNextRecord:\n"
                    "        NULL pointer or uninitialized reader provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  if (!reader->is_awaiting_header)
    lemonReaderCloseRecord(reader);

  err = readAndParseHeader(reader);

  /* readAndParseHeader will produce debug output */
  if (err != LEMON_SUCCESS)
    return err;

  reader->is_last = reader->curr_header->ME_flag;
  reader->is_awaiting_header = 0;

  return LEMON_SUCCESS;
}
