#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <setjmp.h>

// Enable interface section
#define FACE
#include "N32.lib.c"
#undef FACE

// Jump buffer for signal handling
static sigjmp_buf jump_buffer;

// Signal handler for catching fatal errors
void signal_handler(int signal){
  siglongjmp(jump_buffer ,1); // Jump back to test_head on error
}

// Test function prototypes
bool test_copy();
bool test_bitwise_operations();
bool test_comparisons();
bool test_arithmetic();
bool test_shifts();

// Test array (null-terminated)
typedef bool (*TestFunction)();
typedef struct{
  TestFunction function;
  const char *name;
}TestEntry;

TestEntry test_list[] = {
   {test_copy ,"test_copy"}
  ,{test_bitwise_operations ,"test_bitwise_operations"}
  ,{test_comparisons ,"test_comparisons"}
  ,{test_arithmetic ,"test_arithmetic"}
  ,{test_shifts ,"test_shifts"}
  ,{NULL ,NULL}  // Null termination
};

// The test runner
int test_head(){
  int pass_count = 0;
  int fail_count = 0;

  // Set up signal handlers
  signal(SIGSEGV ,signal_handler);  // Catch segmentation faults
  signal(SIGFPE ,signal_handler);   // Catch floating point errors
  signal(SIGABRT ,signal_handler);  // Catch abort() calls

  for(TestEntry *entry = test_list; entry->function != NULL; entry++){
    if( sigsetjmp(jump_buffer ,1) == 0 ){
      // Run the test normally
      if( !entry->function() ){
        printf("Failed: %s\n" ,entry->name);
        fail_count++;
      }else{
        pass_count++;
      }
    }else{
      // If a signal was caught
      printf("Failed due to signaling: %s\n" ,entry->name);
      fail_count++;
    }
  }

  printf("Tests passed: %d\n" ,pass_count);
  printf("Tests failed: %d\n" ,fail_count);
  return (fail_count == 0) ? 0 : 1;
}

// Main function
int main(int argc ,char **argv){
  return test_head();
}

//------------------------------------------------------------------------------
// Test Implementations
//------------------------------------------------------------------------------

bool test_copy(){
  // Allocate memory
  N32·T *array = N32·λ.allocate_array(2 ,NULL);
  if( !array ) return false;

  // Access elements via access function
  N32·T *a = N32·λ.access(array ,0);
  N32·T *b = N32·λ.access(array ,1);

  // Assign value and copy
  N32·λ.from_uint32(a ,42);
  N32·λ.copy(b ,a);

  bool success = ( N32·λ.compare(b ,a) == N32·Order_eq );
  N32·λ.deallocate(array);
  return success;
}

bool test_arithmetic(){
  // Allocate memory
  N32·T *array = N32·λ.allocate_array(3 ,NULL);
  if( !array ) return false;

  N32·T *a = N32·λ.access(array ,0);
  N32·T *b = N32·λ.access(array ,1);
  N32·T *result = N32·λ.access(array ,2);

  N32·λ.from_uint32(a ,20);
  N32·λ.from_uint32(b ,22);

  if( N32·λ.add(result ,a ,b) != N32·Status·ok ) return false;
  if( N32·λ.compare(result ,N32·λ.access(array ,0)) != N32·Order_gt ) return false;

  if( N32·λ.subtract(result ,b ,a) != N32·Status·ok ) return false;
  if( N32·λ.compare(result ,N32·λ.access(array ,0)) != N32·Order_lt ) return false;

  N32·λ.deallocate(array);
  return true;
}

// Include the local section of N32.lib.c for testing
#define LOCAL
#include "N32.lib.c"
#undef LOCAL
