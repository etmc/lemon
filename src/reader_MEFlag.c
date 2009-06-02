#include <config.h>
#include <lemon.h>
#include <stdio.h>

int lemonReaderMEFlag(LemonReader *reader)
{
  if ((reader == (LemonReader*)NULL) || reader->is_awaiting_header)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderMEFlag:\n"
                    "        NULL pointer or uninitialized reader provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }
  return reader->curr_header->ME_flag;
}
