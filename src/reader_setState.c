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
#include <memory.h>

int lemonReaderSetState(LemonReader *rdest, LemonReader const *rsrc)
{
  MPI_Offset   disp;
  MPI_Datatype etype;
  MPI_Datatype ftype;
  char         drep[32];

  /* Set rdest reader state from rsrc */
  /* We do not copy the file pointer member fp - this needs to be set at construction */
  if (rdest->curr_header == (LemonRecordHeader*)NULL)
    rdest->curr_header = (LemonRecordHeader*)malloc(sizeof(LemonRecordHeader));
  if (!rsrc->is_awaiting_header)
    memcpy(rdest->curr_header, rsrc->curr_header, sizeof(LemonRecordHeader));

  rdest->is_last            = rsrc->is_last;
  rdest->is_awaiting_header = rsrc->is_awaiting_header;
  rdest->is_busy            = 0;
  rdest->is_striped         = 0;

  rdest->off                = rsrc->off;
  rdest->pos                = rsrc->pos;

  /* Now make the system agree with the reader state */
  MPI_File_get_view(*rsrc->fp, &disp, &etype, &ftype, drep);
  MPI_File_set_view(*rdest->fp, disp, etype, ftype, drep, MPI_INFO_NULL);
  MPI_File_seek(*rdest->fp, rdest->pos, MPI_SEEK_SET);

  MPI_Comm_dup(rsrc->cartesian, &rdest->cartesian);
  MPI_Comm_rank(rdest->cartesian, &rdest->my_rank);

  return LEMON_SUCCESS;
}
