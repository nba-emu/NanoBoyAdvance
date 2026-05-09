#include "unarr.h"
#include <stdio.h>

ar_archive *ar_open_any_archive(ar_stream *stream) {
  ar_archive *ar = ar_open_zip_archive(stream, false);
  if (!ar)
    ar = ar_open_zip_archive(stream, true);
  if (!ar)
    ar = ar_open_rar_archive(stream);
  if (!ar)
    ar = ar_open_7z_archive(stream);
  if (!ar)
    ar = ar_open_tar_archive(stream);
  return ar;
}

void read_test(ar_stream *stream) {

  ar_archive *ar = ar_open_any_archive(stream);

  if (!ar) {
    return;
  }

  while (true) {

    if (ar_at_eof(ar)) {
      break;
    }

    bool ok = ar_parse_entry(ar);

    if (!ok) {
      break;
    }

    size_t size = ar_entry_get_size(ar);
    while (size > 0) {
      unsigned char buffer[1024];
      size_t count = size < sizeof(buffer) ? size : sizeof(buffer);
      if (!ar_entry_uncompress(ar, buffer, count))
        break;
      size -= count;
    }
  }

  if (!ar_at_eof(ar)) {
    printf("not reach eof\n");
  }

  ar_close_archive(ar);
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {

  ar_stream *stream;

  stream = ar_open_memory(Data, Size);

  read_test(stream);

  ar_close(stream);

  return 0; // Non-zero return values are reserved for future use.
}
