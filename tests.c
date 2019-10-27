#include "tests.h"
#include "avocc.h"
#include <stdio.h>
#include <string.h>

void test_source_init_free() {
  avoc_source src0;
  avoc_source_init(&src0, NULL, NULL, 0L);
  assert_okb(src0.buf_data == NULL);
  assert_eql(src0.buf_len, 0L);
  assert_eql(src0.buf_pos, 0L);
  assert_eq(src0.cur_cp, 0);
  assert_eq(src0.nxt_cp, 0);
  assert_eql(src0.cur_cp_pos, 0L);
  assert_eql(src0.nxt_cp_pos, 0L);
  assert_eql(src0.col, 1L);
  assert_eql(src0.row, 1L);
  assert_okb(src0.name == NULL);

  avoc_source src1;
  load_string(&src1, "avocado");

  assert_ok(src1.buf_data != NULL);
  assert_okb(src1.buf_len == 7L);
  assert_okb(src1.buf_data != NULL && strncmp("avocado", (char *) src1.buf_data, src1.buf_len) == 0);
  assert_eql(src1.buf_pos, 0L);
  assert_eq(src1.cur_cp, 0);
  assert_eq(src1.nxt_cp, 0);
  assert_eql(src1.cur_cp_pos, 0L);
  assert_eql(src1.nxt_cp_pos, 0L);
  assert_eql(src1.col, 1L);
  assert_eql(src1.row, 1L);

  avoc_source_free(&src1);
  assert_okb(src0.buf_data == NULL);
  assert_okb(src0.name == NULL);

  avoc_source src2;
  avoc_source_init(&src2, "filename", NULL, 0L);
  assert_okb(src2.name != NULL);
  assert_eqs(src2.name, "filename");
  avoc_source_free(&src2);
}

void test_source_move_fwd_ascii() {
  avoc_source src;
  load_string(&src, "ABC");

  assert_eql(src.buf_pos, 0L);
  assert_eq(src.cur_cp, 0);
  assert_eq(src.nxt_cp, 0);
  assert_eql(src.cur_cp_pos, 0L);
  assert_eql(src.nxt_cp_pos, 0L);

  avoc_source_fwd(&src);
  assert_eql(src.buf_pos, 2L);
  assert_eq(src.cur_cp, 'A');
  assert_eq(src.nxt_cp, 'B');
  assert_eql(src.cur_cp_pos, 0L);
  assert_eql(src.nxt_cp_pos, 1L);

  avoc_source_fwd(&src);
  assert_eql(src.buf_pos, 3L);
  assert_eq(src.cur_cp, 'B');
  assert_eq(src.nxt_cp, 'C');
  assert_eql(src.cur_cp_pos, 1L);
  assert_eql(src.nxt_cp_pos, 2L);

  avoc_source_fwd(&src);
  assert_eql(src.buf_pos, 3L);
  assert_eq(src.cur_cp, 'C');
  assert_eq(src.nxt_cp, UTF8_END);
  assert_eql(src.cur_cp_pos, 2L);
  assert_eql(src.nxt_cp_pos, 3L);

  avoc_source_fwd(&src);
  assert_eql(src.buf_pos, 3L);
  assert_eq(src.cur_cp, UTF8_END);
  assert_eq(src.nxt_cp, UTF8_END);
  assert_eql(src.cur_cp_pos, 3L);
  assert_eql(src.nxt_cp_pos, 3L);

  avoc_source_fwd(&src);
  assert_eql(src.buf_pos, 3L);
  assert_eq(src.cur_cp, UTF8_END);
  assert_eq(src.nxt_cp, UTF8_END);
  assert_eql(src.cur_cp_pos, 3L);
  assert_eql(src.nxt_cp_pos, 3L);

  avoc_source_free(&src);
}

void test_source_move_fwd_utf8() {
  avoc_source src;

  // One continuation (UTF-8)
  load_string(&src, "\xC2\xA1"); // U+00A1
  avoc_source_fwd(&src);
  assert_eql(src.buf_pos, 2L);
  assert_eq(src.cur_cp, 0x00A1);
  assert_eq(src.nxt_cp, UTF8_END);
  assert_eql(src.cur_cp_pos, 0L);
  assert_eql(src.nxt_cp_pos, 2L);
  avoc_source_free(&src);

  // Two continuations (UTF-8)
  load_string(&src, "\xE0\xA0\x80"); // U+0800
  avoc_source_fwd(&src);
  assert_eql(src.buf_pos, 3L);
  assert_eq(src.cur_cp, 0x0800);
  assert_eq(src.nxt_cp, UTF8_END);
  assert_eql(src.cur_cp_pos, 0L);
  assert_eql(src.nxt_cp_pos, 3L);
  avoc_source_free(&src);

  load_string(&src, "\xEE\x80\x80"); // U+E000 (reserved)
  avoc_source_fwd(&src);
  assert_eql(src.buf_pos, 3L);
  assert_eq(src.cur_cp, 0xE000);
  assert_eq(src.nxt_cp, UTF8_END);
  assert_eql(src.cur_cp_pos, 0L);
  assert_eql(src.nxt_cp_pos, 3L);
  avoc_source_free(&src);

  // Three continuations (UTF-8)
  load_string(&src, "\xF0\x90\x80\x81"); // U+10001
  avoc_source_fwd(&src);
  assert_eql(src.buf_pos, 4L);
  assert_eq(src.cur_cp, 0x10001);
  assert_eq(src.nxt_cp, UTF8_END);
  assert_eql(src.cur_cp_pos, 0L);
  assert_eql(src.nxt_cp_pos, 4L);
  avoc_source_free(&src);

  load_string(&src, "\xF0\x9F\x98\x8A"); // U+1F60A (smiling face)
  avoc_source_fwd(&src);
  assert_eql(src.buf_pos, 4L);
  assert_eq(src.cur_cp, 0x1F60A);
  assert_eq(src.nxt_cp, UTF8_END);
  assert_eql(src.cur_cp_pos, 0L);
  assert_eql(src.nxt_cp_pos, 4L);
  avoc_source_free(&src);

  // U+1F60A U+1F951 (smiling face, avocado)
  load_string(&src, "\xF0\x9F\x98\x8A\xF0\x9F\xA5\x91");
  avoc_source_fwd(&src);
  assert_eql(src.buf_pos, 8L);
  assert_eq(src.cur_cp, 0x1F60A);
  assert_eq(src.nxt_cp, 0x1F951);
  assert_eql(src.cur_cp_pos, 0L);
  assert_eql(src.nxt_cp_pos, 4L);
  avoc_source_free(&src);

  // 'A' U+1F951 'V' 'O'
  load_string(&src, "A\xF0\x9F\xA5\x91VO");
  avoc_source_fwd(&src);
  assert_eql(src.buf_pos, 5L);
  assert_eq(src.cur_cp, 'A');
  assert_eq(src.nxt_cp, 0x1F951);
  assert_eql(src.cur_cp_pos, 0L);
  assert_eql(src.nxt_cp_pos, 1L);
  avoc_source_fwd(&src);
  assert_eql(src.buf_pos, 6L);
  assert_eq(src.cur_cp, 0x1F951);
  assert_eq(src.nxt_cp, 'V');
  assert_eql(src.cur_cp_pos, 1L);
  assert_eql(src.nxt_cp_pos, 5L);
  avoc_source_fwd(&src);
  assert_eql(src.buf_pos, 7L);
  assert_eq(src.cur_cp, 'V');
  assert_eq(src.nxt_cp, 'O');
  assert_eql(src.cur_cp_pos, 5L);
  assert_eql(src.nxt_cp_pos, 6L);
  avoc_source_fwd(&src);
  assert_eql(src.buf_pos, 7L);
  assert_eq(src.cur_cp, 'O');
  assert_eq(src.nxt_cp, UTF8_END);
  assert_eql(src.cur_cp_pos, 6L);
  assert_eql(src.nxt_cp_pos, 7L);
  avoc_source_free(&src);
}

void test_source_move_fwd_newl() {
  avoc_source src;

  load_string(&src, "A\nB");
  assert_eql(src.row, 1L);
  assert_eql(src.col, 1L);

  avoc_source_fwd(&src);
  assert_eql(src.row, 1L);
  assert_eql(src.col, 2L);

  avoc_source_fwd(&src);
  assert_eql(src.row, 2L);
  assert_eql(src.col, 1L);
  avoc_source_free(&src);
}

void test_token_init() {
  avoc_token token;
  avoc_token_init(&token);
  assert_eq(token.type, 0);
  assert_eq(token.lit_type, 0);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 0L);
}

void test_token_next_singlechar() {
  avoc_source src;
  avoc_status status;
  avoc_token token;

  load_string(&src, "");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_EOF);
  avoc_source_free(&src);

  load_string(&src, "\n");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_EOL);
  avoc_source_free(&src);

  load_string(&src, ":");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_COLON);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 1L);
  avoc_source_free(&src);

  load_string(&src, "<[(");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_HLS);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 1L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_HLS);
  assert_eql(token.start_pos, 1L);
  assert_eql(token.length, 1L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_HLS);
  assert_eql(token.start_pos, 2L);
  assert_eql(token.length, 1L);
  avoc_source_free(&src);

  load_string(&src, ")]>");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_HLE);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 1L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_HLE);
  assert_eql(token.start_pos, 1L);
  assert_eql(token.length, 1L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_HLE);
  assert_eql(token.start_pos, 2L);
  assert_eql(token.length, 1L);
  avoc_source_free(&src);
}

void test_token_next_comments() {
  avoc_source src;
  avoc_status status;
  avoc_token token;

  load_string(&src, "; this is a line comment\n");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_COMMENT);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 25L);
  avoc_source_free(&src);

  load_string(&src, ";; this is a \n block comment ;;\n");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_COMMENT);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 31L);
  avoc_source_free(&src);

  load_string(&src, ";; a ;; ;; b ;; ;c\n");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_COMMENT);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 7L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_COMMENT);
  assert_eql(token.start_pos, 8L);
  assert_eql(token.length, 7L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_COMMENT);
  assert_eql(token.start_pos, 16L);
  assert_eql(token.length, 3L);
  avoc_source_free(&src);
}

void test_token_next_str_lit() {
  avoc_source src;
  avoc_status status;
  avoc_token token;

  load_string(&src, "\"string\"");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_STR);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 8L);
  avoc_source_free(&src);

  load_string(&src, "\"a\\\"b\"");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_STR);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 6L);
  avoc_source_free(&src);

  load_string(&src, "\"a\" \"b\"");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_STR);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 3L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_STR);
  assert_eql(token.start_pos, 4L);
  assert_eql(token.length, 3L);
  avoc_source_free(&src);

  load_string(&src, "\"\\n\"");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_STR);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 4L);
  assert_eql(token.unqlen, 1L);
  avoc_source_free(&src);

  load_string(&src, "\"\\\\\"");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_STR);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 4L);
  assert_eql(token.unqlen, 1L);
  avoc_source_free(&src);

  load_string(&src, "\"\\xFF\"");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_STR);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 6L);
  assert_eql(token.unqlen, 1L);
  avoc_source_free(&src);

  load_string(&src, "\"\\u00A1\"");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_STR);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 8L);
  assert_eql(token.unqlen, 2L);
  avoc_source_free(&src);
}

void test_token_next_num_lit() {
  avoc_source src;
  avoc_status status;
  avoc_token token;

  load_string(&src, "1234");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_NUM);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 4L);
  avoc_source_free(&src);

  load_string(&src, "1u32");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_NUM);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 4L);
  avoc_source_free(&src);

  load_string(&src, "0x12");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_NUM);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 4L);
  avoc_source_free(&src);

  load_string(&src, "0b01");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_NUM);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 4L);
  avoc_source_free(&src);

  load_string(&src, "0o66");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_NUM);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 4L);
  avoc_source_free(&src);

  load_string(&src, "0.12");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_NUM);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 4L);
  avoc_source_free(&src);

  load_string(&src, ".123");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_NUM);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 4L);
  avoc_source_free(&src);

  load_string(&src, "-.12");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_NUM);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 4L);
  avoc_source_free(&src);

  load_string(&src, "-.12f32");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_NUM);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 7L);
  avoc_source_free(&src);

  load_string(&src, "1 -2 .3 4i64");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_NUM);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 1L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_NUM);
  assert_eql(token.start_pos, 2L);
  assert_eql(token.length, 2L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_NUM);
  assert_eql(token.start_pos, 5L);
  assert_eql(token.length, 2L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_NUM);
  assert_eql(token.start_pos, 8L);
  assert_eql(token.length, 4L);
  avoc_source_free(&src);
}

void test_token_next_nilbol_lit() {
  avoc_source src;
  avoc_status status;
  avoc_token token;

  load_string(&src, "false true");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_BOL);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 5L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_BOL);
  assert_eql(token.start_pos, 6L);
  assert_eql(token.length, 4L);
  avoc_source_free(&src);

  load_string(&src, "falses trues");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_ID);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_ID);
  avoc_source_free(&src);

  load_string(&src, "nil nil");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_NIL);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 3L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_NIL);
  assert_eql(token.start_pos, 4L);
  assert_eql(token.length, 3L);
  avoc_source_free(&src);

  load_string(&src, "nils nils");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_ID);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 4L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_ID);
  assert_eql(token.start_pos, 5L);
  assert_eql(token.length, 4L);
  avoc_source_free(&src);
}

void test_token_next_id() {
  avoc_source src;
  avoc_status status;
  avoc_token token;

  load_string(&src, "var");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_ID);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 3L);
  avoc_source_free(&src);

  load_string(&src, "\xF0\x9F\xA5\x91");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_ID);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 4L);
  avoc_source_free(&src);

  load_string(&src, "\xF0\x9F\xA5\x91 A");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_ID);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 4L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_ID);
  assert_eql(token.start_pos, 5L);
  assert_eql(token.length, 1L);
  avoc_source_free(&src);
}

void test_token_edge_cases() {
  avoc_source src;
  avoc_status status;
  avoc_token token;

  load_string(&src, "-.nonum");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_ID);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 7L);
  avoc_source_free(&src);

  load_string(&src, "-nonum");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_ID);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 6L);
  avoc_source_free(&src);

  load_string(&src, ".nonum");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_ID);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 6L);
  avoc_source_free(&src);

  // this is not an error: a token number is detected,
  // however this should fail on parsing
  load_string(&src, "0xNUMBER?");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_NUM);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 9L);
  avoc_source_free(&src);

  load_string(&src, ";; \xF0\x9F\xA5\x91 ;;");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_COMMENT);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 10L);
  avoc_source_free(&src);

  load_string(&src, ";; \xF0\x9F\xA5\x91 ;; 1 ; something \n 2");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_COMMENT);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 10L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eql(token.start_pos, 11L);
  assert_eql(token.length, 1L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_COMMENT);
  assert_eql(token.start_pos, 13L);
  assert_eql(token.length, 13L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eql(token.start_pos, 27L);
  assert_eql(token.length, 1L);
  avoc_source_free(&src);

  load_string(&src, "\"\xF0\x9F\xA5\x91\"");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_STR);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 6L);
  avoc_source_free(&src);

  load_string(&src, "\"\xF0\x9F\xA5\x91 avocado\" \"a\"");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_STR);
  assert_eql(token.start_pos, 0L);
  assert_eql(token.length, 14L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  assert_eq(token.type, TOKEN_LIT);
  assert_eq(token.lit_type, LIT_STR);
  assert_eql(token.start_pos, 15L);
  assert_eql(token.length, 3L);
  avoc_source_free(&src);
}

void test_lists() {
  avoc_item item1, item2, item3, item4;
  avoc_list list1, list2;

  avoc_item_init(&item1);
  assert_eq(item1.type, 0);
  assert_eql(item1.as_u64, 0UL);
  assert_eql(item1.token_pos, 0L);
  assert_eql(item1.token_length, 0L);
  assert_ok(item1.next_sibling == NULL);
  assert_ok(item1.prev_sibling == NULL);

  avoc_list_init(&list1);
  assert_ok(list1.head == NULL);
  assert_ok(list1.tail == NULL);
  assert_eql(list1.item_count, 0L);

  avoc_list_push(&list1, &item1);
  assert_eql(list1.item_count, 1L);
  assert_okb(list1.head == &item1);
  assert_okb(list1.tail == &item1);
  assert_okb(list1.head->next_sibling == NULL);
  assert_okb(list1.tail->next_sibling == NULL);

  avoc_item_init(&item2);
  avoc_list_push(&list1, &item2);
  assert_eql(list1.item_count, 2L);
  assert_okb(list1.head == &item1);
  assert_okb(list1.tail == &item2);
  assert_okb(list1.head->next_sibling == &item2);
  assert_okb(list1.tail->prev_sibling == &item1);
  assert_okb(list1.tail->next_sibling == NULL);

  avoc_item_init(&item3);
  avoc_item_init(&item4);
  avoc_list_init(&list2);
  avoc_list_push(&list2, &item3);
  avoc_list_push(&list2, &item4);
  avoc_list_merge(&list1, &list2);
  assert_okb(list1.tail == &item4);
  assert_okb(list1.tail->prev_sibling == &item3);
  assert_okb(item2.next_sibling == &item3);
  assert_okb(item3.prev_sibling == &item2);
}

void test_parse_bol_lit() {
  avoc_source src;
  avoc_token token;
  avoc_item item;
  avoc_status status;

  load_string(&src, "true false");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_BOL);
  assert_eq(item.as_bol, 1);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_BOL);
  assert_eq(item.as_bol, 0);
  avoc_source_free(&src);
}

void test_parse_int_lit() {
  avoc_source src;
  avoc_token token;
  avoc_item item;
  avoc_status status;

  load_string(&src, "0 1i32 2i64 3u32 4u64");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_I32);
  assert_eq(item.as_i32, 0);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_I32);
  assert_eq(item.as_i32, 1);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_I64);
  assert_eql(item.as_i64, 2L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_U32);
  assert_eq(item.as_u32, 3);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_U64);
  assert_eql(item.as_u64, 4UL);
  avoc_source_free(&src);

  load_string(&src, "0b11 0o77 0xFF 0b0i32 0o77i64 0xFFu32");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_I32);
  assert_eq(item.as_i32, 3);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_I32);
  assert_eq(item.as_i32, 077);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_I32);
  assert_eq(item.as_i32, 0xFF);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_I32);
  assert_eq(item.as_i32, 0);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_I64);
  assert_eql(item.as_i64, 077L);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_U32);
  assert_eq(item.as_u32, 0xFF);
  avoc_source_free(&src);

  load_string(&src, "-1 -2i32 -0x1 -0x1i64");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_I32);
  assert_eq(item.as_i32, -1);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_I32);
  assert_eq(item.as_i32, -2);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_I32);
  assert_eq(item.as_i32, -0x1);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_I64);
  assert_eql(item.as_i64, -0x1L);
  avoc_source_free(&src);
}

void test_parse_flt_lit() {
  avoc_source src;
  avoc_token token;
  avoc_item item;
  avoc_status status;

  load_string(&src, "0.1 .2 3.4f32 .5f64");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_F32);
  assert_eqf(item.as_f32, 0.1f);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_F32);
  assert_eqf(item.as_f32, 0.2f);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_F32);
  assert_eqf(item.as_f32, 3.4f);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_F64);
  assert_eqf(item.as_f64, 0.5f);
  avoc_source_free(&src);

  load_string(&src, "-0.1 -.2 -3.4f32 -.5f64");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_F32);
  assert_eqf(item.as_f32, -0.1f);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_F32);
  assert_eqf(item.as_f32, -0.2f);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_F32);
  assert_eqf(item.as_f32, -3.4f);

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_F64);
  assert_eqf(item.as_f64, -0.5f);
  avoc_source_free(&src);
}

void test_parse_str_lit() {
  avoc_source src;
  avoc_token token;
  avoc_item item;
  avoc_status status;

  load_string(&src, "'str1' \"str2\" `str3`");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_STR);
  assert_eqs(item.as_str,"str1");

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_STR);
  assert_eqs(item.as_str,"str2");

  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_STR);
  assert_eqs(item.as_str,"str3");
  avoc_source_free(&src);

  load_string(&src, "'c\\\\c'");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_STR);
  assert_eqs(item.as_str, "c\\c");
  avoc_source_free(&src);

  load_string(&src, "'c\\x0Ac'");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_STR);
  assert_eqs(item.as_str, "c\nc");
  avoc_source_free(&src);

  load_string(&src, "'x \\u00A1 x'");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_STR);
  assert_eqs(item.as_str, "x \xC2\xA1 x");
  avoc_source_free(&src);

  load_string(&src, "'x \\U0001F60A x'");
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_lit(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_STR);
  assert_eqs(item.as_str, "x \xF0\x9F\x98\x8A x");
  avoc_source_free(&src);
}

void test_parse_sym_no_type() {
  avoc_source src;
  avoc_token token;
  avoc_item item;
  avoc_status status;

  load_string(&src, "sym1 SYM2 $sym3 \xF0\x9F\xA5\x91");
  avoc_item_init(&item);
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_sym(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_SYM);
  assert_eqs(item.as_str,"sym1");

  avoc_item_init(&item);
  status = avoc_parse_sym(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_SYM);
  assert_eqs(item.as_str,"SYM2");

  avoc_item_init(&item);
  status = avoc_parse_sym(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_SYM);
  assert_eqs(item.as_str,"$sym3");

  avoc_item_init(&item);
  status = avoc_parse_sym(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_SYM);
  assert_eqs(item.as_str,"\xF0\x9F\xA5\x91");
  avoc_source_free(&src);
}

void test_parse_sym_with_ordinary_type() {
  avoc_source src;
  avoc_token token;
  avoc_item item;
  avoc_status status;

  load_string(&src, "sym1:type1 SYM2:TYPE2 $sym3:$type3 \xF0\x9F\xA5\x91:\xF0\x9F\xA5\x91");
  avoc_item_init(&item);
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_sym(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_SYM);
  assert_eqs(item.as_str,"sym1");
  assert_eqs(item.sym_ordinary_type,"type1");

  avoc_item_init(&item);
  status = avoc_parse_sym(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_SYM);
  assert_eqs(item.as_str,"SYM2");
  assert_eqs(item.sym_ordinary_type,"TYPE2");

  avoc_item_init(&item);
  status = avoc_parse_sym(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_SYM);
  assert_eqs(item.as_str,"$sym3");
  assert_eqs(item.sym_ordinary_type,"$type3");

  avoc_item_init(&item);
  status = avoc_parse_sym(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_SYM);
  assert_eqs(item.as_str,"\xF0\x9F\xA5\x91");
  assert_eqs(item.sym_ordinary_type,"\xF0\x9F\xA5\x91");
  avoc_source_free(&src);
}

void test_parse_comment() {
  avoc_source src;
  avoc_token token;
  avoc_item item;
  avoc_status status;

  load_string(&src, ";; this is a comment ;; ;; another comment ;;");
  avoc_item_init(&item);
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_comment(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_COMMENT);
  assert_eqs(item.as_str,";; this is a comment ;;");

  avoc_item_init(&item);
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_comment(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_COMMENT);
  assert_eqs(item.as_str,";; another comment ;;");
  avoc_source_free(&src);
}

void test_parse_item_no_lists() {
  avoc_source src;
  avoc_token token;
  avoc_item item;
  avoc_status status;

  load_string(&src, "a '1' 2 3.0 ;; 4 ;; %5:%6 7i64");
  avoc_item_init(&item);
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_item(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_SYM);
  assert_eqs(item.as_sym, "a");

  status = avoc_parse_item(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_STR);
  assert_eqs(item.as_str, "1");

  status = avoc_parse_item(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_I32);
  assert_eq(item.as_i32, 2);

  status = avoc_parse_item(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_F32);
  assert_eqf(item.as_f32, 3.0f);

  status = avoc_parse_item(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_COMMENT);
  assert_eqs(item.as_str, ";; 4 ;;");

  status = avoc_parse_item(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_SYM);
  assert_eqs(item.as_sym, "%5");
  assert_eqs(item.sym_ordinary_type, "%6");

  status = avoc_parse_item(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_LIT_I64);
  assert_eql(item.as_i64, 7L);
  avoc_source_free(&src);
}

void test_parse_lists() {
  avoc_source src;
  avoc_token token;
  avoc_list list;
  avoc_status status;

  load_string(&src, "()");
  avoc_list_init(&list);
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_list(&src, &token, &list);
  assert_okb(status == OK);
  assert_okb(list.head == NULL);
  assert_okb(list.tail == NULL);

  load_string(&src, "(first second)");
  avoc_list_init(&list);
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_list(&src, &token, &list);
  assert_okb(status == OK);
  assert_okb(list.head != NULL);
  assert_eq(list.head->type, ITEM_SYM);
  assert_eqs(list.head->as_sym, "first");

  assert_okb(list.tail != NULL);
  assert_eq(list.tail->type, ITEM_SYM);
  assert_eqs(list.tail->as_sym, "second");
  avoc_list_free(&list);

  load_string(&src, "(first \n second)");
  avoc_list_init(&list);
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_list(&src, &token, &list);
  assert_okb(status == OK);
  assert_okb(list.head != NULL);
  assert_eq(list.head->type, ITEM_SYM);
  assert_eqs(list.head->as_sym, "first");

  assert_okb(list.tail != NULL);
  assert_eq(list.tail->type, ITEM_SYM);
  assert_eqs(list.tail->as_sym, "second");
  avoc_list_free(&list);
  avoc_source_free(&src);

  load_string(&src, "(\n first \n second \n)");
  avoc_list_init(&list);
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_list(&src, &token, &list);
  assert_okb(status == OK);
  assert_okb(list.head != NULL);
  assert_eq(list.head->type, ITEM_SYM);
  assert_eqs(list.head->as_sym, "first");

  assert_okb(list.tail != NULL);
  assert_eq(list.tail->type, ITEM_SYM);
  assert_eqs(list.tail->as_sym, "second");
  avoc_list_free(&list);
  avoc_source_free(&src);

  load_string(&src, "(first second:third)");
  avoc_list_init(&list);
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_list(&src, &token, &list);
  assert_okb(status == OK);
  assert_okb(list.head != NULL);
  assert_eq(list.head->type, ITEM_SYM);
  assert_eqs(list.head->as_sym, "first");

  assert_okb(list.tail != NULL);
  assert_eq(list.tail->type, ITEM_SYM);
  assert_eqs(list.tail->as_sym, "second");
  assert_okb(list.tail->sym_ordinary_type != NULL);
  assert_eqs(list.tail->sym_ordinary_type, "third");
  avoc_list_free(&list);

  load_string(&src, "(num 123)");
  avoc_list_init(&list);
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_list(&src, &token, &list);
  assert_okb(status == OK);
  assert_okb(list.head != NULL);
  assert_eq(list.head->type, ITEM_SYM);
  assert_eqs(list.head->as_sym, "num");

  assert_okb(list.tail != NULL);
  assert_eq(list.tail->type, ITEM_LIT_I32);
  assert_eq(list.tail->as_i32, 123);
  avoc_list_free(&list);
  avoc_source_free(&src);

  load_string(&src, "(str 'string')");
  avoc_list_init(&list);
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_list(&src, &token, &list);
  assert_okb(status == OK);
  assert_okb(list.head != NULL);
  assert_eq(list.head->type, ITEM_SYM);
  assert_eqs(list.head->as_sym, "str");

  assert_okb(list.tail != NULL);
  assert_eq(list.tail->type, ITEM_LIT_STR);
  assert_eqs(list.tail->as_str, "string");
  avoc_list_free(&list);
  avoc_source_free(&src);

  load_string(&src, "(parent [child])");
  avoc_list_init(&list);
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_list(&src, &token, &list);
  assert_okb(status == OK);
  assert_okb(list.head != NULL);
  assert_eq(list.head->type, ITEM_SYM);
  assert_eqs(list.head->as_sym, "parent");

  assert_okb(list.tail != NULL);
  assert_okb(list.tail->type == ITEM_LIST);
  assert_okb(list.tail->as_list->head->type == ITEM_SYM);
  assert_eqs(list.tail->as_list->head->as_sym, "child");
  avoc_list_free(&list);
  avoc_source_free(&src);
}

void test_parse_sym_with_composed_type() {
  avoc_source src;
  avoc_token token;
  avoc_item item;
  avoc_status status;
  avoc_list *type;

  load_string(&src, "sym:(composed type)");
  avoc_item_init(&item);
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_item(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_SYM);
  assert_eqs(item.as_sym, "sym");

  assert_okb(item.sym_composed_type != NULL);
  type = item.sym_composed_type;

  assert_okb(type->head != NULL);
  assert_eq(type->head->type, ITEM_SYM);
  assert_eqs(type->head->as_sym, "composed");

  assert_okb(type->tail != NULL);
  assert_eq(type->tail->type, ITEM_SYM);
  assert_eqs(type->tail->as_sym, "type");
  avoc_source_free(&src);

  load_string(&src, "sym:(composed (type))");
  avoc_item_init(&item);
  status = avoc_next_token(&src, &token);
  assert_okb(status == OK);
  status = avoc_parse_item(&src, &token, &item);
  assert_okb(status == OK);
  assert_eq(item.type, ITEM_SYM);
  assert_eqs(item.as_sym, "sym");

  assert_okb(item.sym_composed_type != NULL);
  type = item.sym_composed_type;

  assert_okb(type->head != NULL);
  assert_eq(type->head->type, ITEM_SYM);
  assert_eqs(type->head->as_sym, "composed");

  assert_okb(type->tail != NULL);
  assert_eq(type->tail->type, ITEM_LIST);
  assert_okb(type->tail->as_list->head->type == ITEM_SYM);
  assert_eqs(type->tail->as_list->head->as_sym, "type");
  avoc_source_free(&src);
}

void test_parse_source() {
  avoc_source src;
  avoc_list list;
  avoc_status status;

  load_string(&src, "");
  avoc_list_init(&list);
  status = avoc_parse_source(&src, &list);
  assert_okb(status == OK);
  assert_okb(list.head == NULL);
  assert_okb(list.tail == NULL);
  avoc_list_free(&list);
  avoc_source_free(&src);

  load_string(&src, "(1) (2)");
  avoc_list_init(&list);
  status = avoc_parse_source(&src, &list);
  assert_okb(status == OK);
  assert_okb(list.head->type == ITEM_LIST);
  assert_okb(list.head->as_list->head->type == ITEM_LIT_I32);
  assert_eq(list.head->as_list->head->as_i32, 1);
  avoc_list_free(&list);
  avoc_source_free(&src);

  load_string(&src, "(1) \n (2)");
  avoc_list_init(&list);
  status = avoc_parse_source(&src, &list);
  assert_okb(status == OK);
  assert_okb(list.head->type == ITEM_LIST);
  assert_okb(list.head->as_list->head->type == ITEM_LIT_I32);
  assert_eq(list.head->as_list->head->as_i32, 1);
  avoc_list_free(&list);
  avoc_source_free(&src);
}

int main(){
  trun("test_source_init_free", test_source_init_free);
  trun("test_source_move_fwd_ascii", test_source_move_fwd_ascii);
  trun("test_source_move_fwd_utf8", test_source_move_fwd_utf8);
  trun("test_source_move_fwd_newl", test_source_move_fwd_newl);
  trun("test_token_init", test_token_init);
  trun("test_token_next_singlechar", test_token_next_singlechar);
  trun("test_token_next_comments", test_token_next_comments);
  trun("test_token_next_str_lit", test_token_next_str_lit);
  trun("test_token_next_num_lit", test_token_next_num_lit);
  trun("test_token_next_nilbol_lit", test_token_next_nilbol_lit);
  trun("test_token_next_id", test_token_next_id);
  trun("test_token_edge_cases", test_token_edge_cases);
  trun("test_lists", test_lists);
  trun("test_parse_bol_lit", test_parse_bol_lit);
  trun("test_parse_int_lit", test_parse_int_lit);
  trun("test_parse_flt_lit", test_parse_flt_lit);
  trun("test_parse_str_lit", test_parse_str_lit);
  trun("test_parse_comment", test_parse_comment);
  trun("test_parse_sym_no_type", test_parse_sym_no_type);
  trun("test_parse_sym_with_ordinary_type", test_parse_sym_with_ordinary_type);
  trun("test_parse_item_no_lists", test_parse_item_no_lists);
  trun("test_parse_lists", test_parse_lists);
  trun("test_parse_sym_with_composed_type", test_parse_sym_with_composed_type);
  trun("test_parse_source", test_parse_source);
  tresults();
  return 0;
}
