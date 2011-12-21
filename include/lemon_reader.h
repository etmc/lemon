/*****************************************************************************
 * LEMON v1.00                                                               *
 *                                                                           *
 * This file is part of the LEMON implementation of the SCIDAC LEMON format. *
 *                                                                           *
 * It is based directly upon the original c-lemon implementation,            *
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

#pragma once

#include <lemon_header.h>

typedef struct
{
  /* Binary structure */
  MPI_File *fp;
  LemonRecordHeader *curr_header;

  /* Communicator setup */
  MPI_Comm cartesian;
  int      my_rank;

  /* File position trackers */
  MPI_Offset off;
  MPI_Offset pos;

  /* Reader state flags */
  int is_last;
  int is_awaiting_header;
  int is_busy;
  int is_striped;

  /* Data needed for tracking I/O requests */
  void *buffer;
  size_t bytes_wanted;
} LemonReader;

/* Reader manipulators */
LemonReader* lemonCreateReader(MPI_File *fp, MPI_Comm cartesian);
void lemonDestroyReader(LemonReader *reader);
int lemonSetReaderPointer(LemonReader *reader, MPI_Offset offset);
MPI_Offset lemonGetReaderPointer(LemonReader *reader);

int lemonReaderNextRecord(LemonReader *reader);
int lemonReaderMBFlag(LemonReader *reader);
int lemonReaderMEFlag(LemonReader *reader);
char const *lemonReaderType(LemonReader *reader);
MPI_Offset lemonReaderBytes(LemonReader *reader);
MPI_Offset lemonReaderPadBytes(LemonReader *reader);

int lemonReaderReadData(void *dest, MPI_Offset *nbytes, LemonReader *reader);
int lemonReaderCloseRecord(LemonReader *reader);
int lemonReaderSeek(LemonReader *reader, MPI_Offset offset, int whence);
int lemonReaderSetState(LemonReader *rdest, LemonReader const *rsrc);
int lemonEOM(LemonReader *reader);

/* Additions for LEMON follow */
int lemonReadLatticeParallel(LemonReader *reader, void *data, MPI_Offset siteSize, int const *latticeDims);
int lemonReadLatticeParallelMapped(LemonReader *reader, void *data, MPI_Offset siteSize, int const *latticeDims, int const *mapping);
int lemonReadLatticeParallelNonBlocking(LemonReader *reader, void *data, MPI_Offset siteSize, int const *latticeDims);
int lemonReadLatticeParallelNonBlockingMapped(LemonReader *reader, void *data, MPI_Offset siteSize, int const *latticeDims, int const *mapping);
int lemonReaderReadDataNonBlocking(void *dest, MPI_Offset const *nbytes, LemonReader *reader);
int lemonFinishReading(LemonReader *reader);

