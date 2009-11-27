#include <config.h>
#include <lemon.h>
#include <stdio.h>

#include "internal_padding.static"

int lemonFinishWriting(LemonWriter *writer)
{
  int written;
  int size;
  MPI_Status status;

  if (!writer->is_busy)
    return LEMON_SUCCESS;

  if (writer->is_collective)
    MPI_File_write_at_all_end(*writer->fp, writer->buffer, &status);
  else if (writer->my_rank == 0)
    MPI_Wait(&writer->request, &status);

  writer->pos += writer->bytes_wanted;
  MPI_File_set_view(*writer->fp, writer->off, MPI_BYTE, MPI_BYTE, "native", MPI_INFO_NULL);
  MPI_File_seek(*writer->fp, writer->pos, MPI_SEEK_SET);

  MPI_Get_count(&status, MPI_BYTE, &written);

  if (writer->is_collective)
  {
    MPI_Comm_size(writer->cartesian, &size);
    if (written != writer->bytes_wanted / size)
    {
      fprintf(stderr, "[LEMON] Node %d reports in lemonFinishWriting:\n"
                      "        Could not write the required amount of data.\n", writer->my_rank);
      fprintf(stderr, "        needed: %d, written: %d\n", writer->bytes_wanted / size , written);
      return LEMON_ERR_WRITE;
    }
  }
  else
  {
    MPI_Bcast(&written, 1, MPI_INT, 0, writer->cartesian);
    if (written != writer->bytes_wanted)
    {
      fprintf(stderr, "[LEMON] Node %d reports in lemonFinishWriting:\n"
                      "        Could not write the required amount of data.\n", writer->my_rank);
      fprintf(stderr, "        needed: %d, written: %d\n", writer->bytes_wanted / size , written);
      return LEMON_ERR_WRITE;
    }
  }

  writer->data_length = 0;
  writer->buffer = NULL;
  writer->is_busy = 0;
  writer->is_collective = 0;

  return LEMON_SUCCESS;
}
