#pragma once

/****************************************************************************
 * LEMON v0.99                                                              *
 *                                                                          *
 * This file is part of the LEMON implementation of the SCIDAC LEMON format. *
 *                                                                          *
 * It is based directly upon the original c-lemon implementation,            *
 * as maintained by C. deTar for the USQCD Collaboration,                   *
 * and inherits its license model and parts of its original code.           *
 *                                                                          *
 * LEMON is free software: you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by     *
 * the Free Software Foundation, either version 3 of the License, or        *
 * (at your option) any later version.                                      *
 *                                                                          *
 * LEMON is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details. You should have received    *
 * a copy of the GNU General Public License along with LEMON. If not,       *
 * see <http://www.gnu.org/licenses/>.                                      *
 *                                                                          *
 * LEMON was written for the European Twisted Mass Collaboration.           *
 * For support requests or bug reports,                                     *
 * please contact A. Deuzeman (a.deuzeman@rug.nl)                           *
 ****************************************************************************/

#include <mpi.h>
#include <lemon_header.h>

typedef struct
{
  /* Binary structure */
  MPI_File* fp;

  /* Communicator setup */
  MPI_Comm cartesian;
  int      my_rank;

  /* File position trackers */
  MPI_Offset off;
  MPI_Offset pos;

  /* Writer state flags */
  int is_awaiting_header;
  int is_busy;
  int is_collective;
  int is_first_record;
  int is_last;
  int is_last_written;

  /* Data needed for tracking I/O requests */
  void *buffer;
  MPI_Request request;
  int bytes_wanted;
  MPI_Offset data_length;
} LemonWriter;

/* Writer manipulators */
LemonWriter* lemonCreateWriter(MPI_File *fp, MPI_Comm cartesian);
int lemonDestroyWriter(LemonWriter *writer);
int lemonWriteRecordHeader(LemonRecordHeader const *props, LemonWriter* writer);
int lemonWriteRecordData(void *source, MPI_Offset *nbytes, LemonWriter* writer);

int lemonWriterCloseRecord(LemonWriter *writer);
int lemonWriterSeek(LemonWriter *writer, MPI_Offset offset, int whence);
int lemonWriterSetState(LemonWriter *wdest, LemonWriter const *wsrc);

/* Additions for LEMON follow */
int lemonWriteLatticeParallel(LemonWriter *writer, void *data, MPI_Offset siteSize, int const *latticeDims);
int lemonWriteLatticeParallelNonBlocking(LemonWriter *writer, void *data, MPI_Offset siteSize, int const *latticeDims);
int lemonWriteLatticeParallelMapped(LemonWriter *writer, void *data, MPI_Offset siteSize, int const *latticeDims, int const *mapping);
int lemonWriteLatticeParallelNonBlockingMapped(LemonWriter *writer, void *data, MPI_Offset siteSize, int const *latticeDims, int const *mapping);
int lemonWriteRecordDataNonBlocking(void *source, MPI_Offset const *nbytes, LemonWriter* writer);
int lemonFinishWriting(LemonWriter *writer);
