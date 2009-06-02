#include <config.h>
#include <lemon.h>
#include <stdio.h>

int lemonReadLatticeParallelNonBlocking(LemonReader *reader, void *data,
                                        MPI_Offset siteSize, int *latticeDims)
{
  MPI_Datatype etype;
  MPI_Datatype ftype;

  int idx = 0;
  int ndims = 0;
  int localVol = 1;
  int totalVol = 1;
  int err = 0;
  int *starts;
  int *localDims;
  int *mpiDims;
  int *mpiCoords;
  int *period;

  /* Deal with reader state */
  if (reader == (LemonReader*)NULL)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReadLatticeParallelNonBlocking:\n"
                    "        NULL pointer provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  if (reader->is_awaiting_header)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReadLatticeParallelNonBlocking:\n"
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
  mpiDims = (int*)malloc(ndims * sizeof(int));
  mpiCoords = (int*)malloc(ndims * sizeof(int));
  period = (int*)malloc(ndims * sizeof(int));
  MPI_Cart_get(reader->cartesian, ndims, mpiDims, period, mpiCoords);
  free(period);

  /* Calculation of local lattice dimensions from the MPI data we obtained. */
  for (idx = 0; idx < ndims; ++idx)
  {
    localDims[idx] = latticeDims[idx] / mpiDims[idx];
    localVol *= localDims[idx];
    totalVol *= latticeDims[idx];
    starts[idx] = localDims[idx] * mpiCoords[idx];
  }

  /* Build up a filetype that provides the right offsets for the reading of a N-dimensional lattice. */
  MPI_Type_create_subarray(ndims, latticeDims, localDims, starts, MPI_ORDER_C, etype, &ftype);
  MPI_Type_commit(&ftype);

  /* Install the data organization we worked out above on the file as a view.
     We keep the individual file pointers synchronized explicitly, so assume they are here. */
  MPI_File_set_view(*reader->fh, reader->off + reader->pos,
                     etype, ftype, "native", MPI_INFO_NULL);

  /* Blast away! */
  err = MPI_File_read_at_all_begin(*reader->fh, reader->pos, data, localVol, etype);
  reader->is_busy = 1;
  reader->is_striped = 1;
  reader->buffer = data;
  reader->bytes_wanted = totalVol * siteSize;

  /* Free up the resources we claimed for this operation. */
  MPI_Type_free(&etype);
  MPI_Type_free(&ftype);
  free(starts);
  free(localDims);
  free(mpiDims);
  free(mpiCoords);

  /* Doing a data read should never get us to EOF, only header scanning */
  if (err != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReadLatticeParallelNonBlocking:\n"
                    "        MPI_File_read_at_all_begin returned error %d.\n", reader->my_rank, err);
    return LEMON_ERR_READ;
  }

  return LEMON_SUCCESS;
}
