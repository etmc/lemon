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

LemonWriter* lemonCreateWriter(MPI_File *fp, MPI_Comm cartesian)
{
  LemonWriter* result;

  if (fp == (MPI_File*)NULL)
    return NULL;

  result = (LemonWriter *)malloc(sizeof(LemonWriter));
  if (result == (LemonWriter *)NULL)
    return NULL;

  result->fp = fp;

  result->is_awaiting_header = 1;
  result->is_busy = 0;
  result->is_collective = 0;
  result->is_first_record = 1;
  result->is_last = 0;
  result->is_last_written = 0;

  MPI_File_get_position(*(result->fp), &(result->off));
  result->pos = 0;

  MPI_Comm_dup(cartesian, &result->cartesian);
  MPI_Comm_rank(cartesian, &result->my_rank);
  
  result->setup = NULL;

  return result;
}

