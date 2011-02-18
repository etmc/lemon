#include <stdio.h>

#include <config.h>
#include <lemon.h>

#include "internal_readAndParseHeader.static"

int lemonReaderNextRecord(LemonReader *reader)
{
  int err = 0;

  if (reader == (LemonReader *)NULL)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonReaderNextRecord:\n"
                    "        NULL pointer or uninitialized reader provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  if (!reader->is_awaiting_header)
    lemonReaderCloseRecord(reader);

  err = readAndParseHeader(reader);

  /* readAndParseHeader will produce debug output */
  if (err != LEMON_SUCCESS)
    return err;

  reader->is_last = reader->curr_header->ME_flag;
  reader->is_awaiting_header = 0;

  return LEMON_SUCCESS;
}
