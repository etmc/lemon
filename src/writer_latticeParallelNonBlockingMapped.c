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
#include "internal_clearWriterState.static"
#include "internal_setupIOTypes.static"
#include "internal_freeIOTypes.static"

int lemonWriteLatticeParallelNonBlockingMapped(LemonWriter *writer, void *data, MPI_Offset siteSize, int const *latticeDims, int const *mapping)
{
  int        error;
  LemonSetup setup;

  error = lemonClearWriterState(writer);
  if (error != LEMON_SUCCESS)
    return error;

  lemonSetupIOTypes(&setup, writer->cartesian, siteSize, latticeDims, mapping);

  /* Install the data organization we worked out above on the file as a view */
  /* We want to start writing wherever the latest write occurred (because of the
     no seeking in LemonWriter files rule this should be consistent). We therefore
     take the global maximum of all MPI_long File filepointers in absolute offsets and
     start our writeout operation from there. */
  MPI_Barrier(writer->cartesian);
  MPI_File_set_view(*writer->fp, writer->off + writer->pos, setup.etype, setup.ftype, "native", MPI_INFO_NULL);

  /* Blast away! */
  error = MPI_File_write_at_all_begin(*writer->fp, writer->pos, data, setup.localVol, setup.etype);
  writer->is_busy = 1;
  writer->is_collective = 1;
  writer->buffer = data;
  writer->bytes_wanted = setup.totalVol * siteSize;

  MPI_Barrier(writer->cartesian);

  /* Free up the resources we claimed for this operation. */
  lemonFreeIOTypes(&setup);

  if (error != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteLatticeParallelNonBlocking:\n"
                    "        MPI_File_write_at_all_begin return error %d.\n", writer->my_rank, error);
    return LEMON_ERR_WRITE;
  }

  return LEMON_SUCCESS;
}
