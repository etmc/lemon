#include <config.h>
#include <lemon.h>
#include <stdio.h>

#include "internal_padding.static"

size_t lemonReaderPadBytes(LemonReader *reader)
{
  if ((reader == (LemonReader*)NULL) || reader->is_awaiting_header)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderPadBytes:\n"
                    "        NULL pointer or uninitialized reader provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }
  return lemon_padding(reader->curr_header->data_length);
}

