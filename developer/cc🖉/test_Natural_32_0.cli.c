
#include "environment.h"
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>

#define IFACE
#include "Natural_32.lib.c"

#define TEST_COUNT 10  // Adjust as needed

jmp_buf test_env;
const char *current_test = NULL;

void signal_handler(int sig){
  printf("Failed due to Exception: %s (Signal %d)\n", current_test, sig);
  longjmp(test_env, 1);
}

int main(void){
  bool test_results[TEST_COUNT] = {false};
  const char *test_names[TEST_COUNT] = {
    "Addition",
    "Subtraction",
    "Multiplication",
    "Division",
    "Modulus",
    "Shift Left",
    "Shift Right",
    "Comparison",
    "Complement",
    "Two's Complement"
  };

  int pass_count = 0, fail_count = 0;
  bool *test_ptr = test_results;
  const char **name_ptr = test_names;

  // Install signal handler
  signal(SIGFPE, signal_handler);
  signal(SIGSEGV, signal_handler);
  signal(SIGABRT, signal_handler);

  Natural_32 a, b, result, overflow;
  Natural_32·set_to_zero(&a);
  Natural_32·set_to_one(&b);

  // Macro to run tests with proper failure messaging
  #define RUN_TEST(expr) \
    current_test = *name_ptr; \
    if(setjmp(test_env) == 0){ \
      if(expr){ \
        *test_ptr++ = true; \
        pass_count++; \
      } else { \
        printf("Failed due to Bad Return Value: %s\n", *name_ptr); \
        *test_ptr++ = false; \
        fail_count++; \
      } \
    } else { \
      *test_ptr++ = false; \
      fail_count++; \
    } \
    name_ptr++;

  RUN_TEST(Natural_32·add(&result, &a, &b) == Natural_32·Status·ok && result.d0 == 1);
  RUN_TEST(Natural_32·subtract(&result, &b, &a) == Natural_32·Status·ok && result.d0 == 1);
  RUN_TEST(Natural_32·multiply(&overflow, &result, &b, &b) == Natural_32·Status·ok && result.d0 == 1 && overflow.d0 == 0);
  RUN_TEST(Natural_32·divide(&result, &overflow, &b, &b) == Natural_32·Status·ok && result.d0 == 1 && overflow.d0 == 0);
  RUN_TEST(Natural_32·modulus(&result, &b, &b) == Natural_32·Status·ok && result.d0 == 0);
  
  Natural_32·shift_left(1, &overflow, &result, &b);
  RUN_TEST(result.d0 == 2 && overflow.d0 == 0);

  Natural_32·shift_right(1, &b, &result, &overflow);
  RUN_TEST(result.d0 == 0 && overflow.d0 == 1);

  RUN_TEST(Natural_32·compare(&a, &b) == Natural_32·Order·lt);
  
  Natural_32·complement(&result, &a);
  RUN_TEST(result.d0 == ~0);

  Natural_32·twos_complement(&result, &b);
  RUN_TEST(result.d0 == (uint32_t)(-1));

  printf("Pass: %d, Fail: %d\n", pass_count, fail_count);
  return fail_count > 0 ? 1 : 0;
}
