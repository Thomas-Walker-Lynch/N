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
  N32·T *array = N32·m.allocate_array(2 ,NULL);
  if( !array ) return false;

  // Access elements via access function
  N32·T *a = N32·m.access(array ,0);
  N32·T *b = N32·m.access(array ,1);

  // Assign value and copy
  N32·m.from_uint32(a ,42);
  N32·m.copy(b ,a);

  bool success = ( N32·m.compare(b ,a) == N32·Order_eq );
  N32·m.deallocate(array);
  return success;
}

bool test_arithmetic(){
  // Allocate memory
  N32·T *array = N32·m.allocate_array(3 ,NULL);
  if( !array ) return false;

  N32·T *a = N32·m.access(array ,0);
  N32·T *b = N32·m.access(array ,1);
  N32·T *result = N32·m.access(array ,2);

  N32·m.from_uint32(a ,20);
  N32·m.from_uint32(b ,22);

  if( N32·m.add(result ,a ,b) != N32·Status·ok ) return false;
  if( N32·m.compare(result ,N32·m.access(array ,0)) != N32·Order_gt ) return false;

  if( N32·m.subtract(result ,b ,a) != N32·Status·ok ) return false;
  if( N32·m.compare(result ,N32·m.access(array ,0)) != N32·Order_lt ) return false;

  N32·m.deallocate(array);
  return true;
}

bool test_bitwise_operations(){
  // Allocate memory
  N32·T *array = N32·m.allocate_array(3, NULL);
  if(!array) return false;

  N32·T *a = N32·m.access(array, 0);
  N32·T *b = N32·m.access(array, 1);
  N32·T *result = N32·m.access(array, 2);

  // a = 0x0F0F0F0F, b = 0xF0F0F0F0
  N32·m.from_uint32(a, 0x0F0F0F0F);
  N32·m.from_uint32(b, 0xF0F0F0F0);

  // bit_and => expect 0x00000000
  N32·m.bit_and(result, a, b);
  N32·m.from_uint32(a, 0x00000000);
  if(N32·m.compare(result, a) != N32·Order_eq){
    N32·m.deallocate(array);
    return false;
  }

  // Reset a to 0x0F0F0F0F for next tests
  N32·m.from_uint32(a, 0x0F0F0F0F);

  // bit_or => expect 0xFFFFFFFF
  N32·m.bit_or(result, a, b);
  N32·m.from_uint32(b, 0xFFFFFFFF);
  if(N32·m.compare(result, b) != N32·Order_eq){
    N32·m.deallocate(array);
    return false;
  }

  // bit_complement(a=0x0F0F0F0F) => expect 0xF0F0F0F0
  N32·m.from_uint32(a, 0x0F0F0F0F);
  N32·m.bit_complement(result, a);
  N32·m.from_uint32(b, 0xF0F0F0F0);
  if(N32·m.compare(result, b) != N32·Order_eq){
    N32·m.deallocate(array);
    return false;
  }

  // bit_twos_complement(a=0x0F0F0F0F) => expect 0xF0F0F0F1
  N32·m.from_uint32(a, 0x0F0F0F0F);
  N32·m.bit_twos_complement(result, a);
  N32·m.from_uint32(b, 0xF0F0F0F1);
  if(N32·m.compare(result, b) != N32·Order_eq){
    N32·m.deallocate(array);
    return false;
  }

  N32·m.deallocate(array);
  return true;
}

bool test_comparisons(){
  // Allocate memory
  N32·T *array = N32·m.allocate_array(3, NULL);
  if(!array) return false;

  N32·T *a = N32·m.access(array, 0);
  N32·T *b = N32·m.access(array, 1);
  N32·T *c = N32·m.access(array, 2);

  // First set: a=0, b=42, c=42
  N32·m.from_uint32(a, 0);
  N32·m.from_uint32(b, 42);
  N32·m.from_uint32(c, 42);

  // eq_zero(a) => true
  if(!N32·m.eq_zero(a)){
    N32·m.deallocate(array);
    return false;
  }
  // eq_zero(b) => false
  if(N32·m.eq_zero(b)){
    N32·m.deallocate(array);
    return false;
  }
  // eq(b, c) => true
  if(!N32·m.eq(b, c)){
    N32·m.deallocate(array);
    return false;
  }
  // eq(a, b) => false
  if(N32·m.eq(a, b)){
    N32·m.deallocate(array);
    return false;
  }
  // compare(a, b) => N32·Order_lt
  if(N32·m.compare(a, b) != N32·Order_lt){
    N32·m.deallocate(array);
    return false;
  }
  // compare(b, a) => N32·Order_gt
  if(N32·m.compare(b, a) != N32·Order_gt){
    N32·m.deallocate(array);
    return false;
  }
  // compare(b, c) => N32·Order_eq
  if(N32·m.compare(b, c) != N32·Order_eq){
    N32·m.deallocate(array);
    return false;
  }
  // lt(a, b) => true, gt(b, a) => true
  if(!N32·m.lt(a, b) || !N32·m.gt(b, a)){
    N32·m.deallocate(array);
    return false;
  }

  // Second set: a=100, b=50
  N32·m.from_uint32(a, 100);
  N32·m.from_uint32(b, 50);
  if(N32·m.compare(a, b) != N32·Order_gt){
    N32·m.deallocate(array);
    return false;
  }
  // eq_zero(a) => false
  if(N32·m.eq_zero(a)){
    N32·m.deallocate(array);
    return false;
  }
  // eq_zero(b) => false
  if(N32·m.eq_zero(b)){
    N32·m.deallocate(array);
    return false;
  }

  N32·m.deallocate(array);
  return true;
}

bool test_shifts(){
  // Allocate memory for operand, fill, spill
  N32·T *array = N32·m.allocate_array(3, NULL);
  if(!array) return false;

  N32·T *operand = N32·m.access(array, 0);
  N32·T *fill    = N32·m.access(array, 1);
  N32·T *spill   = N32·m.access(array, 2);

  // Subtest A: shift_left(4) with operand=1 => expect operand=16, fill=0, spill=0
  N32·m.from_uint32(operand, 1);
  N32·m.from_uint32(fill, 0);
  N32·m.from_uint32(spill, 0);
  if(N32·m.shift_left(4, spill, operand, fill) != N32·Status·ok){
    N32·m.deallocate(array);
    return false;
  }
  N32·T *temp = N32·m.allocate_array(1, NULL);
  if(!temp){
    N32·m.deallocate(array);
    return false;
  }
  N32·m.from_uint32(temp, 16);
  if(N32·m.compare(operand, temp) != N32·Order_eq){
    N32·m.deallocate(temp);
    N32·m.deallocate(array);
    return false;
  }
  if(N32·m.compare(fill, N32·zero) != N32·Order_eq){
    N32·m.deallocate(temp);
    N32·m.deallocate(array);
    return false;
  }
  if(N32·m.compare(spill, N32·zero) != N32·Order_eq){
    N32·m.deallocate(temp);
    N32·m.deallocate(array);
    return false;
  }

  // Subtest B: shift_left(1) with operand=0x80000000 => expect operand=0, spill=1
  N32·m.from_uint32(operand, 0x80000000);
  N32·m.from_uint32(fill, 0);
  N32·m.from_uint32(spill, 0);
  if(N32·m.shift_left(1, spill, operand, fill) != N32·Status·ok){
    N32·m.deallocate(temp);
    N32·m.deallocate(array);
    return false;
  }
  if(!N32·m.eq_zero(operand)){
    N32·m.deallocate(temp);
    N32·m.deallocate(array);
    return false;
  }
  N32·m.from_uint32(temp, 1);
  if(N32·m.compare(spill, temp) != N32·Order_eq){
    N32·m.deallocate(temp);
    N32·m.deallocate(array);
    return false;
  }

  // Subtest C: shift_right(1) with operand=0x80000000 => expect operand=0x40000000, spill=0
  N32·m.from_uint32(operand, 0x80000000);
  N32·m.from_uint32(fill, 0);
  N32·m.from_uint32(spill, 0);
  if(N32·m.shift_right(1, spill, operand, fill) != N32·Status·ok){
    N32·m.deallocate(temp);
    N32·m.deallocate(array);
    return false;
  }
  N32·m.from_uint32(temp, 0x40000000);
  if(N32·m.compare(operand, temp) != N32·Order_eq){
    N32·m.deallocate(temp);
    N32·m.deallocate(array);
    return false;
  }
  if(!N32·m.eq_zero(spill)){
    N32·m.deallocate(temp);
    N32·m.deallocate(array);
    return false;
  }

  // Subtest D: arithmetic_shift_right(1) with operand=0x80000000 => expect operand=0xC0000000, spill=0
  N32·m.from_uint32(operand, 0x80000000);
  N32·m.from_uint32(spill, 0);
  if(N32·m.arithmetic_shift_right(1, operand, spill) != N32·Status·ok){
    N32·m.deallocate(temp);
    N32·m.deallocate(array);
    return false;
  }
  N32·m.from_uint32(temp, 0xC0000000);
  if(N32·m.compare(operand, temp) != N32·Order_eq){
    N32·m.deallocate(temp);
    N32·m.deallocate(array);
    return false;
  }
  if(!N32·m.eq_zero(spill)){
    N32·m.deallocate(temp);
    N32·m.deallocate(array);
    return false;
  }

  N32·m.deallocate(temp);
  N32·m.deallocate(array);
  return true;
}



// Include the local section of N32.lib.c for testing
#define LOCAL
#include "N32.lib.c"
#undef LOCAL
