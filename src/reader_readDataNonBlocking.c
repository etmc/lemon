#include <config.h>
#include <lemon.h>
#include <stdio.h>

int lemonReaderReadDataNonBlocking(void *dest, uint64_t nbytes, LemonReader *reader)
{
  int err;

  if ((reader == (LemonReader*)NULL) || (dest == NULL))
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderReadDataNonBlocking:\n"
                    "        NULL pointer or uninitialized reader provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  err = MPI_File_read_at_all_begin(*reader->fp, reader->off + reader->pos, dest, nbytes, MPI_BYTE);

  if (err != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderReadDataNonBlocking:\n"
                    "        MPI_File_read_at_all_begin returned error code %d.\n", reader->my_rank, err);
    return LEMON_ERR_READ;
  }
  reader->is_busy = 1;
  reader->is_striped = 0;
  reader->buffer = dest;
  reader->bytes_wanted = nbytes;

  return LEMON_SUCCESS;
}
