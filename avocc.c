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
  token->unqlen = 0L;
}

void avoc_item_init(avoc_item *item) {
  assert(item != NULL);
  item->type = 0;
  item->as_u64 = 0UL;
  item->token_pos = 0L;
  item->token_length = 0L;
  item->next_sibling = NULL;
  item->prev_sibling = NULL;
  item->sym_composed_type = NULL;
  item->sym_ordinary_type = NULL;
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
      if (item->sym_ordinary_type != NULL) {
        free(item->sym_ordinary_type);
      }

      if (item->sym_composed_type != NULL) {
        avoc_list_free(item->sym_composed_type);
        free(item->sym_composed_type);
      }
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

static int utf8_get(avoc_source *src) {
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
  int c0 = utf8_get(src);
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

int utf8_encode(char *dest, unsigned ch) {
    if (ch < 0x80) {
        dest[0] = (char)ch;
        return 1;
    }
    if (ch < 0x800) {
        dest[0] = (ch>>6) | 0xC0;
        dest[1] = (ch & 0x3F) | 0x80;
        return 2;
    }
    if (ch < 0x10000) {
        dest[0] = (ch>>12) | 0xE0;
        dest[1] = ((ch>>6) & 0x3F) | 0x80;
        dest[2] = (ch & 0x3F) | 0x80;
        return 3;
    }
    if (ch < 0x110000) {
        dest[0] = (ch>>18) | 0xF0;
        dest[1] = ((ch>>12) & 0x3F) | 0x80;
        dest[2] = ((ch>>6) & 0x3F) | 0x80;
        dest[3] = (ch & 0x3F) | 0x80;
        return 4;
    }
    return 0;
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
  int terminator = 0;
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
      token->type = TOKEN_HLS;
      return OK;
    case '>':
    case ']':
    case ')':
      token->type = TOKEN_HLE;
      return OK;
    case '}':
    case '{':
      PRINT_ERROR(src, "lists with curly braces delimiters {} are reserved and not supported");
      return FAILED;
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
    case '\'':
    case '`':
    case '"':
      token->type = TOKEN_LIT;
      token->lit_type = LIT_STR;
      terminator = cur;
      if (cur == '`') {
        allow_newl = 1;
      }

      while ((cur = avoc_source_fwd(src)) != UTF8_END) {
        if (cur == UTF8_ERROR) {
          PRINT_ERROR(src, "utf-8 encoding error");
          return FAILED;
        }

        if (cur == '\n' && !allow_newl) {
          PRINT_ERROR(src, "unterminated string");
          return FAILED;
        }

        cp_size = utf8_cp_size(cur);
        if (cp_size == -1) {
          PRINT_ERROR(src, "utf-8 encoding error");
          return FAILED;
        }

        if (cur == '\\') {
          char codebuf[9] = "\0\0\0\0\0\0\0\0\0";
          int taken_cps = 0;
          if (src->nxt_cp == terminator || strchr("abefnrtv\\?", src->nxt_cp) != NULL) {
            taken_cps = 1;
          } else if (src->nxt_cp == 'x') {
            taken_cps = 3;
          } else if (src->nxt_cp == 'u') {
            taken_cps = 5;
            if ((src->buf_pos + token->start_pos + 7) < (src->buf_pos + src->buf_len)) {
              memcpy(codebuf, (const char *) (src->buf_data + token->start_pos + 3), 4);
            }
          } else if (src->nxt_cp == 'U') {
            taken_cps = 9;
            if ((src->buf_pos + token->start_pos + 11) < (src->buf_pos + src->buf_len)) {
              memcpy(codebuf, (const char *) (src->buf_data + token->start_pos + 3), 8);
            }
          } else {
            PRINT_ERRORF(src, "unknown escape sequence: \\%c", src->nxt_cp);
            return FAILED;
          }

          // Accept this character '\\'
          token->length += cp_size;

          // Predict length based on the codepoint size of the future unescaped character
          if (codebuf[0] != 0) {
            int codelen = (int) strtol(codebuf, NULL, 16);
            token->unqlen += utf8_cp_size(codelen);
          } else {
            token->unqlen += cp_size;
          }

          // Following escape sequence after '\\': 'a', 'x12', 'u1234', 'U12345678'
          for (int i=0;i<taken_cps;i++) {
            int cp = avoc_source_fwd(src);
            token->length += utf8_cp_size(cp);
          }

          continue;
        } else {
          token->length += cp_size;
          token->unqlen += cp_size;
        }

        if (cur == terminator) {
          token->unqlen--;
          break;
        }
      }

      if (cur == UTF8_END) {
        PRINT_ERROR(src, "unterminated string");
        return FAILED;
      }

      return OK;
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
        if (strchr(":<[({})]>", src->nxt_cp) != NULL || isspace(src->nxt_cp)) {
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
  left->item_count = left->item_count + right->item_count;
  if (left->tail == NULL) {
    left->tail = right->head;
  } else {
    left->tail->next_sibling = right->head;
    right->head->prev_sibling = left->tail;
    left->tail = right->tail;
  }
}

avoc_status avoc_parse_lit(avoc_source *src, avoc_token *token,
                               avoc_item *item) {
  assert(src != NULL);
  assert(token != NULL);
  assert(item != NULL);
  assert(token->type == TOKEN_LIT);

  const char *contents = (const char *) src->buf_data + token->start_pos;
  char *contents_cpy = NULL;
  size_t contents_len = token->length;
  enum { BASE_BIN, BASE_OCT, BASE_DEC, BASE_HEX } num_base = BASE_DEC;
  const int bases[] = {2, 8, 10, 16};
  const char *digits[] = {"01", "01234567", "0123456789", "0123456789ABCDEF"};
  int allow_neg_exp = 1;
  int allow_float = 1;
  int is_float = 0;
  int is_neg = 0;
  int has_exp = 0;

  switch (token->lit_type)  {
    case LIT_BOL:
      item->type = ITEM_LIT_BOL;
      if (strncmp("true", contents, 4) == 0 && token->length == 4) {
        item->as_bol = 1;
      } else if (strncmp("false", contents, 5) == 0 && token->length == 5) {
        item->as_bol = 0;
      } else {
        PRINT_ERRORF(src, "boolean literal neither 'true' or 'false': %s", contents);
        return FAILED;
      }
      break;
    case LIT_NUM:
      item->type = ITEM_LIT_I32;
      if (contents_len >= 1 && contents[0] == '-') {
        is_neg = 1;
        contents += 1;
        contents_len -= 1;
      }

      if (contents_len >= 2) {
        if (strncmp("0b", contents, 2) == 0) {
          num_base = BASE_BIN;
          contents += 2;
          contents_len -= 2;
          allow_float = 0;
          allow_neg_exp = 0;
        } else if (strncmp("0o", contents, 2) == 0) {
          num_base = BASE_OCT;
          contents += 2;
          contents_len -= 2;
          allow_float = 0;
        } else if (strncmp("0x", contents, 2) == 0) {
          num_base = BASE_HEX;
          contents += 2;
          contents_len -= 2;
          allow_float = 0;
          allow_neg_exp = 0;
        }
      }

      for (size_t i = 0; i < contents_len; i++) {
        char digit = contents[i];
        if (digit == '.') {
          if (allow_float && !is_float) {
            is_float = 1;
            continue;
          } else if (is_float) {
            PRINT_ERROR(src, "repeated floating point for this constant");
            return FAILED;
          } else if (!allow_float) {
            PRINT_ERROR(src, "unexpected floating point for this constant");
            return FAILED;
          }
        }

        if ((digit == 'i' || digit == 'f' || digit == 'u') && (i + 2) < contents_len) {
          const char *suffix = (contents+i);
          if (strncmp("i32", suffix, 2) == 0) {
            if (is_float) {
              PRINT_ERROR(src, "i32 literals cannot have floating poing");
              return FAILED;
            }

            item->type = ITEM_LIT_I32;
          } else if (strncmp("i64", suffix, 2) == 0) {
            if (is_float) {
              PRINT_ERROR(src, "i64 literals cannot have floating poing");
              return FAILED;
            }

            item->type = ITEM_LIT_I64;
          } else if (strncmp("u32", suffix, 2) == 0) {
            if (is_float) {
              PRINT_ERROR(src, "u32 literals cannot have floating poing");
              return FAILED;
            }
            if (is_neg) {
              PRINT_ERROR(src, "u32 literals cannot have negative sign");
              return FAILED;
            }

            item->type = ITEM_LIT_U32;
          } else if (strncmp("u64", suffix, 2) == 0) {
            if (is_float) {
              PRINT_ERROR(src, "u64 literals cannot have floating poing");
              return FAILED;
            }
            if (is_neg) {
              PRINT_ERROR(src, "u64 literals cannot have negative sign");
              return FAILED;
            }

            item->type = ITEM_LIT_U64;
          } else if (strncmp("f32", suffix, 2) == 0) {
            item->type = ITEM_LIT_F32;
          } else if (strncmp("f64", suffix, 2) == 0) {
            item->type = ITEM_LIT_F64;
          } else {
            PRINT_ERROR(src, "numeric literal suffix is invalid or not supported");
            return FAILED;
          }

          contents_len -= 3;
          continue;
        } else if (digit == 'i' || digit == 'f' || digit == 'u') {
          PRINT_ERROR(src, "numeric literal is incomplete, invalid suffix");
          return FAILED;
        }

        if (digit == 'e') {
          if (has_exp) {
            PRINT_ERROR(src, "numeric literal already has an exponent");
            return FAILED;
          }

          has_exp = 1;
          continue;
        }

        if (digit == '-' && !has_exp) {
          PRINT_ERROR(src, "numeric literal already has an exponent");
          return FAILED;
        } else if (digit == '-' && has_exp && !allow_neg_exp) {
          PRINT_ERROR(src, "unexpected negative exponent for this constant");
          return FAILED;
        } else if (digit == '-' && has_exp){
          continue;
        }

        if (strchr(digits[num_base], digit) == NULL) {
          PRINT_ERRORF(src, "invalid char '%c' for this numeric base", digit);
          return FAILED;
        }
      }

      if (contents_len == 0) {
        PRINT_ERROR(src, "numeric literal is incomplete, does not contain any digits");
        return FAILED;
      }

      if (is_float && item->type != ITEM_LIT_F32 && item->type != ITEM_LIT_F64) {
        item->type = ITEM_LIT_F32;
      }

      contents_cpy = calloc(contents_len+1, sizeof(char));
      memset(contents_cpy, 0L, contents_len+1);
      memcpy(contents_cpy, contents, contents_len);

      switch (item->type) {
        case ITEM_LIT_I32:
          item->as_i32 = (int) strtol(contents_cpy, NULL, bases[num_base]);
          item->as_i32 = is_neg ? -item->as_i32 : item->as_i32;
          break;
        case ITEM_LIT_I64:
          item->as_i64 = strtol(contents_cpy, NULL, bases[num_base]);
          item->as_i64 = is_neg ? -item->as_i64 : item->as_i64;
          break;
        case ITEM_LIT_U32:
          item->as_u32 = (unsigned int) strtoul(contents_cpy, NULL, bases[num_base]);
          break;
        case ITEM_LIT_U64:
          item->as_u64 = strtoul(contents_cpy, NULL, bases[num_base]);
          break;
        case ITEM_LIT_F32:
          item->as_f32 = strtof(contents_cpy, NULL);
          item->as_f32 = is_neg ? -item->as_f32 : item->as_f32;
          break;
        case ITEM_LIT_F64:
          item->as_f64 = strtod(contents_cpy, NULL);
          item->as_f64 = is_neg ? -item->as_f64 : item->as_f64;
          break;
        default:
          PRINT_ERROR(src, "unexpected literal type, please report this bug");
          return FAILED;
      }

      free(contents_cpy);
      break;
    case LIT_STR:
      item->type = ITEM_LIT_STR;
      contents_cpy = calloc(token->unqlen+1, sizeof(char));
      memset(contents_cpy, 0L, contents_len+1);

      for (size_t i = 1, j = 0; i<token->length-1; i++, j++) {
        int peek = contents[i];
        int mask = peek & 0xFF;
        int skip = 0;
        if ((mask & 0x80) == 0) {
          skip = 0;
          if (peek == '\\' && i<token->length-2 && contents[0] != '`') {
            peek = contents[++i];
            char codebuf[9] = "\0\0\0\0\0\0\0\0\0";
            int ch = 0;
            int take = 0;
            switch (peek) {
              case 'e':
              case '\\':
                ch = '\\';
                break;
              case 'a':
                ch = '\a';
                break;
              case 'b':
                ch = '\b';
                break;
              case 'f':
                ch = '\f';
                break;
              case 'n':
                ch = '\n';
                break;
              case 'r':
                ch = '\r';
                break;
              case 't':
                ch = '\t';
                break;
              case 'v':
                ch = '\v';
                break;
              case '"':
                ch = '"';
                break;
              case '\'':
                ch = '\'';
                break;
              case 'x':
                take = 2;
                break;
              case 'u':
                take = 4;
                break;
              case 'U':
                take = 8;
                break;
              default:
                PRINT_ERROR(src, "unknown escape sequence");
                return FAILED;
            }

            if (take > 0) {
              for (int k = 0; k < take; k++) {
                codebuf[k] = contents[++i];
              }

              ch = strtol(codebuf, NULL, 16);
            }

            int cp_size = utf8_encode(contents_cpy+j, ch);
            j += cp_size - 1;
            continue;
          } else if (peek == '\\' && contents[0] != '`') {
            PRINT_ERROR(src, "unterminated escape sequence");
            return FAILED;
          }
        } else if ((mask & 0xE0) == 0xC0) {
          skip = 1;
        } else if ((mask & 0xF0) == 0xE0) {
          skip = 2;
        } else if ((mask & 0xF8) == 0xF0) {
          skip = 3;
        } else {
          PRINT_ERROR(src, "utf-8 decode error");
          return FAILED;
        }

        contents_cpy[j] = contents[i];
        for (int k=0;k<skip;k++) {
          i++;
          contents_cpy[k] = contents[i];
        }
      }

      item->as_str = contents_cpy;
      break;
  }

  return OK;
}

avoc_status avoc_parse_sym(avoc_source *src, avoc_token *token,
                           avoc_item *item) {
  assert(src != NULL);
  assert(token != NULL);
  assert(item != NULL);
  assert(token->type == TOKEN_ID);
  avoc_status status = OK;

  item->type = ITEM_SYM;
  item->as_sym = calloc(token->length, sizeof(char)+1);
  memset(item->as_sym, 0L, token->length+1);
  memcpy(item->as_sym, src->buf_data + token->start_pos, token->length);

  status = avoc_next_token(src, token);
  if (status != OK) {
    return status;
  }

  if (token->type == TOKEN_COLON) {
    do {
      status = avoc_next_token(src, token);
      if (status != OK) {
        return status;
      }
    } while (token->type == TOKEN_EOL || token->type == TOKEN_COMMENT);

    if (token->type == TOKEN_ID) {
      item->sym_ordinary_type = calloc(token->length, sizeof(char));
      memset(item->sym_ordinary_type, 0L, token->length+1);
      memcpy(item->sym_ordinary_type, src->buf_data + token->start_pos, token->length);
    } else if (token->type == TOKEN_HLS) {
      item->sym_composed_type = malloc(sizeof(avoc_item));
      avoc_list_init(item->sym_composed_type);
      status = avoc_parse_list(src, token, item->sym_composed_type);
    } else {
      PRINT_UNEXPECTED_TOKEN_ERROR(src, TOKEN_ID, token->type);
      PRINT_UNEXPECTED_TOKEN_ERROR(src, TOKEN_HLS, token->type);
      return FAILED;
    }

    status = avoc_next_token(src, token);
    if (status != OK) {
      return status;
    }
  }

  return status;
}

avoc_status avoc_parse_comment(avoc_source *src, avoc_token *token,
                           avoc_item *item) {
  assert(src != NULL);
  assert(token != NULL);
  assert(item != NULL);

  item->type = ITEM_COMMENT;
  item->as_str = calloc(token->length, sizeof(char)+1);
  memset(item->as_str, 0L, token->length+1);
  memcpy(item->as_str, src->buf_data + token->start_pos, token->length);

  return OK;
}

avoc_status avoc_parse_item(avoc_source *src, avoc_token *token, avoc_item *item) {
  assert(src != NULL);
  assert(token != NULL);
  assert(item != NULL);
  avoc_status status = OK;

  switch (token->type) {
    case TOKEN_ID:
      status = avoc_parse_sym(src, token, item);
      break;
    case TOKEN_HLS:
      item->as_list = malloc(sizeof(avoc_list));
      item->type = ITEM_LIST;
      avoc_list_init(item->as_list);
      status = avoc_parse_list(src, token, item->as_list);
      break;
    case TOKEN_COMMENT:
      status = avoc_parse_comment(src, token, item);
      if (status != OK) {
        return OK;
      }

      status = avoc_next_token(src, token);
      break;
    case TOKEN_LIT:
      status = avoc_parse_lit(src, token, item);
      if (status != OK) {
        return OK;
      }

      status = avoc_next_token(src, token);
      break;
    default:
      return OK;
  }

  return status;
}

avoc_status avoc_parse_list(avoc_source *src, avoc_token *token, avoc_list *list) {
  assert(src != NULL);
  assert(token != NULL);
  assert(list != NULL);

  avoc_status status = avoc_next_token(src, token);
  if (status != OK) {
    return status;
  }

  while (token->type != TOKEN_EOF && token->type != TOKEN_HLE) {
    if (token->type == TOKEN_EOL) {
      status = avoc_next_token(src, token);
      if (status != OK) {
        return status;
      }

      continue;
    }

    avoc_item *item = malloc(sizeof(avoc_item));
    avoc_item_init(item);

    status = avoc_parse_item(src, token, item);
    if (status != OK) {
      return status;
    }

    avoc_list_push(list, item);
  }

  if (token->type == TOKEN_EOF) {
    PRINT_UNEXPECTED_TOKEN_ERROR(src, TOKEN_HLE, token->type);
    return FAILED;
  }

  return OK;
}

avoc_status avoc_parse_source(avoc_source *src, avoc_list *list) {
  assert(src != NULL);
  assert(list != NULL);

  avoc_token token;
  avoc_token_init(&token);

  avoc_status status = avoc_next_token(src, &token);
  if (status != OK) {
    return status;
  }

  while (token.type != TOKEN_EOF) {
    if (token.type == TOKEN_EOL || token.type == TOKEN_HLE) {
      status = avoc_next_token(src, &token);
      if (status != OK) {
        return status;
      }

      continue;
    }

    if (token.type == TOKEN_HLS) {
      avoc_list *child = malloc(sizeof(avoc_list));
      avoc_list_init(child);

      status = avoc_parse_list(src, &token, child);
      if (status != OK) {
        return status;
      }

      avoc_item *child_item = malloc(sizeof(avoc_item));
      avoc_item_init(child_item);
      child_item->type = ITEM_LIST;
      child_item->as_list = child;
      avoc_list_push(list, child_item);
    } else {
      PRINT_UNEXPECTED_TOKEN_ERROR(src, TOKEN_HLS, token.type);
      return FAILED;
    }
  }

  return OK;
}
