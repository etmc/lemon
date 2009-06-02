#include <config.h>
#include <lemon.h>
#include "internal_writeRecordBinaryHeader.static"

int lemonWriteRecordHeader(LemonRecordHeader *props, LemonWriter* writer)
{
  int result;

  if ((writer == (LemonWriter *)NULL) || (props == (LemonRecordHeader *)NULL))
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteRecordHeader:\n"
                    "        NULL pointer provided.\n", writer->my_rank);
    return LEMON_ERR_PARAM;
  }

  /* Make sure header is expected now */
  if (!writer->is_awaiting_header)
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteRecordHeader:\n"
                    "        Writer not ready for header.\n", writer->my_rank);
    return LEMON_ERR_HEADER_NEXT;
  }

  if (writer->is_busy)
    lemonFinishWriting(writer);

  /* If last record ended a message, this one must begin a new one */
  /* If last record did not end a message, this one must not begin one */
  /* Since we allow appending to a file and we don't reread it to check
     the state of the last flag, we don't do this check for the first
     record written. */
  if (!writer->is_first_record && writer->is_last_written != props->MB_flag )
  {
    fprintf(stderr, "[LEMON] Node %d reports in lemonWriteRecordHeader:\n"
                    "        message configuration inconsistent.\n", writer->my_rank);
    return LEMON_ERR_MBME;
  }

  result = writeLemonRecordBinaryHeader(writer, props);

  /* Set new writer state */
  writer->is_awaiting_header = 0;
  writer->is_first_record = 0;
  writer->is_last      = props->ME_flag;
  writer->is_last_written = 0;
  writer->data_length  = props->data_length;

  return result;
}
