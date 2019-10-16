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
    TOKEN_HLS,
    TOKEN_HLE,
    TOKEN_VLS,
    TOKEN_VLE,
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

  size_t token_pos;
  size_t token_length;
  struct _avoc_item *next_sibling;
  struct _avoc_item *prev_sibling;
} avoc_item;

typedef struct _avoc_list {
  struct _avoc_item *head;
  struct _avoc_item *tail;
  size_t item_count;
} avoc_list;

__attribute__((unused)) static const char *token_type_names[] = {
    "EOF", "EOL",     "COLON", "HLS", "HLE", "VLS",
    "VLE", "COMMENT", "NIL",   "LIT", "ID",
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

// Parse a boolean literal
avoc_status avoc_parse_bol_lit(avoc_source *src, avoc_token *token,
                               avoc_item *item);

// Parse a numeric literal
avoc_status avoc_parse_num_lit(avoc_source *src, avoc_token *token,
                               avoc_item *item);

// Parse a string literal
avoc_status avoc_parse_str_lit(avoc_source *src, avoc_token *token,
                               avoc_item *item);

// Parse a symbol
avoc_status avoc_parse_sym(avoc_source *src, avoc_token *token,
                           avoc_item *item);

// Parse an item
avoc_status avoc_parse_item(avoc_source *src, avoc_item *item);

// Parse an horizontal list
avoc_status avoc_parse_hlst(avoc_source *src, avoc_list *list);

// Parse a line sub list
avoc_status avoc_parse_llst(avoc_source *src, avoc_list *list);

// Parse a vertical list
avoc_status avoc_parse_vlst(avoc_source *src, avoc_list *list);

// Parse a source
avoc_status avoc_parse_source(avoc_source *src, avoc_list *list);

#endif /* AVOCC_H */
