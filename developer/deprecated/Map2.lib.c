




















































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
        ·(TM·Array ,AU) *read_tm
        ,·(TM·Array ,AU) *write_tm
      );

      Map·Completion (*copy_hex_to_byte)(
        ·(TM·Array ,uint16_t) *read_tm
        ,·(TM·Array ,AU) *write_tm
      );

      Map·Completion (*copy_byte_to_hex)(
        ·(TM·Array ,AU) *read_tm
        ,·(TM·Array ,uint16_t) *write_tm
      );

      // Terminate string function
      Map·Completion (*terminate_string)(
        ·(TM·Array ,AU) *write_tm
      );
    }
  #endif


  #if defined(CVT_read) && defined(CVT_write)

    // Function passed to map type signature must be this:
    typedef Core·Status (*·(Map·fn ,CVT_read ,CVT_write))(
      CVT_read *read_value
      ,CVT_write *write_value
    );

    typedef struct {
      // Map a function over all elements from read_tm to write_tm
      Map·Completion (*map)(
        ·(TM·Array ,CVT_read) *read_tm
        ,·(TM·Array ,CVT_write) *write_tm
        ,·(Map·fn ,CVT_read ,CVT_write) map_fn
      );

      // Map a function over elements from read_tm to write_tm until a condition is met
      Map·Completion (*map_while)(
        ·(TM·Array ,CVT_read) *read_tm
        ,·(TM·Array ,CVT_write) *write_tm
        ,·(Map·fn ,CVT_read ,CVT_write) map_fn
        ,bool (*condition)(·(TM·Array ,CVT_read) *read_tm)
      );

      // Map a function over n elements from read_tm to write_tm
      Map·Completion (*map_extent)(
        ·(TM·Array ,CVT_read) *read_tm
        ,·(TM·Array ,CVT_write) *write_tm
        ,·(Map·fn ,CVT_read ,CVT_write) map_fn
        ,·(extent_t ,CVT_read) extent
      );

    } ·(Map·FG ,CVT_read ,CVT_write);

    // Default function given table
    ·(Map·FG ,CVT_read ,CVT_write) ·(Map·fg ,CVT_read ,CVT_write);
  #endif // !defined(CVT_read) && !defined(CVT_write)

#endif // #ifndef Map·FACE

//--------------------------------------------------------------------------------
// Implementation

#ifdef Map·IMPLEMENTATION

  // declarations available to all of the IMPLEMENTATION go here
  #ifdef Map·DEBUG
    #include <stdio.h>
  #endif

  // implementation to go into the lib.a file
  #ifndef LOCAL
  #endif

  #ifdef LOCAL
    //----------------------------------------
    // Map implementation
    //----------------------------------------

    #if !defined(CVT_read) && !defined(CVT_write)
      //-----------------------------------
      // common error messages
      const char *Map·Msg·read_tm = "given NULL read_tm";
      const char *Map·Msg·write_tm = "given NULL write_tm";
      const char *Map·Msg·map_fn = "given NULL map_fn";
      const char *Map·Msg·condition = "given NULL condition function";

      //-----------------------------------
      // Helper functions for copy operations
      
      // Byte to byte copy function
      Local Core·Status Map·byte_to_byte_fn(AU *read_value, AU *write_value) {
        *write_value = *read_value;
        return Core·Status·on_track;
      }
      
      // Hex to byte conversion function
      Local Core·Status Map·hex_to_byte_fn(uint16_t *read_value, AU *write_value) {
        // Convert hex value to byte
        *write_value = (AU)(*read_value & 0xFF);
        return Core·Status·on_track;
      }
      
      // Byte to hex conversion function
      Local Core·Status Map·byte_to_hex_fn(AU *read_value, uint16_t *write_value) {
        // Convert byte to lowercase hex representation (no prefix)
        static const char hex_chars[] = "0123456789abcdef";
        *write_value = hex_chars[(*read_value >> 4) & 0x0F];
        return Core·Status·on_track;
      }
    #endif // !defined(CVT_read) && !defined(CVT_write)

    #if defined(CVT_read) && defined(CVT_write)
      //-----------------------------------
      // Map implementation with specific types

      // Map a function over all elements from read_tm to write_tm
      Local Core·Status ·(Map ,CVT_read ,CVT_write)·map(
        ·(TM·Array ,CVT_read) *read_tm
        ,·(TM·Array ,CVT_write) *write_tm
        ,·(Map·fn ,CVT_read ,CVT_write) map_fn
      ){
        #ifdef Map·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,read_tm ,Map·Msg·read_tm);
          Core·Guard·fg.check(&chk ,1 ,write_tm ,Map·Msg·write_tm);
          Core·Guard·fg.check(&chk ,1 ,map_fn ,Map·Msg·map_fn);
          Core·Guard·if_return(chk);
        #endif

        // Rewind both tape machines to ensure we start at the beginning
        ·(TM·Array ,CVT_read)·fg.rewind(read_tm);
        ·(TM·Array ,CVT_write)·fg.rewind(write_tm);

        // Initial check if can_read (not part of the loop)
        if(!·(TM·Array ,CVT_read)·fg.can_read(read_tm)) return Core·Status·on_track;

        // Track completion status
        uint completion = 0;

        // Following the first-rest pattern described in TTCA
        while(1){
          // Read value from source
          CVT_read read_value;
          ·(TM·Array ,CVT_read)·fg.read(read_tm ,&read_value);

          // Apply mapping function to get write value
          CVT_write write_value;
          Core·Status status = map_fn(&read_value ,&write_value);
          if(status != Core·gStatus·on_track) {
            completion |= Core·Map·Completion·failed;
            return status;
          }

          // Write result to destination
          ·(TM·Array ,CVT_write)·fg.write(write_tm ,&write_value);

          // Check if we're at the rightmost position for read
          bool read_rightmost = ·(TM·Array ,CVT_read)·fg.on_rightmost(read_tm);
          if(read_rightmost) {
            completion |= Core·Map·Completion·rightmost_read;
          }

          // Check if we're at the rightmost position for write
          bool write_rightmost = ·(TM·Array ,CVT_write)·fg.on_rightmost(write_tm);
          if(write_rightmost) {
            completion |= Core·Map·Completion·rightmost_write;
          }

          // Break if either machine is at rightmost
          if(read_rightmost || write_rightmost) break;

          // Step both machines
          ·(TM·Array ,CVT_read)·fg.step(read_tm);
          ·(TM·Array ,CVT_write)·fg.step(write_tm);
        }

        return Core·Status·on_track;
      }

      // Map a function over elements from read_tm to write_tm until a condition is met
      Local Core·Status ·(Map ,CVT_read ,CVT_write)·map_while(
        ·(TM·Array ,CVT_read) *read_tm
        ,·(TM·Array ,CVT_write) *write_tm
        ,·(Map·fn ,CVT_read ,CVT_write) map_fn
        ,bool (*condition)(CVT_read *value)
      ){
        #ifdef Map·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,read_tm ,Map·Msg·read_tm);
          Core·Guard·fg.check(&chk ,1 ,write_tm ,Map·Msg·write_tm);
          Core·Guard·fg.check(&chk ,1 ,map_fn ,Map·Msg·map_fn);
          Core·Guard·fg.check(&chk ,1 ,condition ,Map·Msg·condition);
          Core·Guard·if_return(chk);
        #endif

        // Rewind both tape machines to ensure we start at the beginning
        ·(TM·Array ,CVT_read)·fg.rewind(read_tm);
        ·(TM·Array ,CVT_write)·fg.rewind(write_tm);

        // Initial check if can_read (not part of the loop)
        if(!·(TM·Array ,CVT_read)·fg.can_read(read_tm)) return Core·Status·on_track;

        // Track completion status
        uint completion = 0;

        // Following the first-rest pattern described in TTCA
        while(1){
          // Read value from source
          CVT_read read_value;
          ·(TM·Array ,CVT_read)·fg.read(read_tm ,&read_value);

          // Check condition
          if(!condition(&read_value)) break;

          // Apply mapping function to get write value
          CVT_write write_value;
          Core·Status status = map_fn(&read_value ,&write_value);
          if(status != Core·Status·on_track) {
            completion |= Core·Map·Completion·failed;
            return status;
          }

          // Write result to destination
          ·(TM·Array ,CVT_write)·fg.write(write_tm ,&write_value);

          // Check if we're at the rightmost position for read
          bool read_rightmost = ·(TM·Array ,CVT_read)·fg.on_rightmost(read_tm);
          if(read_rightmost) {
            completion |= Core·Map·Completion·rightmost_read;
          }

          // Check if we're at the rightmost position for write
          bool write_rightmost = ·(TM·Array ,CVT_write)·fg.on_rightmost(write_tm);
          if(write_rightmost) {
            completion |= Core·Map·Completion·rightmost_write;
          }

          // Break if either machine is at rightmost
          if(read_rightmost || write_rightmost) break;

          // Step both machines
          ·(TM·Array ,CVT_read)·fg.step(read_tm);
          ·(TM·Array ,CVT_write)·fg.step(write_tm);
        }

        return Core·Status·on_track;
      }

      // Map a function over n elements from read_tm to write_tm
      Local Core·Status ·(Map ,CVT_read ,CVT_write)·map_n(
        ·(TM·Array ,CVT_read) *read_tm
        ,·(TM·Array ,CVT_write) *write_tm
        ,·(Map·fn ,CVT_read ,CVT_write) map_fn
        ,size_t n
      ){
        #ifdef Map·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,read_tm ,Map·Msg·read_tm);
          Core·Guard·fg.check(&chk ,1 ,write_tm ,Map·Msg·write_tm);
          Core·Guard·fg.check(&chk ,1 ,map_fn ,Map·Msg·map_fn);
          Core·Guard·if_return(chk);
        #endif

        // Rewind both tape machines to ensure we start at the beginning
        ·(TM·Array ,CVT_read)·fg.rewind(read_tm);
        ·(TM·Array ,CVT_write)·fg.rewind(write_tm);

        // Initial check if can_read (not part of the loop)
        if(!·(TM·Array ,CVT_read)·fg.can_read(read_tm)) return Core·Status·on_track;
        
        // Track completion status
        uint completion = 0;
        
        // Following the first-rest pattern described in TTCA
        size_t count = 0;
        while(count < n){
          // Read value from source
          CVT_read read_value;
          ·(TM·Array ,CVT_read)·fg.read(read_tm ,&read_value);

          // Apply mapping function to get write value
          CVT_write write_value;
          Core·Status status = map_fn(&read_value ,&write_value);
          if(status != Core·Status·on_track) {
            completion |= Core·Map·Completion·failed;
            return status;
          }

          // Write result to destination
          ·(TM·Array ,CVT_write)·fg.write(write_tm ,&write_value);

          // Increment count
          count++;

          // Check if we're at the rightmost position for read
          bool read_rightmost = ·(TM·Array ,CVT_read)·fg.on_rightmost(read_tm);
          if(read_rightmost) {
            completion |= Core·Map·Completion·rightmost_read;
          }

          // Check if we're at the rightmost position for write
          bool write_rightmost = ·(TM·Array ,CVT_write)·fg.on_rightmost(write_tm);
          if(write_rightmost) {
            completion |= Core·Map·Completion·rightmost_write;
          }

          // Break if either machine is at rightmost
          if(read_rightmost || write_rightmost) break;

          // Step both machines
          ·(TM·Array ,CVT_read)·fg.step(read_tm);
          ·(TM·Array ,CVT_write)·fg.step(write_tm);
        }

        return Core·Status·on_track;
      }

      // Initialize the function given table
      ·(Map·FG ,CVT_read ,CVT_write) ·(Map·fg ,CVT_read ,CVT_write) = {
        .map = ·(Map ,CVT_read ,CVT_write)·map
        ,.map_while = ·(Map ,CVT_read ,CVT_write)·map_while
        ,.map_n = ·(Map ,CVT_read ,CVT_write)·map_n
      };
    #endif // defined(CVT_read) && defined(CVT_write)

    //----------------------------------------
    // Copy functions implementation
    //----------------------------------------

    // Byte to byte copy
    Local Core·Status Map·copy_byte_to_byte(
      ·(TM·Array ,AU) *read_tm
      ,·(TM·Array ,AU) *write_tm
    ){
      #ifdef Map·DEBUG
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,read_tm ,Map·Msg·read_tm);
        Core·Guard·fg.check(&chk ,1 ,write_tm ,Map·Msg·write_tm);
        Core·Guard·if_return(chk);
      #endif

      return ·(Map ,AU ,AU)·fg.map(read_tm, write_tm, Map·byte_to_byte_fn);
    }

    // Hex to byte copy
    Local Core·Status Map·copy_hex_to_byte(
      ·(TM·Array ,uint16_t) *read_tm
      ,·(TM·Array ,AU) *write_tm
    ){
      #ifdef Map·DEBUG
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,read_tm ,Map·Msg·read_tm);
        Core·Guard·fg.check(&chk ,1 ,write_tm ,Map·Msg·write_tm);
        Core·Guard·if_return(chk);
      #endif

      return <response clipped><NOTE>To save on context only part of this file has been shown to you. You should retry this tool after you have searched inside the file with `grep -n` in order to find the line numbers of what you are looking for.</NOTE>
