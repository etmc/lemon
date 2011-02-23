#include <config.h>
#include <lemon.h>
#include <stdio.h>

int lemonSetReaderPointer(LemonReader *reader, MPI_Offset offset)
{
  return lemonReaderSeek(reader, offset, MPI_SEEK_SET);
}
