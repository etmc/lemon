#include <config.h>
#include <lemon.h>
#include <stdio.h>

int lemonReadLatticeParallelMapped(LemonReader *reader, void *data,
		           MPI_Offset siteSize, int *latticeDims, int const *mapping)
{
  MPI_Datatype etype;
  MPI_Datatype ftype;
  MPI_Status   status;

  int idx = 0;
  int ndims = 0;
  int localVol = 1;
  int totalVol = 1;
  int read;
  int *starts;
  int *localDims;
  int *mappedDims;
  int *mpiDims;
  int *mpiCoords;
  int *period;

  /* Deal with reader state */
  if (reader == (LemonReader*)NULL)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReadLatticeParallel:\n"
                    "        NULL pointer provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  if (reader->is_awaiting_header)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReadLatticeParallel:\n"
                    "        uninitialized reader provided.\n", reader->my_rank);
    return LEMON_ERR_READ;
  }

  if (reader->is_busy)
    lemonFinishReading(reader);

  /* Set up data types and file view */
  /* Elementary datatype for reading/writing: a single lattice site of the correct size.
     Note that we always assume we obtained this as raw binary data of the correct type,
     because apparently we cannot trust MPI libraries to deal with conversions internally. */
  MPI_Type_contiguous(siteSize, MPI_BYTE, &etype);
  MPI_Type_commit(&etype);

  /* Gathering of the required MPI data from the cartesian communicator. */
  MPI_Cartdim_get(reader->cartesian, &ndims);
  starts = (int*)malloc(ndims * sizeof(int));
  localDims = (int*)malloc(ndims * sizeof(int));
  mappedDims = (int*)malloc(ndims * sizeof(int));
  mpiDims = (int*)malloc(ndims * sizeof(int));
  mpiCoords = (int*)malloc(ndims * sizeof(int));
  period = (int*)malloc(ndims * sizeof(int));
  MPI_Cart_get(reader->cartesian, ndims, mpiDims, period, mpiCoords);
  free(period);

  /* Calculation of local lattice dimensions from the MPI data we obtained. */
  for (idx = 0; idx < ndims; ++idx)
  {
    mappedDims[idx] = latticeDims[mapping[idx]];
    localDims[idx]  = mappedDims[idx] / mpiDims[mapping[idx]];
    localVol       *= localDims[idx];
    totalVol       *= mappedDims[idx];
    starts[idx]     = localDims[idx] * mpiCoords[mapping[idx]];
  }

  /* Build up a filetype that provides the right offsets for the reading of a N-dimensional lattice. */
  MPI_Type_create_subarray(ndims, mappedDims, localDims, starts, MPI_ORDER_C, etype, &ftype);
  MPI_Type_commit(&ftype);

  /* Install the data organization we worked out above on the file as a view.
     We keep the individual file pointers synchronized explicitly, so assume they are here. */
  MPI_File_set_view(*reader->fh, reader->off + reader->pos,
                     etype, ftype, "native", MPI_INFO_NULL);

  /* Blast away! */
  MPI_File_read_at_all(*reader->fh, reader->pos, data, localVol, etype, &status);
  MPI_Barrier(reader->cartesian);

  MPI_Get_count(&status, MPI_BYTE, &read);

  reader->pos += totalVol * siteSize;

  /* We want to leave the file in a well-defined state, so we reset the view to a default. */
  /* We don't want to reread any data, so we maximize the file pointer globally. */
  MPI_Barrier(reader->cartesian);
  MPI_File_set_view(*reader->fh, 0, MPI_BYTE, MPI_BYTE, "native", MPI_INFO_NULL);

  /* Free up the resources we claimed for this operation. */
  MPI_Type_free(&etype);
  MPI_Type_free(&ftype);
  free(starts);
  free(localDims);
  free(mappedDims);
  free(mpiDims);
  free(mpiCoords);

  /* Doing a data read should never get us to EOF, only header scanning */
  if (read != siteSize * localVol)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReadLatticeParallel:\n"
                    "        Could not read the required amount of data.\n", reader->my_rank);
    return LEMON_ERR_READ;
  }

  return LEMON_SUCCESS;
}
