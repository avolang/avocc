#ifndef AVOCC_H // NOLINT
#define AVOCC_H

#include <stddef.h> // size_t
#include <stdio.h>  // fprintf()

// Contains the state of a source code buffer
typedef struct _avoc_source {
  unsigned char *buf_data;
  size_t buf_len; // Buffer total length
  size_t buf_pos; // Cursor position (two by two code points)
  size_t cur_pos; // Current code point position

  int cur_cp; // Current code point value
  int nxt_cp; // Next code point value

  long cur_cp_pos; // Current codepoint position
  long nxt_cp_pos; // Next codepoint position

  size_t row;
  size_t col;
  char *name;
} avoc_source;

// Function result status
typedef enum { OK, FAILED } avoc_status;

// Token reference
typedef struct _avoc_token {
  enum {
    TOKEN_EOF,
    TOKEN_EOL,
    TOKEN_COLON,
    TOKEN_LSTART,
    TOKEN_LEND,
    TOKEN_COMMENT,
    TOKEN_NIL,
    TOKEN_LIT,
    TOKEN_ID,
  } type;

  enum {
    LIT_BOL,
    LIT_NUM,
    LIT_STR,
  } lit_type;

  size_t start_pos;
  size_t length;
} avoc_token;

__attribute__((unused)) static const char *token_type_names[] = {
    "EOF", "EOL", "COLON", "LSTART", "LEND", "COMMENT", "NIL", "LIT", "ID",
};

__attribute__((unused)) static const char *lit_type_names[] = {"BOL", "NUM",
                                                               "STR"};

#define UTF8_END (-1)
#define UTF8_ERROR (-2)
#define PRINT_ERROR(src, msg)                                                  \
  fprintf(stderr, "%s:%ld:%ld: " msg "\n", (src)->name, (src)->row, (src)->col)

#define PRINT_ERRORF(src, msg, ...)                                            \
  fprintf(stderr, "%s:%ld:%ld: " msg "\n", (src)->name, (src)->row,            \
          (src)->col, ##__VA_ARGS__)

#define PRINT_UNEXPECTED_CHAR_ERROR(src, expected, given)                      \
  fprintf(stderr,                                                              \
          "%s:%ld:%ld: unexpected character, expected: %c, given: %c\n",       \
          (src)->name, (src)->row, (src)->col, token_type_names[(expected)],   \
          token_type_names[(given)])

#define PRINT_UNEXPECTED_TOKEN_ERROR(src, expected, given)                     \
  fprintf(stderr, "%s:%ld:%ld: unexpected token, expected: %s, given: %s\n",   \
          (src)->name, (src)->row, (src)->col, token_type_names[(expected)],   \
          token_type_names[(given)])

// Initializes a source copying the values into memory.
void avoc_source_init(avoc_source *src, const char *name, const char *buf_data,
                      size_t buf_len);

// Initializes a token setting its value to zero.
void avoc_token_init(avoc_token *token);

// Frees the resources of a src without freeing the src itself.
void avoc_source_free(avoc_source *src);

// Moves forward into the buffer, storing cur_cp and nxt_cp.
int avoc_source_fwd(avoc_source *src);

// Get a token from the current position of the buffer (in src, out token)
avoc_status avoc_next_token(avoc_source *src, avoc_token *token);

#endif /* AVOCC_H */
