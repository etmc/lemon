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

#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "../include/lemon.h"

int main(int argc, char **argv)
{
  int rank;
  MPI_File fp;

  LemonWriter *w;
  LemonReader *r;
  LemonRecordHeader *h;

  int ME_flag=1, MB_flag=1, status=0;
  int err;
  char message[512];
  char message_read[512];
  char const *type;
  MPI_Offset bytes;
  MPI_Offset bytes_read;
  struct timeval t1;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  /* Writing */
  MPI_File_open(MPI_COMM_WORLD, "xlf_non_blocking.test", MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fp);

  w = lemonCreateWriter(&fp, MPI_COMM_WORLD);

  gettimeofday(&t1, NULL);
  sprintf(message,"\n plaquette = %e\n trajectory nr = %d\n beta = %f, kappa = %f, mu = %f, c2_rec = %f\n time = %ld\n hmcversion = %s\n mubar = %f\n epsilonbar = %f\n date = %s",
                  0.0, 1, 6.0, 0.177, 0.5, 0.0, t1.tv_sec, "5.0.1", 0.0, 0.0, ctime(&t1.tv_sec));

  bytes = strlen(message);
  h = lemonCreateHeader(MB_flag, ME_flag, "xlf-info", bytes);
  status = lemonWriteRecordHeader(h, w);

  lemonDestroyHeader( h );
  lemonWriteRecordDataNonBlocking(message, &bytes, w);
  lemonFinishWriting(w);
  lemonWriterCloseRecord(w);
  lemonDestroyWriter(w);

  MPI_File_close(&fp);

  /* Reading */
  MPI_File_open(MPI_COMM_WORLD, "xlf_non_blocking.test", MPI_MODE_RDONLY, MPI_INFO_NULL, &fp);

  r = lemonCreateReader(&fp, MPI_COMM_WORLD);
  if (lemonReaderNextRecord(r))
    fprintf(stderr, "Node %d reports: next record failed.\n", rank);

  type = lemonReaderType(r);
  if (strncmp(type, "xlf-info", 8))
    fprintf(stderr, "Node %d reports: wrong type read.\n", rank);

  bytes_read = bytes;
  lemonReaderReadDataNonBlocking(message_read, &bytes_read, r);
  err = lemonFinishReading(r);

  if (err)
    fprintf(stderr, "Node %d reports: wrong message read.\n", rank);
  else
    fprintf(stderr, "Node %d reports data okay.\n", rank);
  lemonReaderCloseRecord(r);
  lemonDestroyReader(r);

  MPI_File_close(&fp);

  MPI_Finalize();

  return(0);
}
