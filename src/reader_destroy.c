#include <config.h>
#include <lemon.h>

void lemonDestroyReader(LemonReader *reader)
{
  if (reader == (LemonReader*)NULL)
    return;

  if (reader->is_busy)
    lemonFinishReading(reader);

  free(reader->curr_header);
  MPI_Comm_free(&reader->cartesian);

  free(reader);
}
