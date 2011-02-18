#include <config.h>
#include <lemon.h>
#include <stdio.h>

int lemonFinishReading(LemonReader *reader)
{
  int read;
  int size;
  MPI_Status status;
  char MPImode[] = "native";

  if (!reader->is_busy)
    return LEMON_SUCCESS;

  MPI_Comm_size(reader->cartesian, &size);

  MPI_File_read_at_all_end(*reader->fp, reader->buffer, &status);

  reader->pos += reader->bytes_wanted;
  MPI_File_set_view(*reader->fp, reader->off, MPI_BYTE, MPI_BYTE, MPImode, MPI_INFO_NULL);
  MPI_File_seek(*reader->fp, reader->pos, MPI_SEEK_SET);

  MPI_Get_count(&status, MPI_BYTE, &read);

  /* Doing a data read should never get us to EOF, only header scanning */
  if (read != (reader->is_striped ? reader->bytes_wanted / size : reader->bytes_wanted))
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonFinishReading:\n"
                    "        Could not read the required amount of data.\n", reader->my_rank);
    return LEMON_ERR_READ;
  }

  reader->bytes_wanted = 0;
  reader->buffer = NULL;
  reader->is_busy = 0;
  reader->is_striped = 0;

  return LEMON_SUCCESS;
}
