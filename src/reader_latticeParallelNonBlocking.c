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

#include <config.h>
#include <lemon.h>
#include <stdio.h>

int lemonReadLatticeParallelNonBlocking(LemonReader *reader, void *data, MPI_Offset siteSize, int const *latticeDims)
{
  /* We forward this to the mapped routine to avoid doubling code, effectively making it a convenient alias. */
  int const mapping[4] = {0, 1, 2, 3};
  return lemonReadLatticeParallelNonBlockingMapped(reader, data, siteSize, latticeDims, mapping);
}
