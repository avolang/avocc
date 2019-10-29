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

// Token type
typedef enum {
  TOKEN_EOF,
  TOKEN_EOL,
  TOKEN_COLON,
  TOKEN_LSTART,
  TOKEN_LEND,
  TOKEN_NIL,
  TOKEN_LIT_NUM,
  TOKEN_LIT_STR,
  TOKEN_LIT_BOL,
  TOKEN_ID,
  TOKEN_COMMENT,
} avoc_token_type;

// Token reference
typedef struct _avoc_token {
  avoc_token_type type; // Type of this token
  size_t offset; // Offset
  size_t length; // Length
  size_t auxlen; // Auxiliar length (for escaping characters)
} avoc_token;

struct _avoc_list;

typedef struct _avoc_item {
  enum {
    ITEM_LIT_BOL,
    ITEM_LIT_U32,
    ITEM_LIT_U64,
    ITEM_LIT_I32,
    ITEM_LIT_I64,
    ITEM_LIT_F32,
    ITEM_LIT_F64,
    ITEM_LIT_STR,
    ITEM_SYM,
    ITEM_LIST,
    ITEM_COMMENT,
  } type;

  union {
    short as_bol;
    unsigned int as_u32;
    unsigned long as_u64;
    int as_i32;
    long as_i64;
    float as_f32;
    double as_f64;
    char *as_str;
    char *as_sym;
    struct _avoc_list *as_list;
  };

  struct _avoc_list* sym_composed_type; // Composed type definition. i.e. a::(T ...)
  char *sym_ordinary_type; // Ordinary type definition i.e. a::T

  struct _avoc_item *next_sibling; // when used as item, this is the next element
  struct _avoc_item *prev_sibling; // when used as item, this is the next element
} avoc_item;

typedef struct _avoc_list {
  struct _avoc_item *head;
  struct _avoc_item *tail;
  size_t item_count;
} avoc_list;

__attribute__((unused)) static const char *token_type_names[] = {
  "EOF",
  "EOL",
  "COLON",
  "LSTART",
  "LEND",
  "NIL",
  "LIT_NUM",
  "LIT_STR",
  "LIT_BOL",
  "ID",
  "COMMENT",
};

#define UTF8_END (-1)
#define UTF8_ERROR (-2)
#define PRINT_ERROR(src, msg)                                                  \
  fprintf(stderr, "%s:%ld:%ld: " msg "\n", (src)->name, (src)->row, (src)->col)

#define PRINT_ERRORF(src, msg, ...)                                            \
  fprintf(stderr, "%s:%ld:%ld: " msg "\n", (src)->name, (src)->row,            \
          (src)->col, __VA_ARGS__)

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

// Initializes a item
void avoc_item_init(avoc_item *item);

// Initializes a list
void avoc_list_init(avoc_list *list);

// Frees the resources of a src without freeing the src itself.
void avoc_source_free(avoc_source *src);

// Frees the resources of an item without freeing the item itself.
void avoc_item_free(avoc_item *item);

// Frees the resources of an list without freeing the list itself.
void avoc_list_free(avoc_list *list);

// Moves forward into the buffer, storing cur_cp and nxt_cp.
int avoc_source_fwd(avoc_source *src);

// Get a token from the current position of the buffer (in src, out token)
avoc_status avoc_next_token(avoc_source *src, avoc_token *token);

// Pushes an item into the dest list, the item must be an initialized valid
// memory address.
void avoc_list_push(avoc_list *dest, avoc_item *item);

// Merges the right list into the left keeping its order as argumented.
void avoc_list_merge(avoc_list *left, avoc_list *right);

// Parse a literal, such strings, numbers and booleans, out: item.
avoc_status avoc_parse_lit(avoc_source *src, avoc_token *token, avoc_item *item);

// Parse a symbol, out: item.
avoc_status avoc_parse_sym(avoc_source *src, avoc_token *token, avoc_item *item);

// Parse an item, such literal, comment, nil or symbol, out: item.
avoc_status avoc_parse_item(avoc_source *src, avoc_token *token, avoc_item *item);

// Parse a list, out: list.
avoc_status avoc_parse_list(avoc_source *src, avoc_token *token, avoc_list *list);

// Parse a source.
avoc_status avoc_parse_source(avoc_source *src, avoc_list *list);

#endif /* AVOCC_H */
