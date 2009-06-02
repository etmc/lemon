#include <config.h>
#include <lemon.h>
#include <stdio.h>

#include "internal_padding.static"

int lemonWriterCloseRecord(LemonWriter *writer)
{
  MPI_Status status;

  size_t pad;
  unsigned char padbuf[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  if (writer == (LemonWriter*)NULL)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriterCloseRecord:\n"
                    "        NULL pointer or uninitialized writer provided.\n", writer->my_rank);
    return LEMON_ERR_PARAM;
  }

  /* Race conditions can occur in between functions here - first synchronize */
  MPI_Barrier(writer->cartesian);

  /* First check if the record is already closed (i.e. ready for header writing) */
  if (writer->is_awaiting_header)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriterCloseRecord:\n"
                    "        writer is not initialized.\n", writer->my_rank);
    return LEMON_ERR_CLOSE;
  }
  if (writer->is_busy)
    lemonFinishWriting(writer);

  /* Advance to end of record */
  MPI_File_seek_shared(*writer->fh, writer->data_length, MPI_SEEK_SET);

  /* Padding */
  pad = lemon_padding(writer->data_length);
  if (pad > 0 && writer->my_rank == 0)
    MPI_File_write_at(*writer->fh, writer->off + writer->pos, padbuf, pad, MPI_BYTE, &status);
  MPI_File_sync(*writer->fh);

  /* Clean up now */
  MPI_Barrier(writer->cartesian);

  /* Synchronize the internal offset cache */
  writer->off += writer->data_length + pad;
  writer->pos = 0;

  writer->is_awaiting_header = 1;  /* Next thing to come is a header */

  if (writer->is_last == 1)
    writer->is_last_written = 1;

  writer->is_last = 0;

  return LEMON_SUCCESS;
}
