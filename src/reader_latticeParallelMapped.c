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
#include "internal_freeIOTypes.static"

int lemonReadLatticeParallelMapped(LemonReader *reader, void *data, MPI_Offset siteSize, int const *latticeDims, int const *mapping)
{
  int        read;
  int        error;
  MPI_Status status;

  error = lemonClearReaderState(reader);
  if (error != LEMON_SUCCESS)
    return error;
  
  lemonSetupIOTypes(&reader->setup, reader->cartesian, siteSize, latticeDims, mapping);

  /* Install the data organization we worked out above on the file as a view.
     We keep the individual file pointers synchronized explicitly, so assume they are here. */
  MPI_File_set_view(*reader->fp, reader->off + reader->pos, reader->setup->etype, reader->setup->ftype, "native", MPI_INFO_NULL);

  /* Blast away! */
  MPI_File_read_at_all(*reader->fp, reader->pos, data, reader->setup->localVol, reader->setup->etype, &status);
  
  MPI_Barrier(reader->cartesian);
  MPI_Get_count(&status, reader->setup->etype, &read);
  MPI_Barrier(reader->cartesian);
  MPI_File_set_view(*reader->fp, 0, MPI_BYTE, MPI_BYTE, "native", MPI_INFO_NULL);

  if (read < 0)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReadLatticeParallelMapped:\n"
                    "        Potential integer overflow in etype count.\n", reader->my_rank);
    return LEMON_ERR_READ;
  } 
  
  /* Synchronize the file pointer */
  reader->pos += reader->setup->totalVol * reader->setup->esize;
  if (((reader->setup->totalVol * reader->setup->esize) < 0) || (reader->pos < 0))
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReadLatticeParallelMapped:\n"
                    "        Integer overflow in file pointer adjusting.\n", reader->my_rank);
    return LEMON_ERR_READ;
  }

  /* Doing a data read should never get us to EOF, only header scanning -- any shortfall is an error */
  if (read != reader->setup->localVol)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReadLatticeParallelMapped:\n"
                    "        Could not read the required amount of data.\n", reader->my_rank);
    fprintf(stderr, "        needed: %lld, read: %lld\n", reader->setup->localVol * reader->setup->esize , read * reader->setup->esize);
    return LEMON_ERR_READ;
  }

  lemonFreeIOTypes(&reader->setup);

  return LEMON_SUCCESS;
}
