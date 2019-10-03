#include "avocc.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void avoc_source_init(avoc_source *src, const char *name, const char *buf_data,
                      size_t buf_len) {
  assert(src != NULL);

  if (buf_data != NULL) {
    src->buf_data = calloc(buf_len, sizeof(unsigned char));
    memset(src->buf_data, 0L, buf_len);
    memcpy(src->buf_data, buf_data, buf_len);
  } else {
    src->buf_data = NULL;
  }

  src->buf_len = buf_len;
  src->buf_pos = 0L;
  src->cur_cp = 0L;
  src->nxt_cp = 0L;
  src->cur_cp_pos = 0L;
  src->nxt_cp_pos = 0L;
  src->row = 0L;
  src->col = 0L;

  if (name != NULL) {
    size_t name_len = strlen(name)+1;
    src->name = calloc(name_len, sizeof(char));
    memset(src->name, 0L, name_len);
    memcpy(src->name, name, name_len);
  } else {
    src->name = NULL;
  }
}

void avoc_source_free(avoc_source *src) {
  assert(src != NULL);

  if (src->buf_data != NULL) {
    free(src->buf_data);
    src->buf_data = NULL;
  }

  if (src->name != NULL) {
    free(src->name);
    src->name = NULL;
  }
}

static unsigned utf8_get(avoc_source *src) {
  return src->buf_pos >= src->buf_len ?
      UTF8_END : (src->buf_data[(src)->buf_pos] & 0xFFu);
}

static unsigned utf8_cont(avoc_source *src) {
  src->buf_pos++;
  unsigned int c = utf8_get(src);
  return ((c & 0xC0u) == 0x80u) ?
    (c & 0x3Fu) : UTF8_ERROR;
}

static int utf8_next_cp(avoc_source *src) {
  assert(src != NULL);
  unsigned int c0 = utf8_get(src);
  if (c0 == UTF8_END || c0 == UTF8_ERROR) {
    return c0;
  }

  unsigned int c1 = 0;
  unsigned int c2 = 0;
  unsigned int c3 = 0;
  unsigned int r = 0;

  // With zero continuations [0, 128]
  if ((c0 & 0x80u) == 0) {
    src->buf_pos += src->buf_pos < src->buf_len;
    return c0;
  }

  // With one continuation (128, 2047]
  if ((c0 & 0xE0u) == 0xC0u) {
    c1 = utf8_cont(src);
    if (c1 >= 0) {
      r = ((c0 & 0x1Fu) << 6u) | c1;
      if (r >= 128u) {
        src->buf_pos += src->buf_pos < src->buf_len;
        return r;
      }
    }
  // With two continuations (2047, 55295] and (57344, 65535]
  } else if ((c0 & 0xF0u) == 0xE0u) {
    c1 = utf8_cont(src);
    c2 = utf8_cont(src);
    if ((c1 | c2) >= 0) {
      r = ((c0 & 0x0Fu) << 12u) | (c1 << 6u) | c2;
      if (r >= 2048u && (r < 55296u || r > 57343u)) {
        src->buf_pos += src->buf_pos < src->buf_len;
        return r;
      }
    }
  // With three continuations [65536, 1114111]
  } else if ((c0 & 0xF8u) == 0xF0u) {
    c1 = utf8_cont(src);
    c2 = utf8_cont(src);
    c3 = utf8_cont(src);
    if ((c1 | c2 | c3) >= 0) {
      r = ((c0 & 0x07u) << 18u) | (c1 << 12u) | (c2 << 6u) | c3;
      if (r >= 65536u && r <= 1114111u) {
        src->buf_pos += src->buf_pos < src->buf_len;
        return r;
      }
    }
  }

  return UTF8_ERROR;
}

int avoc_source_fwd(avoc_source *src) {
  assert(src != NULL);
  // We count two-by-two cps, so we need an extra item
  if (src->buf_pos > src->buf_len) {
    src->cur_cp = UTF8_END;
    return UTF8_END;
  }

  src->cur_cp_pos = src->nxt_cp_pos;
  if (src->buf_pos == 0L) {
    src->cur_cp = utf8_next_cp(src);
    src->nxt_cp_pos = src->buf_pos;
    src->nxt_cp = utf8_next_cp(src);
  } else {
    src->cur_cp = src->nxt_cp;
    src->nxt_cp_pos = src->buf_pos;
    src->nxt_cp = utf8_next_cp(src);
  }

  return src->cur_cp;
}






