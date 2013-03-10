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

#include "internal_LemonSetup.ih"
#include "internal_clearReaderState.static"
#include "internal_setupIOTypes.static"

int lemonReadLatticeParallelNonBlockingMapped(LemonReader *reader, void *data, MPI_Offset siteSize, int const *latticeDims, int const *mapping)
{
  int        error;
  LemonSetup setup;
  MPI_Offset bytes_wanted;

  error = lemonClearReaderState(reader);
  if (error != LEMON_SUCCESS)
    return error;

  lemonSetupIOTypes(&reader->setup, reader->cartesian, siteSize, latticeDims, mapping);

  /* Install the data organization we worked out above on the file as a view.
     We keep the individual file pointers synchronized explicitly, so assume they are here. */
  MPI_File_set_view(*reader->fp, reader->off + reader->pos, reader->setup->etype, reader->setup->ftype, "native", MPI_INFO_NULL);

  /* Blast away! */
  error = MPI_File_read_at_all_begin(*reader->fp, reader->pos, data, reader->setup->localVol, reader->setup->etype);

  reader->is_busy = 1;
  reader->is_striped = 1;
  reader->buffer = data;

  if ((reader->setup->totalVol * reader->setup->esize) < 0 )
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReadLatticeParallelNonBlocking:\n"
                    "        Integer overflow in requested number of bytes.\n", reader->my_rank);
    return LEMON_ERR_READ;
  } 

  /* Check for the occurrence of errors in MPI_File_read_at_all_begin immediately */
  if (error != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReadLatticeParallelNonBlocking:\n"
                    "        MPI_File_read_at_all_begin returned error %d.\n", reader->my_rank, error);
    return LEMON_ERR_READ;
  }
  return LEMON_SUCCESS;
}
