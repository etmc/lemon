#include <config.h>
#include <lemon.h>
#include <stdio.h>

uint64_t lemonReaderBytes(LemonReader *reader)
{
  if (reader == (LemonReader*)NULL || reader->is_awaiting_header)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderBytes:\n"
                    "        NULL pointer or uninitialized reader provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }
  return reader->curr_header->data_length;
}
