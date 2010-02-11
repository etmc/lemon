#include <config.h>
#include <lemon.h>
#include <stdio.h>

#include "internal_LemonSetup.ih"
#include "internal_clearReaderState.static"
#include "internal_setupIOTypes.static"
#include "internal_freeIOTypes.static"

int lemonReadLatticeParallelMapped(LemonReader *reader, void *data, MPI_Offset siteSize,
                                   int *latticeDims, int const *mapping)
{
  int        read;
  int        error;
  MPI_Status status;
  LemonSetup setup;

  error = lemonClearReaderState(reader);
  if (error != LEMON_SUCCESS)
    return error;

  lemonSetupIOTypes(&setup, reader->cartesian, siteSize, latticeDims, mapping);

  /* Install the data organization we worked out above on the file as a view.
     We keep the individual file pointers synchronized explicitly, so assume they are here. */
  MPI_File_set_view(*reader->fp, reader->off + reader->pos, setup.etype, setup.ftype, "native", MPI_INFO_NULL);

  /* Blast away! */
  MPI_File_read_at_all(*reader->fp, reader->pos, data, setup.localVol, setup.etype, &status);
  MPI_Barrier(reader->cartesian);

  /* Synchronize the file pointer */
  MPI_Get_count(&status, MPI_BYTE, &read);
  reader->pos += setup.totalVol * siteSize;

  /* We want to leave the file in a well-defined state, so we reset the view to a default. */
  /* We don't want to reread any data, so we maximize the file pointer globally. */
  MPI_Barrier(reader->cartesian);
  MPI_File_set_view(*reader->fp, 0, MPI_BYTE, MPI_BYTE, "native", MPI_INFO_NULL);

  lemonFreeIOTypes(&setup);

  /* Doing a data read should never get us to EOF, only header scanning -- any shortfall is an error*/
  if (read != siteSize * setup.localVol)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReadLatticeParallel:\n"
                    "        Could not read the required amount of data.\n", reader->my_rank);
    return LEMON_ERR_READ;
  }

  return LEMON_SUCCESS;
}
