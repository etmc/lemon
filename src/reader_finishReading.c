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
#include "internal_freeIOTypes.static"
#include "internal_freeTypeChain.static"

int lemonFinishReading(LemonReader *reader)
{
  int read;
  MPI_Status status;
  MPI_Offset bytes_read;
  MPI_Offset bytes_wanted;
  char MPImode[] = "native";

  if (!reader->is_busy)
    return LEMON_SUCCESS;

  MPI_File_read_at_all_end(*reader->fp, reader->buffer, &status);
  MPI_Get_count(&status, reader->setup->etype, &read);
  MPI_File_set_view(*reader->fp, reader->off, MPI_BYTE, MPI_BYTE, MPImode, MPI_INFO_NULL);
  MPI_File_seek(*reader->fp, reader->pos, MPI_SEEK_SET);
  
  if (read < 0)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonFinishReading:\n"
                    "        Potential integer overflow in etype count.\n", reader->my_rank);
    return LEMON_ERR_READ;
  }
  
  /* Doing a data read should never get us to EOF, only header scanning */
  if (read != reader->setup->localVol)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonFinishReading:\n"
	    "        Could not read the required amount of data.\n", reader->my_rank);
    fprintf(stderr, "        needed: %lld, read: %lld\n", reader->setup->localVol * reader->setup->esize , read * reader->setup->esize);
    return LEMON_ERR_READ;
  }
  
  bytes_wanted = reader->setup->totalVol * reader->setup->esize;
    
  reader->pos += bytes_wanted;
  if ((bytes_wanted < 0) || (reader->pos < 0))
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonFinishReading:\n"
	    "        Integer overflow in file pointer adjusting.\n", reader->my_rank);
    return LEMON_ERR_READ;
  }
  
  if (reader->setup->striped_flag)
    lemonFreeIOTypes(&reader->setup);
  else
    lemonFreeTypeChain(&reader->setup);

  reader->buffer = NULL;
  reader->is_busy = 0;
  reader->is_striped = 0;

  return LEMON_SUCCESS;
}
