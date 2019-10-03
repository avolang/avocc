#ifndef AVOCC_H // NOLINT
#define AVOCC_H

#include <stddef.h> // size_t

// Cointains the state of a source code buffer
typedef struct _avoc_source {
  unsigned char *buf_data;
  size_t buf_len; // Buffer total length
  size_t buf_pos; // Cursor position (two by two code points)
  size_t cur_pos; // Current code point position

  int cur_cp; // Current code point
  int nxt_cp; // Next code point

  size_t row;
  size_t col;
  char *name;
} avoc_source;

#define UTF8_END (-1)
#define UTF8_ERROR (-2)

// Initializes a source copying the values into memory.
void avoc_source_init(avoc_source *src, const char *name, const char *buf_data,
                      size_t buf_len);

// Frees the resources of a src without freeing the src itself.
void avoc_source_free(avoc_source *src);

// Moves forward into the buffer, storing cur_cp and nxt_cp.
int avoc_source_fwd(avoc_source *src);

#endif /* AVOCC_H */
