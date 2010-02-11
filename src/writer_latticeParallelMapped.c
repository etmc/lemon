#include <config.h>
#include <lemon.h>
#include <stdio.h>

#include "internal_LemonSetup.ih"
#include "internal_clearWriterState.static"
#include "internal_setupIOTypes.static"
#include "internal_freeIOTypes.static"

int lemonWriteLatticeParallelMapped(LemonWriter *writer, void *data,
                                    MPI_Offset siteSize, int *latticeDims, int const *mapping)
{
  int        written;
  int        error;
  MPI_Status status;
  LemonSetup setup;

  error = lemonClearWriterState(writer);
  if (error != LEMON_SUCCESS)
    return error;

  lemonSetupIOTypes(&setup, writer->cartesian, siteSize, latticeDims, mapping);

  /* Install the data organization we worked out above on the file as a view */
  MPI_Barrier(writer->cartesian);
  MPI_File_set_view(*writer->fp, writer->off + writer->pos, setup.etype, setup.ftype, "native", MPI_INFO_NULL);

  /* Blast away! */
  MPI_File_write_at_all(*writer->fp, writer->pos, data, setup.localVol, setup.etype, &status);
  MPI_File_sync(*writer->fp);

  MPI_Barrier(writer->cartesian);

  writer->pos += setup.totalVol * siteSize;

  /* We should reset the shared file pointer, in an MPI_BYTE based view... */
  MPI_Barrier(writer->cartesian);
  MPI_File_set_view(*writer->fp, 0, MPI_BYTE, MPI_BYTE, "native", MPI_INFO_NULL);

  /* Free up the resources we claimed for this operation. */
  lemonFreeIOTypes(&setup);

  MPI_Get_count(&status, MPI_BYTE, &written);
  if (written != siteSize * setup.localVol)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteLatticeParallel:\n"
                    "        Could not write the required amount of data.\n", writer->my_rank);
    return LEMON_ERR_WRITE;
  }

  return LEMON_SUCCESS;
}
