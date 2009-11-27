#include <config.h>
#include <lemon.h>
#include <stdio.h>

int lemonSetReaderPointer(LemonReader *reader, MPI_Offset offset)
{
  int err;

  if (reader == (LemonReader*)NULL || reader->is_awaiting_header)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonSetReaderPointer:\n"
                    "        NULL pointer or uninitialized reader provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  if (reader->is_busy)
    lemonFinishReading(reader);

  reader->pos = offset;
  err = MPI_File_seek(*reader->fp, reader->pos, MPI_SEEK_SET);

  if (err != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonSetReaderPointer:\n"
                    "        MPI_File_seek returned error code %d.\n", reader->my_rank, err);
    return LEMON_ERR_SEEK;
  }
  return LEMON_SUCCESS;
}
