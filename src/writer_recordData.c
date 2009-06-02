#include <config.h>
#include <lemon.h>
#include <stdio.h>

int lemonWriteRecordData(void *source, uint64_t *nbytes, LemonWriter* writer)
{
  MPI_Status status;
  int        bytesRead;
  int        err;

  if (source == NULL || nbytes == (uint64_t*)NULL || *nbytes == 0 || writer == (LemonWriter*)NULL)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteRecordData:\n"
                    "        NULL pointer or uninitialized writer provided.\n", writer->my_rank);
    return LEMON_ERR_PARAM;
  }

  if (writer->is_busy)
    lemonFinishWriting(writer);

  if (writer->my_rank == 0)
  {
    err = MPI_File_write_at(*writer->fh, writer->off + writer->pos, source, *nbytes, MPI_BYTE, &status);
    MPI_Get_count(&status, MPI_BYTE, &bytesRead);
    *nbytes = (uint64_t)bytesRead;
  }

  MPI_File_sync(*writer->fh);
  MPI_Bcast(&err, 1, MPI_INT, 0, writer->cartesian);
  MPI_Bcast(nbytes, sizeof(uint64_t), MPI_BYTE, 0, writer->cartesian);
  MPI_Barrier(writer->cartesian);
  writer->pos += *nbytes;

  if (err != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteRecordData:\n"
                    "        MPI_File_write_at returned error %d.\n", writer->my_rank, err);
    return LEMON_ERR_WRITE;
  }
  return LEMON_SUCCESS;
}
