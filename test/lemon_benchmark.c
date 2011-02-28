#include <lemon.h>
#include <stdio.h>
#include <string.h>

#include "md5.h"

static char out[32];
char *humanForm(unsigned long long int filesize)
{
  static char const *units[] = {"kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
  double engFilesize;
  int  prec;

  /* Using SI conventions, unambiguous since these are transmission speeds */
  if (filesize < 1000)
  {
    sprintf(out, "%u B", (unsigned int)filesize);
    return out;
  }

  size_t ucnt = 0;
  while ((ucnt < 7) && (filesize / 1000) > 1000 )
  {
    filesize /= 1000;
    ++ucnt;
  }
  engFilesize = filesize / 1000.0;

  filesize /= 1000;
  if (filesize >= 100)
    sprintf(out, "%5.0f %s", engFilesize, units[ucnt]);
  else if (filesize >= 10)
    sprintf(out, "%5.1f %s", engFilesize, units[ucnt]);
  else 
   sprintf(out, "%5.2f %s", engFilesize, units[ucnt]);

  return out;
}

int main(int argc, char **argv)
{
  MPI_File fp;

  LemonWriter *w;
  LemonReader *r;
  LemonRecordHeader *h;

  double *data;
  double tick, tock;
  double timeRead, timeWrite;
  int mpisize;
  int rank;
  char const *type;
  unsigned long long int fsize;
  int hashMatch, hashMatchAll;

  int ME_flag=1, MB_flag=1, status=0;

  int latDist[] = {0, 0, 0, 0};
  int periods[] = {1, 1, 1, 1};
  /* The following are the local volumes - we extend the lattice as needed. */
  int latSizes[] = {8, 8, 8, 8};
  int localVol = 8 * 8 * 8 * 8;
  int latVol = localVol;

  MPI_Comm cartesian;
  int i;

  md5_state_t state;
  md5_byte_t before[16];
  md5_byte_t after[16];

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &mpisize);

  MPI_Dims_create(mpisize, 4, latDist);
  MPI_Cart_create(MPI_COMM_WORLD, 4, latDist, periods, 1, &cartesian);

  /* Create a block of dummy data */
  data = (double*)malloc(72 * latVol * sizeof(double));
  srand(time(NULL) + rank); /* Make sure all blocks are different */
  for (i = 0; i < (latVol *  72); ++i)
    data[i] = ((double)rand()) / RAND_MAX;

  md5_init(&state);
  md5_append(&state, (md5_byte_t const *)data, 72 * localVol * sizeof(double));
  md5_finish(&state, before);

  /* Convert local to global sizes */
  for (i = 0; i < 4; ++i)
    latSizes[i] *= latDist[i];
  latVol *= mpisize;

  fsize = latVol * 72 * sizeof(double);
  MPI_Comm_rank(cartesian, &rank);

  if (rank == 0)
  {
    fprintf(stdout, "Benchmark on a block of data %s in size,\n", humanForm(fsize));
    fprintf(stdout, "representing a %u x %u x %u x %u lattice", latSizes[0], latSizes[1], latSizes[2], latSizes[3]);
    if (mpisize == 1)
      fprintf(stdout, ".\n\n");
    else
      fprintf(stdout, ",\ndistributed over %u MPI processes.\n\n", mpisize);
  }

   /* Start of writing test */
   /* Note that the following is the only (?) way to truncate the file with MPI */
  MPI_File_open(cartesian, "benchmark.test", MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fp);
  MPI_File_set_size(fp, 0);
  w = lemonCreateWriter(&fp, cartesian);

  h = lemonCreateHeader(MB_flag, ME_flag, "benchmark", latVol);
  status = lemonWriteRecordHeader(h, w);

  lemonDestroyHeader(h);

  MPI_Barrier(cartesian);
  tick = MPI_Wtime();
  lemonWriteLatticeParallel(w, data, 72 * sizeof(double), latSizes);
  tock = MPI_Wtime();
  timeWrite = tock - tick;
  MPI_Barrier(cartesian);
  lemonWriterCloseRecord(w);
  lemonDestroyWriter(w);
  MPI_File_close(&fp);

  /* Clear data to avoid an utterly failed read giving md5 hash matches from the old data */
  memset(data, 0, localVol * 72 * sizeof(double));

  /* Start of reading test */
  MPI_File_open(cartesian, "benchmark.test", MPI_MODE_RDONLY | MPI_MODE_DELETE_ON_CLOSE, MPI_INFO_NULL, &fp);
  r = lemonCreateReader(&fp, cartesian);

  if (lemonReaderNextRecord(r))
    fprintf(stderr, "Node %d reports: next record failed.\n", rank);

  type = lemonReaderType(r);
  if (strncmp(type, "benchmark", 13))
    fprintf(stderr, "Node %d reports: wrong type read.\n", rank);

  MPI_Barrier(cartesian);
  tick = MPI_Wtime();
  lemonReadLatticeParallel(r, data, 72 * sizeof(double), latSizes);
  tock = MPI_Wtime();
  timeRead = tock - tick;
  MPI_Barrier(cartesian);
  lemonDestroyReader(r);
  MPI_File_close(&fp);

  md5_init(&state);
  md5_append(&state, (md5_byte_t const *)data, 72 * localVol * sizeof(double));
  md5_finish(&state, after);

  hashMatch = strncmp(before, after, 16);
  MPI_Reduce(&hashMatch, &hashMatchAll, 1, MPI_INT, MPI_SUM, 0, cartesian);

  if (rank == 0)
  {
    fprintf(stdout, "Time spent writing: %4.2g s.\n", timeWrite);
    fprintf(stdout, "Writing speed:      %s/s.\n\n", humanForm((unsigned long long int)(fsize / timeWrite)));
    fprintf(stdout, "Time spent reading: %4.2g s.\n", timeRead);
    fprintf(stdout, "Reading speed:      %s/s.\n\n", humanForm((unsigned long long int)(fsize / timeRead)));
    if (hashMatchAll == 0)
      fprintf(stdout, "All nodes report that MD5 hash matches.\n");
    else
      fprintf(stdout, "WARNING: MD5 hash failure detected!\n");
  }

  MPI_Finalize();

  free(data);

  return(0);
}

