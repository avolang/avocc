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
  assert_eql(src0.col, 0L);
  assert_eql(src0.row, 0L);
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
  assert_eql(src1.col, 0L);
  assert_eql(src1.row, 0L);

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

int main(){
  trun("test_source_init_free", test_source_init_free);
  trun("test_source_move_fwd_ascii", test_source_move_fwd_ascii);
  trun("test_source_move_fwd_utf8", test_source_move_fwd_utf8);
  tresults();
  return 0;
}
