/*****************************************************************************
 * LEMON v1.00                                                               *
 *                                                                           *
 * This file is part of the LEMON implementation of the SCIDAC LEMON format. *
 *                                                                           *
 * It is based directly upon the original c-lemon implementation,            *
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
#include "internal_splitSize.static"
#include "internal_freeIOTypes.static"

int lemonWriteLatticeParallelMapped(LemonWriter *writer, void *data, MPI_Offset siteSize, int const *latticeDims, int const *mapping)
{
  int          written;
  int          error;
  int          factor;
  size_t       volfac;
  MPI_Status   status;
  MPI_Datatype factype;
  LemonSetup   setup;
  size_t       total;
  
  error = lemonClearWriterState(writer);
  if (error != LEMON_SUCCESS)
    return error;

  lemonSetupIOTypes(&setup, writer->cartesian, siteSize, latticeDims, mapping);
  total = (size_t)setup.totalVol * (size_t)siteSize;
  
  /* Install the data organization we worked out above on the file as a view */
  MPI_Barrier(writer->cartesian);
  MPI_File_set_view(*writer->fp, writer->off + writer->pos, setup.etype, setup.ftype, "native", MPI_INFO_NULL);

  /* Blast away! */
  MPI_File_write_at_all(*writer->fp, writer->pos, data, setup.localVol, setup.etype, &status);
  MPI_File_sync(*writer->fp);

  MPI_Barrier(writer->cartesian);
  writer->pos += total;

  /* We should reset the shared file pointer, in an MPI_BYTE based view... */
  MPI_Barrier(writer->cartesian);
  MPI_File_set_view(*writer->fp, 0, MPI_BYTE, MPI_BYTE, "native", MPI_INFO_NULL);

  /* Free up the resources we claimed for this operation. */
  lemonFreeIOTypes(&setup);

  /* Workaround to avoid integer overflows for filesizes > 2G, 
   * should be fixed in the 3.0 version of the MPI standard. 
   * Uses a prime factorization to split the volume in smaller chunks. */
  factor = lemonSplitSize(total);
  if (factor == 0)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteLatticeParallel:\n"
                    "        Had issues in factorizing the total volume to fit integer dataype.\n", writer->my_rank);
    return LEMON_ERR_WRITE;
  }
  MPI_Type_contiguous(factor, MPI_BYTE, &factype);
  MPI_Type_commit(&factype);
  MPI_Get_count(&status, factype, &written);
  MPI_Type_free(&factype);
  if (written != (total / factor))
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteLatticeParallel:\n"
                    "        Could not write the required amount of data.\n", writer->my_rank);
    return LEMON_ERR_WRITE;
  }

  return LEMON_SUCCESS;
}
