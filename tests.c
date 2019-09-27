#include "tests.h"
#include <stdio.h>

void test_oks() {
  assert_ok(1);
  assert_eq(1, 1);
  assert_eql(1L, 1L);
  assert_eqf(1.0f, 1.0f);
  assert_eqs("hello", "hello");
}

void test_fails() {
  assert_ok(0);
  assert_eq(0, 1);
  assert_eql(0L, 1L);
  assert_eqf(1.1f, 1.0f);
  assert_eqs("olleh", "hello");
}

int main(){
  trun("test_oks", test_oks);
  trun("test_fails", test_fails);
  tresults();
  return 0;
}
