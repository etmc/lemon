#include <config.h>
#include <lemon.h>
#include <stdio.h>

int lemonReaderReadData(void *dest, MPI_Offset *nbytes, LemonReader *reader)
{
  MPI_Status status;
  int err;
  int read;

  if ((reader == (LemonReader*)NULL) || (dest == NULL))
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderReadData:\n"
                    "        NULL pointer or uninitialized reader provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  err = MPI_File_read_at_all(*reader->fp, reader->off + reader->pos, dest, *nbytes, MPI_BYTE, &status);
  MPI_Barrier(reader->cartesian);

  if (err != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderReadData:\n"
                    "        MPI_File_read_at_all returned error code %d.\n", reader->my_rank, err);
    return LEMON_ERR_READ;
  }

  MPI_Get_count(&status, MPI_BYTE, &read);
  *nbytes = (uint64_t)read;
  reader->pos += *nbytes;

  return LEMON_SUCCESS;
}
