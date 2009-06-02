#include <config.h>
#include <lemon.h>
#include <stdio.h>

int lemonReaderSeek(LemonReader *reader, MPI_Offset offset, int whence)
{
  int err;
  if (reader == (LemonReader*)NULL || reader->is_awaiting_header)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderSeek:\n"
                    "        NULL pointer or uninitialized reader provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  if (reader->is_busy)
    lemonFinishReading(reader);

  if (whence == MPI_SEEK_CUR)
    reader->pos += offset;
  else if (whence == MPI_SEEK_SET)
    reader->pos = offset;
  else if (whence == MPI_SEEK_END)
    reader->pos = reader->curr_header->data_length - offset;
  else
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderSeek:\n"
                    "        Value passed for whence not recognized.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  err = MPI_File_seek(*reader->fh, reader->pos, MPI_SEEK_SET);

  if (err != MPI_SUCCESS)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderSeek:\n"
                    "        MPI_File_seek returned error code %d.\n", reader->my_rank, err);
    return LEMON_ERR_SEEK;
  }
  return LEMON_SUCCESS;
}
