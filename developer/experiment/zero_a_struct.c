#include <stdio.h>
#include <stdint.h>

// works:
union C;

// works fine:
/* 
union B {
  union C *pt;
  uint64_t x;
  uint8_t zero[ sizeof(uint64_t) ]; // or is one of the pointers bigger?
};
*/


union B;

union B {
  union C *pt;
  uint64_t x;
  uint8_t zero[ sizeof(B) ];  
};


int main() {
  const union B b = {
    .zero = { 0 }  // Zero-initialize the entire structure
  };

  printf("b.x: %llu\n", (unsigned long long)b.x);
  return 0;
}

/*
Experiment for zeroing out a union.

> gcc zero_a_struct.c 
zero_a_struct.c:22:24: error: ‘B’ undeclared here (not in a function)
   22 |   uint8_t zero[ sizeof(B) ];
      |                        ^

I suppose, as it can not know how big B is until knowing how big the
array will be, wish it recognized this pattern.
*/
