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

int lemonWriteLatticeParallelMapped(LemonWriter *writer, void *data, MPI_Offset siteSize, int const *latticeDims, int const *mapping)
{
  int        written;
  int        error;
  MPI_Status status;
  
  error = lemonClearWriterState(writer);
  if (error != LEMON_SUCCESS)
    return error;
  
  /* We will pass siteSize by pointer, because we want to scale to the largest contiguous region.
   * Afterwards, and internal to these functions, siteSize will contain the number of bytes in the elementary datatype. */
  lemonSetupIOTypes(&writer->setup, writer->cartesian, siteSize, latticeDims, mapping);
  
  /* Install the data organization we worked out above on the file as a view */
  MPI_Barrier(writer->cartesian);
  MPI_File_set_view(*writer->fp, writer->off + writer->pos, writer->setup->etype, writer->setup->ftype, "native", MPI_INFO_NULL);

  /* Blast away! */
  MPI_File_write_at_all(*writer->fp, writer->pos, data, writer->setup->localVol, writer->setup->etype, &status);
  MPI_File_sync(*writer->fp);

  MPI_Barrier(writer->cartesian);
  MPI_Get_count(&status, writer->setup->etype, &written);
  MPI_Barrier(writer->cartesian);
  MPI_File_set_view(*writer->fp, 0, MPI_BYTE, MPI_BYTE, "native", MPI_INFO_NULL);
  
  if (written < 0)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteLatticeParallelMapped:\n"
                    "        Potential integer overflow in etype count.\n", writer->my_rank);
    return LEMON_ERR_WRITE;
  } 
  
  writer->pos += writer->setup->totalVol * writer->setup->esize;
  if (((writer->setup->totalVol * writer->setup->esize) < 0) || (writer->pos < 0))
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteLatticeParallelMapped:\n"
                    "        Integer overflow in file pointer adjusting.\n", writer->my_rank);
    return LEMON_ERR_WRITE;
  }
    
  if (written != writer->setup->localVol)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteLatticeParallelMapped:\n"
                    "        Could not write the required amount of data.\n", writer->my_rank);
    fprintf(stderr, "        needed: %lld, written: %lld\n", writer->setup->localVol * writer->setup->esize , written * writer->setup->esize);
    return LEMON_ERR_WRITE;
  }

  /* Free up the resources we claimed for this operation. */
  lemonFreeIOTypes(&writer->setup);

  return LEMON_SUCCESS;
}
