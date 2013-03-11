/*****************************************************************************
 * LEMON v1.1                                                                *
 *                                                                           *
 * This file is part of the LEMON implementation of the SCIDAC LIME format.  *
 *                                                                           *
 * It is based directly upon the original c-lime implementation,             *
 * as maintained by C. deTar for the USQCD Collaboration,                    *
 * and inherits its license model and parts of its original code.            *
 *                                                                           *
 * LEMON is free software: you can redistribute it and/or modify             *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * LEMON is distributed in the hope that it will be useful,                  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details. You should have received     *
 * a copy of the GNU General Public License along with LEMON. If not,        *
 * see <http://www.gnu.org/licenses/>.                                       *
 *                                                                           *
 * LEMON was written for the European Twisted Mass Collaboration.            *
 * For support requests or bug reports, please contact                       *
 *    A. Deuzeman (deuzeman@itp.unibe.ch)                                    *
 *****************************************************************************/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../include/lemon.h"
#include "md5.h"

char const *humanForm(unsigned long long int filesize);

void usage(int rank, char **argv)
{
  if (rank == 0)
  {
    fprintf(stderr, "LEMON_BENCHMARK\n");
    fprintf(stderr, "   A utility to determine the performance of parallel reading and writing\n");
    fprintf(stderr, "   with the Lemon library for MPI I/O using the LIME file format.\n\n");
    fprintf(stderr, "   Usage:\n");
    fprintf(stderr, "      %s [L] [iterations]\n\n", argv[0]);
    fprintf(stderr, "   L:\n");
    fprintf(stderr, "      Size hint for the lattice. Will attempt to create a L^3 x 2L lattice,\n");
    fprintf(stderr, "      if this can be distributed over the given number of MPI processes.\n");
    fprintf(stderr, "      If not, dimensions are adjusted dynamically.\n");
    fprintf(stderr, "      Needs to be a positive integer.\n");
    fprintf(stderr, "   iterations:\n");
    fprintf(stderr, "      The number of times the measurement is repeated.\n");
    fprintf(stderr, "      Results for each measurement are written out individually and aggregated.\n");
    fprintf(stderr, "      Needs to be a positive integer.\n");
  }
  MPI_Finalize();
  exit(1);
}

int main(int argc, char **argv)
{
  MPI_File fp;

  LemonWriter *w;
  LemonReader *r;
  LemonRecordHeader *h;

  double *data;
  double tick, tock;
  double *timesRead;
  double *timesWrite;
  double stdRead = 0.0;
  double stdWrite = 0.0;
  int mpisize;
  int rank;
  char const *type;
  size_t ldsize;
  size_t fsize;
  int *hashMatch, *hashMatchAll;
  double const rscale = 1.0 / RAND_MAX;

  int ME_flag=1, MB_flag=1, status=0;

  int latDist[] = {0, 0, 0, 0};
  int periods[] = {1, 1, 1, 1};
  int locSizes[4];
  int latSizes[4];
  int localVol = 1;
  int latVol = localVol;

  MPI_Comm cartesian;
  int i, j;

  md5_state_t state;
  md5_byte_t before[16];
  md5_byte_t after[16];
  
  int L;
  int iters; 

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &mpisize);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  if (argc != 3)
  {
    usage(rank, argv);
    MPI_Finalize();
    return 1;
  }
  
  L = atoi(argv[1]);
  if (L <= 0)
    usage(rank, argv);

  iters = atoi(argv[2]);
  if (iters <= 0)
    usage(rank, argv);

  timesWrite = (double*)calloc(iters, sizeof(double));
  if (timesWrite == (double*)NULL)
  {
    fprintf(stderr, "ERROR: Could not allocate memory.\n");
    return 1;
  }
  timesRead = (double*)calloc(iters, sizeof(double));
  if (timesRead == (double*)NULL)
  {
    fprintf(stderr, "ERROR: Could not allocate memory.\n");
    return 1;
  }
  hashMatch = (int*)calloc(iters, sizeof(int));
    if (hashMatch == (int*)NULL)
  {
    fprintf(stderr, "ERROR: Could not allocate memory.\n");
    return 1;
  }
  hashMatchAll = (int*)calloc(iters, sizeof(int));
  if (hashMatchAll == (int*)NULL)
  {
    fprintf(stderr, "ERROR: Could not allocate memory.\n");
    return 1;
  }
  
  /* Construct a Cartesian topology, adjust lattice sizes where needed */
  MPI_Dims_create(mpisize, 4, latDist);
  
  for (i = 0; i < 4; ++i)
  {
    int div = (i == 3 ? (2 * L) : L) / latDist[i];
    locSizes[i] = div ? div : 1;
    localVol *= locSizes[i];
    latSizes[i] = locSizes[i] * latDist[i];
  }
  latVol = mpisize * localVol;
  ldsize = (unsigned long long int)localVol * 72 * sizeof(double);
  fsize = (unsigned long long int)latVol * 72 * sizeof(double);
 
  MPI_Cart_create(MPI_COMM_WORLD, 4, latDist, periods, 1, &cartesian);
  MPI_Comm_rank(cartesian, &rank);
  
  if (rank == 0)
  {
    fprintf(stdout, "Benchmark on a block of data %s in size,\n", humanForm(fsize));
    fprintf(stdout, "representing a %u x %u x %u x %u lattice", latSizes[0], latSizes[1], latSizes[2], latSizes[3]);
    if (mpisize == 1)
      fprintf(stdout, ".\n\n");
    else
    {
      fprintf(stdout, ",\ndistributed over %u MPI processes\n", mpisize);
      fprintf(stdout, "for a local %u x %u x %u x %u lattice.\n\n", locSizes[0], locSizes[1], locSizes[2], locSizes[3]);
    }
  }

  /* Allocate a block of memory for dummy data to write */
  data = (double*)malloc(ldsize);
  if (data == (double*)NULL)
  {
    fprintf(stderr, "ERROR: Could not allocate memory.\n");
    return 1;
  }
  srand(time(NULL) + rank);

  /* Start of test */
  for (i = 0; i < iters; ++i)
  {
    if (rank == 0)
      fprintf(stdout, "Measurement %d of %d.\n", i + 1, iters);
    /* Create a block of dummy data to write out 
       Fill with some random numbers to make sure we don't get coincidental matches here */
     for (j = 0; j < (localVol * 72); ++j)
	   data[j] = rscale * (double)rand();

    /* Calculate a hash of the data, to check integrity against */
    md5_init(&state);
    md5_append(&state, (md5_byte_t const *)data, ldsize);
    md5_finish(&state, before);
    
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
    MPI_Barrier(cartesian);
    timesWrite[i] = tock - tick;
    if (rank == 0)
      fprintf(stdout, "Time spent writing was %4.2g s.\n", timesWrite[i]);

    lemonWriterCloseRecord(w);
    lemonDestroyWriter(w);
    MPI_File_close(&fp);

    /* Clear data to avoid an utterly failed read giving md5 hash matches from the old data */
     memset(data, 0, ldsize);

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
    timesRead[i] = tock - tick;
    MPI_Barrier(cartesian);
    if (rank == 0)
      fprintf(stdout, "Time spent reading was %4.2g s.\n", timesRead[i]);

    lemonDestroyReader(r);
    MPI_File_close(&fp);

    md5_init(&state);
    md5_append(&state, (md5_byte_t const *)data, ldsize);
    md5_finish(&state, after);

    hashMatch[i] = strncmp((char const *)before, (char const *)after, 16) != 0 ? 1 : 0;
    MPI_Reduce(hashMatch + i, hashMatchAll + i, 1, MPI_INT, MPI_SUM, 0, cartesian);
    if (rank == 0)
    {
      if (hashMatchAll[i] == 0)
        fprintf(stdout, "All nodes report that MD5 hash matches.\n\n");
      else
        fprintf(stdout, "WARNING: MD5 hash failure detected!\n\n");
    }
  }

  /* Aggregate the data */
  hashMatch[0] = 0;
  stdWrite = timesWrite[0] * timesWrite[0];
  stdRead = timesRead[0] * timesRead[0];
  for (i = 1; i < iters; ++i)
  {
    hashMatchAll[0] += hashMatchAll[i];
    timesWrite[0] += timesWrite[i];
    stdWrite += timesWrite[i] * timesWrite[i];
    timesRead[0] += timesRead[i];
    stdRead += timesRead[i] * timesRead[i];
  }
  stdWrite /= iters;
  stdRead /= iters;
  timesWrite[0] /= iters;
  timesRead[0] /= iters;

  stdWrite -= timesWrite[0] * timesWrite[0];
  stdRead -= timesRead[0] * timesRead[0];
  
  if (rank == 0)
  {
    fprintf(stdout, "Average time spent writing was %4.2e s, ", timesWrite[0]);
    fprintf(stdout, "with a standard deviation of %4.2e s.\n", sqrt(stdWrite));
    fprintf(stdout, "Average time spent reading was %4.2e s, ", timesRead[0]);
    fprintf(stdout, "with a standard deviation of %4.2e s.\n\n", sqrt(stdRead));
    
    stdWrite *= (double)fsize / (timesWrite[0] * timesWrite[0]);
    stdRead *= (double)fsize / (timesRead[0] * timesRead[0]);
    fprintf(stdout, "Average writing speed was %s/s\n", humanForm((unsigned long long int)(fsize / timesWrite[0])));
    fprintf(stdout, "Average reading speed was %s/s\n", humanForm((unsigned long long int)(fsize / timesRead[0])));

    if (hashMatchAll[0] == 0)
      fprintf(stdout, "All data hashed correctly.\n");
    else
      fprintf(stdout, "WARNING: %d hash mismatches detected!.\n", hashMatchAll[0]);
  }

  MPI_Finalize();

  free(data);
  free(timesWrite);
  free(timesRead);
  free(hashMatch);
  free(hashMatchAll);
  
  return(0);
}
