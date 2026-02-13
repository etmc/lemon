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

#include <lemon.h>
#include "internal_writeRecordBinaryHeader.static"

int lemonWriteRecordHeader(LemonRecordHeader const *props, LemonWriter* writer)
{
  int result;

  if ((writer == (LemonWriter *)NULL) || (props == (LemonRecordHeader *)NULL))
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteRecordHeader:\n"
                    "        NULL pointer provided.\n", writer->my_rank);
    return LEMON_ERR_PARAM;
  }

  /* Make sure header is expected now */
  if (!writer->is_awaiting_header)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteRecordHeader:\n"
                    "        Writer not ready for header.\n", writer->my_rank);
    return LEMON_ERR_HEADER_NEXT;
  }

  if (writer->is_busy)
    lemonFinishWriting(writer);

  /* If last record ended a message, this one must begin a new one */
  /* If last record did not end a message, this one must not begin one */
  /* Since we allow appending to a file and we don't reread it to check
     the state of the last flag, we don't do this check for the first
     record written. */
  if (!writer->is_first_record && writer->is_last_written != props->MB_flag )
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteRecordHeader:\n"
                    "        message configuration inconsistent.\n", writer->my_rank);
    return LEMON_ERR_MBME;
  }

  result = writeLemonRecordBinaryHeader(props, writer);

  /* Set new writer state */
  writer->is_awaiting_header = 0;
  writer->is_first_record = 0;
  writer->is_last      = props->ME_flag;
  writer->is_last_written = 0;
  writer->data_length  = props->data_length;

  return result;
}
