#include "avocc.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
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
  src->row = 1L;
  src->col = 1L;

  if (name != NULL) {
    size_t name_len = strlen(name)+1;
    src->name = calloc(name_len, sizeof(char));
    memset(src->name, 0L, name_len);
    memcpy(src->name, name, name_len);
  } else {
    src->name = NULL;
  }
}

void avoc_token_init(avoc_token *token) {
  token->type = TOKEN_EOF;
  token->lit_type = LIT_BOL;
  token->start_pos = 0L;
  token->length = 0L;
}

void avoc_item_init(avoc_item *item) {
  assert(item != NULL);
  item->type = 0;
  item->as_u64 = 0UL;
  item->token_pos = 0L;
  item->token_length = 0L;
  item->next_sibling = NULL;
  item->prev_sibling = NULL;
}

void avoc_list_init(avoc_list *list) {
  assert(list != NULL);
  list->head = NULL;
  list->tail = NULL;
  list->item_count = 0L;
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

void avoc_item_free(avoc_item *item) {
  assert(item != NULL);
  // all of those are the same, but let's keep it named
  switch (item->type) {
    case ITEM_LIT_STR:
      free(item->as_str);
      break;
    case ITEM_SYM:
      free(item->as_sym);
      break;
    case ITEM_LIST:
      avoc_list_free(item->as_list);
      free(item->as_list);
      break;
    default:
      break;
  }
}

void avoc_list_free(avoc_list *list) {
  assert(list != NULL);
  if (list->head != NULL) {
    free(list->head);
  }

  if (list->tail != NULL) {
    free(list->tail);
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

static int utf8_cp_size(int cp) {
  int len = -1;
  if (cp <= 128) {
    len = 1;
  } else if (cp > 128 && cp <= 2047) {
    len = 2;
  } else if ((cp > 2047 && cp <= 55295) || (cp > 57344 && cp <= 65535)) {
    len = 3;
  } else if (cp > 65535 && cp <= 1114111) {
    len = 4;
  }
   return len;
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

  if (src->cur_cp == '\n') {
    src->row ++;
    src->col = 1L;
  } else {
    src->col ++;
  }

  return src->cur_cp;
}

avoc_status avoc_next_token(avoc_source *src, avoc_token *token) {
  assert(src != NULL);
  assert(token != NULL);

  avoc_token_init(token);

  int cur = avoc_source_fwd(src);
  // clean whitespaces
  while (isspace(cur) && cur != '\n' && cur != EOF) {
    cur = avoc_source_fwd(src);
  }

  int allow_newl = 0;
  token->start_pos = src->cur_cp_pos;
  int cp_size = utf8_cp_size(cur);
  if (cp_size == -1) {
    PRINT_ERROR(src, "utf-8 encoding error");
    return FAILED;
  }
  token->length += cp_size;

  switch (cur) {
    case UTF8_END:
      token->type = TOKEN_EOF;
      token->length = 0L;
      return OK;
    case '\n':
      token->type = TOKEN_EOL;
      return OK;
    case ':':
      token->type = TOKEN_COLON;
      return OK;
    case '<':
    case '[':
    case '(':
      token->type = TOKEN_LSTART;
      return OK;
    case '>':
    case ']':
    case ')':
      token->type = TOKEN_LEND;
      return OK;
    case ';':
      token->type = TOKEN_COMMENT;
      allow_newl = src->nxt_cp == ';';

      while ((cur = avoc_source_fwd(src)) != UTF8_END) {
        if (cur == UTF8_ERROR) {
          PRINT_ERROR(src, "utf-8 encoding error");
          return FAILED;
        }

        cp_size = utf8_cp_size(cur);
        if (cp_size == -1) {
          PRINT_ERROR(src, "utf-8 encoding error");
          return FAILED;
        }

        token->length += cp_size;
        if (cur == '\n' && !allow_newl) {
          break;
        }

        if (cur == ';' && src->nxt_cp == ';' && allow_newl) {
          token->length += cp_size;
          avoc_source_fwd(src);
          break;
        }
      }

      if (cur == UTF8_END && !allow_newl) {
        PRINT_ERROR(src, "unterminated comment");
        return FAILED;
      }

      return OK;
    case '"':
      token->type = TOKEN_LIT;
      token->lit_type = LIT_STR;

      while ((cur = avoc_source_fwd(src)) != UTF8_END) {
        if (cur == UTF8_ERROR) {
          PRINT_ERROR(src, "utf-8 encoding error");
          return FAILED;
        }

        if (cur == '\n') {
          PRINT_ERROR(src, "unterminated string");
          return FAILED;
        }

        cp_size = utf8_cp_size(cur);
        if (cp_size == -1) {
          PRINT_ERROR(src, "utf-8 encoding error");
          return FAILED;
        }

        token->length += cp_size;
        if (cur == '\\' && src->nxt_cp == '"') {
          avoc_source_fwd(src);
          token->length += utf8_cp_size('"');
          continue;
        }

        if (src->nxt_cp == '"') {
          avoc_source_fwd(src);
          token->length += utf8_cp_size('"'); // Include the trailing '"'
          break;
        }
      }

      if (cur == UTF8_END) {
        PRINT_ERROR(src, "unterminated string");
        return FAILED;
      }

      return OK;
    case '\'':
    case '`':
      PRINT_ERROR(src, "string delimited by ` or ' are reserved and not supported yet");
      return FAILED;
    case '{':
    case '}':
      PRINT_ERROR(src, "lists delimited by curly braces '{}' are not supported yet");
      return FAILED;
    case UTF8_ERROR:
      PRINT_ERROR(src, "utf-8 encoding error");
      return FAILED;
    default:
      token->length = 0L;
      do {
        if (cur == UTF8_ERROR) {
          PRINT_ERROR(src, "utf-8 encoding error");
          return FAILED;
        }

        token->length += utf8_cp_size(cur);
        if (src->nxt_cp == ':' || isspace(src->nxt_cp)) {
          break;
        }
      } while ((cur = avoc_source_fwd(src)) != UTF8_END);

      const char *str_start = (const char *) src->buf_data + token->start_pos;
      const size_t str_len = token->length;
      if (str_len >= 1) {
        if ((strncmp("false", str_start, 5) == 0 && str_len == 5) ||
            (strncmp("true", str_start, 4) == 0 && str_len == 4)) {
          token->type = TOKEN_LIT;
          token->lit_type = LIT_BOL;
          return OK;
        }

        if (strncmp("nil", str_start, 3) == 0 && str_len == 3) {
          token->type = TOKEN_NIL;
          return OK;
        }

        if (isdigit(str_start[0]) ||
            (str_len > 2 && strncmp("0x", str_start, 2) == 0) ||
            (str_len > 2 && strncmp("0b", str_start, 2) == 0) ||
            (str_len > 2 && strncmp("0o", str_start, 2) == 0) ||
            (str_len > 2 && strncmp("-.", str_start, 2) == 0 && isdigit(str_start[2])) ||
            (str_len > 1 && (str_start[0] == '.' || str_start[0] == '-') && isdigit(str_start[1])) ) {
          token->type = TOKEN_LIT;
          token->lit_type = LIT_NUM;
          return OK;
        }
      }

      token->type = TOKEN_ID;
      break;
  }

  return OK;
}

void avoc_list_push(avoc_list *dest, avoc_item *item) {
  assert(dest != NULL);
  assert(item != NULL);
  dest->item_count ++;

  if (dest->head == NULL) {
    dest->head = item;
  }

  if (dest->tail == NULL) {
    dest->tail = item;
  } else {
    item->prev_sibling = dest->tail;
    dest->tail->next_sibling = item;
    dest->tail = item;
  }
}

void avoc_list_merge(avoc_list *left, avoc_list *right) {
  assert(left != NULL);
  assert(right != NULL);
  avoc_list_push(left, right->head);
}

