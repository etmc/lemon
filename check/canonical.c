#include <stdio.h>
#include <mpi.h>
#include <lemon.h>
#include <string.h>

int main(int argc, char **argv)
{
  MPI_File fp;
  MPI_Comm cartesian;

  /* Needed for the creation of a Cartesian communicator */
  int mpiSize;
  int distribution[] = {0, 0, 0, 0};
  int periods[] = {1, 1, 1, 1};

  LemonWriter *w;
  LemonRecordHeader *h;

  /* The data we want to write */
  char message[] = "LEMON test message";
  MPI_Offset bytes = strlen(message);

  MPI_Init(&argc, &argv);

  /* Create a 4 dimensional Cartesian node distribution. */
  MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
  MPI_Dims_create(mpiSize, 4 /* #dims */, distribution);
  MPI_Cart_create(MPI_COMM_WORLD, 4 /* #dims */ , distribution, periods, 1 /* reorder */, &cartesian);

  /* Open the file using the Cartesian communicator just created, initialize a writer with it */
  MPI_File_open(MPI_COMM_WORLD, "canonical.test", MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fp);
  w = lemonCreateWriter(&fp, MPI_COMM_WORLD);

  /* Create a header, write it out and destroy it again */
  h = lemonCreateHeader(1 /* MB */, 1 /* ME */, "lemon-test-text", bytes);
  lemonWriteRecordHeader(h, w);
  lemonDestroyHeader(h);

  /* Write out the small test string defined earlier as the data block */
  lemonWriteRecordData(message, &bytes, w);
  lemonWriterCloseRecord(w);

  /* Close the writer and release resources */
  lemonDestroyWriter(w);
  MPI_File_close(&fp);

  MPI_Finalize();

  return 0;
}
