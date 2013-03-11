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
