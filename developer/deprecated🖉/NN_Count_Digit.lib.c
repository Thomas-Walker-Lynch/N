/*
  A digit count followed by that many digits.
  
  Extent_Digit_Instance:
  - 'NN' stands for Natural Number representation.
  - 'Extent' refers to the maximum array index.
  - 'Digit' specifies that the representation involves digits.
  - 'Instance' differentiates this from the interface struct, ensuring clarity in alternative representations.
*/

#ifndef IFACE
#define NN_Extent_Digit·IMPLEMENTATION
#define IFACE
#endif

#ifndef NN_Extent_Digit·IFACE
#define NN_Extent_Digit·IFACE

  typedef uint32_t Extent;

  // interface function signatures
  //
  typedef NN_Extent_Digit *(*NN_Extent_Digit·Copy·MemoryFault) 
    (
      Extent extent
    );

  typedef NN_Extent_Digit *(*NN_Extent_Digit·Copy) 
    (
      NN_Extent_Digit *original
      ,NN_Extent_Digit·Copy·MemoryFault memory_fault
    );

  typedef void (*NN_Extent_Digit·Accumulate)
    (
      NN_Extent_Digit *accumulator ,...
    );

  typedef NN_Extent_Digit *(*NN_Extent_Digit·Add)
    (
      NN_Extent_Digit *summand ,...
    );

  typedef NN_Extent_Digit *(*NN_Extent_Digit·Multiply)
    (
      NN_Extent_Digit *factor ,...
    );

  typedef NN_Extent_Digit *(*NN_Extent_Digit·Rotate_Right_Digit)
    (
      Extent count
      ,NN_Extent_Digit *a
      ,NN_Extent_Digit *b
      ,NN_Extent_Digit *c
    );

  typedef NN_Extent_Digit *(*NN_Extent_Digit·Rotate_Left_Digit)
    (
      Extent count
      ,NN_Extent_Digit *a
      ,NN_Extent_Digit *b
      ,NN_Extent_Digit *c
    );

  typedef NN_Extent_Digit *(*NN_Extent_Digit·Allocate·MemoryFault)
    (
      Extent extent
    );

  // interface struct definition
  //
  typedef struct {
    NN_Extent_Digit·Copy copy;
    NN_Extent_Digit·Accumulate accumulate;
    NN_Extent_Digit·Add add;
    NN_Extent_Digit·Multiply multiply;
    NN_Extent_Digit·Rotate_Right_Digit rotate_right_digit;
    NN_Extent_Digit·Rotate_Left_Digit rotate_left_digit;
  } NN_Extent_Digit;

  // an extent is a maximum array index
  NN_Extent_Digit *NN_Extent_Digit·allocate(Extent extent, NN_Extent_Digit·Allocate·MemoryFault memory_fault);
  void NN_Extent_Digit·deallocate(NN_Extent_Digit *unencumbered);

#endif

#ifdef NN_Extent_Digit·IMPLEMENTATION

  #include <stdarg.h>
  #include <stdlib.h>
  typedef uint32_t Digit;

  typedef struct {
    Extent extent;
    Digit a[];
  } Instance;

  NN_Extent_Digit *allocate(Extent extent, NN_Extent_Digit·Allocate·MemoryFault memory_fault){
    Extent allocation_size = sizeof(NN_Extent_Digit_Instance) + extent * sizeof(Digit) + sizeof(Digit);
    Instance *instance = malloc(allocation_size);
    if (!instance) {
      return memory_fault ? memory_fault(extent) : NULL;
    }
    instance->extent = extent;

    //  nope->  need to allocate an interface signature struct and assign the method
    //  function pointers to it.  It will also need a field for holding the instance,
    //  yes, declare a new interface struct that has an extra field on the bottom for
    //  the instance pointer .. but then what of interface inheritance?  hmmm. Perhaps
    //  that is better done by composition anyway.
    return (NN_Extent_Digit *)instance;
  }

  void deallocate(NN_Extent_Digit *unencumbered){
    free(unencumbered);
  }

#endif
