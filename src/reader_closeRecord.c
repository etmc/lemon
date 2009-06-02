#include <config.h>
#include <lemon.h>
#include <stdio.h>

#include "internal_padding.static"

int lemonReaderCloseRecord(LemonReader *reader)
{
  if (reader == (LemonReader*)NULL)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderCloseRecord:\n"
                    "        NULL pointer provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  if (reader->is_awaiting_header)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderCloseRecord:\n"
                    "        reader is not initialized.\n", reader->my_rank);
    return LEMON_ERR_CLOSE;
  }

  if (reader->is_busy)
    lemonFinishReading(reader);

  MPI_Barrier(reader->cartesian);
  reader->off += reader->curr_header->data_length + lemon_padding(reader->curr_header->data_length);
  reader->pos = 0;
  reader->is_awaiting_header = 1;

  return LEMON_SUCCESS;
}
