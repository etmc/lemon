#include <config.h>
#include <lemon.h>
#include <stdio.h>

char const *lemonReaderType(LemonReader *reader)
{
  if ((reader == (LemonReader*)NULL) || reader->is_awaiting_header)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderType:\n"
                    "        NULL pointer or uninitialized reader provided.\n", reader->my_rank);
    return NULL;
  }
  return reader->curr_header->type;
}
