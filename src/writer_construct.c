#include <config.h>
#include <lemon.h>

LemonWriter* lemonCreateWriter(MPI_File *fp, MPI_Comm cartesian)
{
  LemonWriter* result;

  if (fp == (MPI_File*)NULL)
    return NULL;

  result = (LemonWriter *)malloc(sizeof(LemonWriter));
  if (result == (LemonWriter *)NULL)
    return NULL;

  result->fp = fp;

  result->is_awaiting_header = 1;
  result->is_busy = 0;
  result->is_collective = 0;
  result->is_first_record = 1;
  result->is_last = 0;
  result->is_last_written = 0;

  MPI_File_get_position(*(result->fp), &(result->off));
  result->pos = 0;

  MPI_Comm_dup(cartesian, &result->cartesian);
  MPI_Comm_rank(cartesian, &result->my_rank);

  return result;
}

