#ifndef TESTS_H // NOLINT
#define TESTS_H

#include <math.h>   // fabs()
#include <stdio.h>  // printf(), fprintf()
#include <string.h> // strcmp(), strlen()
#include <time.h>   // clock_t

// Track of the number of passes and failures
__attribute__((unused)) static int ttotal = 0;
__attribute__((unused)) static int tfails = 0;

// How far apart can floats be before we consider them unequal
#ifndef LTEST_FLOAT_TOLERANCE
#define LTEST_FLOAT_TOLERANCE 0.001
#endif

// Display results
#define tresults()                                                             \
  do {                                                                         \
    int retval = 0;                                                            \
    if (tfails == 0) {                                                         \
      printf("ALL TESTS PASSED (%d/%d)\n", ttotal, ttotal);                    \
    } else {                                                                   \
      printf("SOME TESTS FAILED (%d/%d)\n", ttotal - tfails, ttotal);          \
      retval = 1;                                                              \
    }                                                                          \
    return retval;                                                             \
  } while (0)

// Run a test
#define trun(name, func)                                                       \
  do {                                                                         \
    const int tt = ttotal;                                                     \
    const int tf = tfails;                                                     \
    const clock_t start = clock();                                             \
    printf("\t%-20s ", name);                                                  \
    func();                                                                    \
    printf("pass:%3d fail:%3d %4dms\n", (ttotal - tt) - (tfails - tf),         \
           tfails - tf, (int)((clock() - start) * 1000 / CLOCKS_PER_SEC));     \
  } while (0)

// Assertions

// Fail if argument is zero
#define assert_ok(check)                                                       \
  do {                                                                         \
    ++ttotal;                                                                  \
    if (!(check)) {                                                            \
      if (tfails == 0) {                                                       \
        printf("\n");                                                          \
      }                                                                        \
      ++tfails;                                                                \
      printf("%s:%d: error \n", __FILE__, __LINE__);                           \
    }                                                                          \
  } while (0)

#define assert_okb(check)                                                      \
  do {                                                                         \
    ++ttotal;                                                                  \
    if (!(check)) {                                                            \
      if (tfails == 0) {                                                       \
        printf("\n");                                                          \
      }                                                                        \
      ++tfails;                                                                \
      printf("%s:%d: error, some assertions may not be executed! \n",          \
             __FILE__, __LINE__);                                              \
      return;                                                                  \
    }                                                                          \
  } while (0)

// Prototype to assert equal
#define tequal_base(equality, a, b, format)                                    \
  do {                                                                         \
    ++ttotal;                                                                  \
    if (!(equality)) {                                                         \
      if (tfails == 0) {                                                       \
        printf("\n");                                                          \
      }                                                                        \
      ++tfails;                                                                \
      printf("%s:%d: (" format " != " format ")\n", __FILE__, __LINE__, (a),   \
             (b));                                                             \
    }                                                                          \
  } while (0)

// Fail if two int arguments are different
#define assert_eq(a, b) tequal_base((a) == (b), a, b, "%d")

// Fail if two long arguments are different
#define assert_eql(a, b) tequal_base((a) == (b), a, b, "%ld")

// Fail if two strings are different
#define assert_eqs(a, b) tequal_base(strcmp(a, b) == 0, a, b, "%s")

// Fail if two float or double arguments are different
#define assert_eqf(a, b)                                                       \
  tequal_base(fabs((double)(a) - (double)(b)) <= LTEST_FLOAT_TOLERANCE &&      \
                  fabs((double)(a) - (double)(b)) ==                           \
                      fabs((double)(a) - (double)(b)),                         \
              (double)(a), (double)(b), "%f")

// Loads a buffer into the specified source
#define load_string(src, s)                                                    \
  do {                                                                         \
    avoc_source_init(src, NULL, s, strlen(s));                                 \
  } while (0)

#endif /* TESTS_H */
