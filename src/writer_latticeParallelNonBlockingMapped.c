#include <config.h>
#include <lemon.h>
#include <stdio.h>

#include "internal_clearWriterState.static"
#include "internal_setupIOType.static"
#include "internal_freeIOType.static"

int lemonWriteLatticeParallelNonBlockingMapped(LemonWriter *writer, void *data,
                                               MPI_Offset siteSize, int *latticeDims, int const *mapping)
{
  int        read;
  int        error;
  MPI_Status status;
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
  MPI_File_set_view(*writer->fp, writer->off + writer->pos, etype, ftype, "native", MPI_INFO_NULL);

  /* Blast away! */
  err = MPI_File_write_at_all_begin(*writer->fp, writer->pos, data, localVol, etype);
  writer->is_busy = 1;
  writer->is_collective = 1;
  writer->buffer = data;
  writer->bytes_wanted = totalVol * siteSize;

  MPI_Barrier(writer->cartesian);

  /* Free up the resources we claimed for this operation. */
  lemonFreeIOTypes(&setup);

  if (err != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteLatticeParallelNonBlocking:\n"
                    "        MPI_File_write_at_all_begin return error %d.\n", writer->my_rank, err);
    return LEMON_ERR_WRITE;
  }

  return LEMON_SUCCESS;
}
