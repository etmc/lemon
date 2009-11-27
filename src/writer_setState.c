#include <config.h>
#include <lemon.h>

int lemonWriterSetState(LemonWriter *wdest, LemonWriter *wsrc)
{
  MPI_Offset   disp;
  MPI_Datatype etype;
  MPI_Datatype ftype;
  char         drep[32];

  /* Set wdest writer state from wsrc */
  /* We do not copy the file pointer member fp */
  wdest->is_awaiting_header = wsrc->is_awaiting_header;
  wdest->is_busy            = 0;
  wdest->is_collective      = 0;
  wdest->is_first_record    = wsrc->is_first_record;
  wdest->is_last            = wsrc->is_last;
  wdest->is_last_written    = wsrc->is_last_written;

  wdest->off                = wsrc->off;
  wdest->pos                = wsrc->pos;
  wdest->data_length        = wsrc->data_length;

  /* Now make the system state agree with the writer state */
  MPI_File_get_view(*wsrc->fp, &disp, &etype, &ftype, drep);
  MPI_File_set_view(*wdest->fp, disp, etype, ftype, drep, MPI_INFO_NULL);
  MPI_File_seek_shared(*wdest->fp, wdest->pos, MPI_SEEK_CUR);
  MPI_Barrier(wdest->cartesian);

  MPI_Comm_dup(wsrc->cartesian, &wdest->cartesian);
  MPI_Comm_rank(wdest->cartesian, &wdest->my_rank);

  return LEMON_SUCCESS;
}
