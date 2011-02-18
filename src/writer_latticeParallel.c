#include <config.h>
#include <lemon.h>
#include <stdio.h>

int lemonWriteLatticeParallel(LemonWriter *writer, void *data, MPI_Offset siteSize, int const *latticeDims)
{
  /* We forward this to the mapped routine to avoid doubling code, effectively making it a convenient alias. */
  int const mapping[4] = {0, 1, 2, 3};
  return lemonWriteLatticeParallelMapped(writer, data, siteSize, latticeDims, mapping);
}
