#include <config.h>
#include <lemon.h>
#include <stdio.h>

MPI_Offset lemonGetReaderPointer(LemonReader *reader)
{
  if (reader == (LemonReader*)NULL)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonGetReaderPointer:\n"
                    "        NULL pointer or uninitialized reader provided.\n", reader->my_rank);
    return LEMON_ERR_PARAM;
  }

  return reader->pos;
}
