/*****************************************************************************
 * LEMON v1.01                                                               *
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

#include <stdio.h>
#include <string.h>

#include "../include/lemon.h"

int main(int argc, char **argv)
{
  MPI_File fp;

  LemonWriter *w;
  LemonReader *r;
  LemonRecordHeader *h;

  char *data;
  char *data_read;
  int mpisize;
  int rank;
  char const *type;

  int ME_flag=1, MB_flag=1, status=0;

  int latDist[] = {0, 0, 0, 0};
  int periods[] = {1, 1, 1, 1};
  /* The following are the local volumes - we extend the lattice as needed. */
  int latSizes[] = {4, 4, 4, 4};
  int latVol = 256;

  MPI_Comm cartesian;
  int i;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &mpisize);

  MPI_Dims_create(mpisize, 4, latDist);
  MPI_Cart_create(MPI_COMM_WORLD, 4, latDist, periods, 1, &cartesian);
  for (i = 0; i < 4; ++i)
    latSizes[i] *= latDist[i];
  latVol *= mpisize;
  MPI_Comm_rank(cartesian, &rank);

   /* Start of code - writing */
  MPI_File_open(cartesian, "parallel_non_blocking.test", MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fp);
  w = lemonCreateWriter(&fp, cartesian);

  data = (char*)malloc(257);
  for (i = 0; i < latVol/mpisize; ++i)
    data[i] = (char)(rank + 48);
  data[256] = '\0';

  h = lemonCreateHeader(MB_flag, ME_flag, "parallel-test", latVol);
  status = lemonWriteRecordHeader(h, w);

  lemonDestroyHeader(h);

  lemonWriteLatticeParallelNonBlocking(w, data, sizeof(char), latSizes);
  lemonFinishWriting(w);

  lemonWriterCloseRecord(w);
  lemonDestroyWriter(w);
  MPI_File_close(&fp);

  /* Reading */
  data_read = (char*)malloc(257);

  MPI_File_open(cartesian, "parallel_non_blocking.test", MPI_MODE_RDONLY, MPI_INFO_NULL, &fp);
  r = lemonCreateReader(&fp, cartesian);

  if (lemonReaderNextRecord(r))
    fprintf(stderr, "Node %d reports: next record failed.\n", rank);

  type = lemonReaderType(r);
  if (strncmp(type, "parallel-test", 13))
    fprintf(stderr, "Node %d reports: wrong type read.\n", rank);

  lemonReadLatticeParallelNonBlocking(r, data_read, sizeof(char), latSizes);
  lemonFinishReading(r);
  data_read[256] = '\0';
  if (strncmp(data_read, data, 256))
  {
    fprintf(stderr, "Node %d reports: wrong data read.\n", rank);
    fprintf(stderr, "Node %d wanted: %s\n", rank, data);
    fprintf(stderr, "Node %d got   : %s\n", rank, data_read);
  }
  else
    fprintf(stderr, "Node %d reports data okay.\n", rank);

  lemonDestroyReader(r);

  MPI_File_close(&fp);
  MPI_Finalize();

  free(data);
  free(data_read);

  return(0);
}

