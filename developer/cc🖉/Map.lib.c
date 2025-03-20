/* Map - mapping functions between tape machines.

   This file has two template parameters:
   `CVT_read`  Cell Value Type for the source/read tape machine
   `CVT_write` Cell Value Type for the destination/write tape machine

   By default (when the macros have no definition) the cell value types are taken as AU.
   This file must be included with CVT_read and CVT_write undefined, before inclusions with them defined.

   'Tape' is operated on by the Tape Machine.
   'Area' is subset of an address space that is used as a virtual Tape by a machine.
   An Area with zero elements has 'length == 0' or is 'empty'.
   In contrast, and area located (position specified) with a null pointer is said not to exist.
*/

#define Map·DEBUG

#ifndef FACE
  #define Map·IMPLEMENTATION
  #define FACE
#endif

//--------------------------------------------------------------------------------
// Interface

#ifndef Map·FACE
  #define Map·FACE

  #include <stdint.h>
  #include <stddef.h>
  #include "Core.lib.c"
  #include "TM.lib.c"

  // if only one template parameter is given, we will assume both are the same

  #if defined(CVT_read) && !defined(CVT_write)
    #warning "Given CVT_read template value. Missing CVT_write template value, so setting it to CVT_read"
    #define CVT_write CVT_read
  #endif

  #if !defined(CVT_read) && defined(CVT_write)
    #warning "Given CVT_write template value. Missing CVT_read template value, so setting it to CVT_write"
    #define CVT_read CVT_write
  #endif

  //----------------------------------------
  // Map status and completion enums
  //----------------------------------------

  #if !defined(CVT_read) && !defined(CVT_write)

    typedef enum {
       Map·Completion·mu = 0
      ,Map·Completion·failed          = 1
      ,Map·Completion·null_fn         = 1 << 1
      ,Map·Completion·no_tape_access  = 1 << 2
      ,Map·Completion·rightmost_read  = 1 << 3 
      ,Map·Completion·rightmost_write = 1 << 4 
    } Map·Completion;

    // Masks for combinations
    const uint Map·Completion·rightmost_both =
        Map·Completion·rightmost_read
      | Map·Completion·rightmost_write
      ;

    const uint Map·Completion·derailed =
        Map·Completion·failed
      | Map·Completion·null_fn
      | Map·Completion·no_tape_access
      ;

    const uint Map·Completion·on_track =
        Map·Completion·rightmost_read
      | Map·Completion·rightmost_write
      | Map·Completion·rightmost_both
      ;

  #endif

  //----------------------------------------
  // Map interface
  //----------------------------------------

  #if !defined(CVT_read) || !defined(CVT_write)

    typedef struct {
      Map·Completion (*copy_byte_to_byte)(
        Ξ(TM·Array ,AU) *read_tm
        ,Ξ(TM·Array ,AU) *write_tm
      );

      Map·Completion (*copy_hex_to_byte)(
        Ξ(TM·Array ,uint16_t) *read_tm
        ,Ξ(TM·Array ,AU) *write_tm
      );

      Map·Completion (*copy_byte_to_hex)(
        Ξ(TM·Array ,AU) *read_tm
        ,Ξ(TM·Array ,uint16_t) *write_tm
      );

      // Terminate string function
      Map·Completion (*terminate_string)(
        Ξ(TM·Array ,AU) *write_tm
      );
    } Map·FG;

     // a default function given table
     Map·FG Map·fg;

  #endif


  #if defined(CVT_read) && defined(CVT_write)

    // Function passed to map type signature must be this:
    typedef Core·Status (*Ξ(Map·fn ,CVT_read ,CVT_write))(
      CVT_read *read_value
      ,CVT_write *write_value
    );

    typedef struct {
      // Map a function over all elements from read_tm to write_tm
      Map·Completion (*map)(
        Ξ(TM·Array ,CVT_read) *read_tm
        ,Ξ(TM·Array ,CVT_write) *write_tm
        ,Ξ(Map·fn ,CVT_read ,CVT_write) map_fn
      );

      // Map a function over elements from read_tm to write_tm until a condition is met
      Map·Completion (*map_while)(
        Ξ(TM·Array ,CVT_read) *read_tm
        ,Ξ(TM·Array ,CVT_write) *write_tm
        ,Ξ(Map·fn ,CVT_read ,CVT_write) map_fn
        ,bool (*condition)(Ξ(TM·Array ,CVT_read) *read_tm)
      );

      // Map a function over n elements from read_tm to write_tm
      Map·Completion (*map_extent)(
        Ξ(TM·Array ,CVT_read) *read_tm
        ,Ξ(TM·Array ,CVT_write) *write_tm
        ,Ξ(Map·fn ,CVT_read ,CVT_write) map_fn
        ,Ξ(extent_t ,CVT_read) extent
      );

    } Ξ(Map·FG ,CVT_read ,CVT_write);

    // Default function given table
    Ξ(Map·FG ,CVT_read ,CVT_write) Ξ(Map·fg ,CVT_read ,CVT_write);

  #endif

#endif // #ifndef Map·FACE
