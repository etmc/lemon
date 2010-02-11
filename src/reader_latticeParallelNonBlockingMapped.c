#include <config.h>
#include <lemon.h>
#include <stdio.h>

#include "internal_clearReaderState.static"
#include "internal_setupIOType.static"
#include "internal_freeIOType.static"

int lemonReadLatticeParallelNonBlockingMapped(LemonReader *reader, void *data, MPI_Offset siteSize,
                                              int *latticeDims, int const *mapping)
{
  error = lemonClearReaderState(reader);
  if (error != LEMON_SUCCESS)
    return error;

  lemonSetupIOTypes(&setup, reader->cartesian, siteSize, latticeDims, mapping);

  /* Install the data organization we worked out above on the file as a view.
     We keep the individual file pointers synchronized explicitly, so assume they are here. */
  MPI_File_set_view(*reader->fp, reader->off + reader->pos, setup.etype, setup.ftype, "native", MPI_INFO_NULL);

  /* Blast away! */
  err = MPI_File_read_at_all_begin(*reader->fp, reader->pos, data, setup.localVol, setup.etype);

  reader->is_busy = 1;
  reader->is_striped = 1;
  reader->buffer = data;
  reader->bytes_wanted = totalVol * siteSize;

  lemonFreeIOTypes(&status);

  /* Check for the occurrence of errors in MPI_File_read_at_all_begin immediately */
  if (err != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReadLatticeParallelNonBlocking:\n"
                    "        MPI_File_read_at_all_begin returned error %d.\n", reader->my_rank, err);
    return LEMON_ERR_READ;
  }
  return LEMON_SUCCESS;
}
