#include <config.h>
#include <lemon.h>
#include <stdio.h>

int lemonWriteRecordDataNonBlocking(void *source, uint64_t nbytes, LemonWriter* writer)
{
  int err;

  if (source == NULL || nbytes == 0 || writer == (LemonWriter*)NULL)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteRecordDataNonBlocking:\n"
                    "        NULL pointer or uninitialized writer provided.\n", writer->my_rank);
    return LEMON_ERR_PARAM;
  }

  if (writer->is_busy)
    lemonFinishWriting(writer);

  if (writer->my_rank == 0)
    err = MPI_File_iwrite_at(*writer->fp, writer->off + writer->pos, source, nbytes, MPI_BYTE, &writer->request);

  writer->is_busy = 1;
  writer->is_collective = 0;
  writer->buffer = source;
  writer->bytes_wanted = (int)nbytes;

  MPI_File_sync(*writer->fp);
  MPI_Bcast(&err, 1, MPI_INT, 0, writer->cartesian);

  if (err != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteRecordDataNonBlocking:\n"
                    "        MPI_File_iwrite_at returned error %d.\n", writer->my_rank, err);
    return LEMON_ERR_WRITE;
  }
  return LEMON_SUCCESS;
}
