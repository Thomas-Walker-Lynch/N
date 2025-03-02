/*
  Core - core memory operations.

  'ATP'  'At This Point' in the code. Used in comments to state assertions.
   by definition an 'extent' is one less than a 'size'.
*/

#define Core·DEBUG

#ifndef FACE
#define Core·IMPLEMENTATION
#define FACE
#endif 

//--------------------------------------------------------------------------------
// Interface

#ifndef Core·FACE
#define Core·FACE

  #include <stdint.h>
  #include <stddef.h>

  #define extentof(x) (sizeof(x) - 1)
  #define extent_t size_t

  typedef struct{
    void *read0; 
    extent_t read_extent;
    void *write0;
    extent_t write_extent;
  } Core·It;

  typedef enum{
    Core·It·Status·valid = 0
    ,Core·It·Status·null
    ,Core·It·Status·null_read
    ,Core·It·Status·null_write
    ,Core·It·Status·overlap
  } Core·It·Status;

  typedef enum{
     Core·Step·perfect_fit = 0
    ,Core·Step·argument_guard // something wrong with the arguments to step
    ,Core·Step·read_surplus
    ,Core·Step·read_surplus_write_gap
    ,Core·Step·write_available
    ,Core·Step·write_gap
  } Core·Step·Status;

  typedef struct{
    bool Core·IntervalPts·in(void *pt ,void *pt0 ,void *pt1);
    bool Core·IntervalPts·contains(void *pt00 ,void *pt01 ,void *pt10 ,void *pt11);
    bool Core·IntervalPts·overlap(void *pt00 ,void *pt01 ,void *pt10 ,void *pt11);

    bool Core·IntervalPtSize·in(void *pt ,void *pt0 ,size_t s);
    bool Core·IntervalPtSize·overlap(void *pt00 ,size_t s0 ,void *pt10 ,size_t s1);

    Core·It·Status Core·wellformed_it(Core·It *it)

    void *identity(void *read0 ,void *read1 ,void *write0);
    void *reverse_byte_order(void *read0 ,void *read1 ,void *write0);

    Core·Status Core·Step·identity(Core·It *it);
    Core·Status Core·Step·reverse_order(Core·It *it);
    Core·Status Core·Step·write_hex(Core·It *it);
    Core·Status Core·Step·read_hex(Core·It *it);
  } Core·M;

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef Core·IMPLEMENTATION

  #ifdef Core·DEBUG
    #include <stdio.h>
  #endif

  // this part goes into Copylib.a
  // yes this is empty, so there is no Copylib.a
  #ifndef LOCAL
  #endif 

  #ifdef LOCAL

    // Interval predicates.
    // Intervals in Copy have inclusive bounds

    Local bool Core·aligned64(void *p){
      return ((uintptr_t)p & 0x7) == 0;
    }

    Local bool Core·IntervalPts·in(void *pt ,void *pt0 ,void *pt1){
      return pt >= pt0 && pt <= pt1; // Inclusive bounds
    }

    Local bool Core·IntervalPtExtent·in(void *pt ,void *pt0 ,extent_t e){
      return Core·IntervalPts·in(pt ,pt0 ,pt0 + e);
    }

    // interval 0 contains interval 1, overlap on boundaries allowed.
    Local bool Core·IntervalPts·contains(
      void *pt00 ,void *pt01 ,void *pt10 ,void *pt11
    ){
     return pt10 >= pt00 && pt11 <= pt01;
    }

    Local bool Core·IntervalPtExtent·contains(
      void *pt00 ,size_t e0 ,void *pt10 ,size_t e1
    ){
      contains(pt00 ,pt00 + e0 ,pt10 ,pt10 + e1)
    }

    // interval 0 properly contains interval 1, overlap on boundaries not allowed.
    Local bool Core·IntervalPts·contains_proper(
      void *pt00 ,void *pt01 ,void *pt10 ,void *pt11
    ){
     return pt10 > pt00 && pt11 < pt01;
    }
    Local bool Core·IntervalPtExtent·contains_proper(
      void *pt00 ,size_t e0 ,void *pt10 ,size_t e1
    ){
      contains_proper(pt00 ,pt00 + e0 ,pt10 ,pt10 + 1)
    }


    // Possible cases of overlap, including just touching
    // 1. interval 0 to the right of interval 1, just touching p00 == p11
    // 2. interval 0 to the left of interval 1, just touching p01 == p10
    // 3. interval 0 wholly contained in interval 1
    // 4. interval 0 wholly contains interval 1
    Local bool Core·IntervalPts·overlap(void *pt00 ,void *pt01 ,void *pt10 ,void *pt11){
      return 
        Core·IntervalPts·in(pt00 ,pt10 ,pt11) // #1, #3
        || Core·IntervalPts·in(pt10 ,pt00 ,pt01) // #2, #4
        ;
    }

    Local bool Core·IntervalPtExtent·overlap(
      void *pt00 ,extent_t e0 ,void *pt10 ,extent_t e1
    ){
      return Core·IntervalPts·overlap(pt00 ,pt00 + e0 ,pt10 ,pt10 + e1);
    }

    Local Copy·It·Status Copy·wellformed_it(Copy·it *it){

      bool print = false;
      #ifdef Core·DEBUG
        print = true;
      #endif 
      
      char *this_name = "Copy·wellformed_it";
      Copy·WFIt·Status status = Copy·WFIt·Status·valid;

      if(it == NULL){
        if(print) fprintf( stderr ,"%s: NULL read pointer\n" ,this_name );
        return Core·It·Status·null;
      }

      if(it->read0 == NULL){
        if(print) fprintf( stderr ,"%s: NULL read pointer\n" ,this_name );
        status |= Copy·WFIt·Status·null_read;
      }

      if(it->write0 == NULL){
        if(print) fprintf( stderr ,"%s: NULL write pointer\n" ,this_name );
        status |= Copy·WFIt·Status·null_write;
      }

      if(it->read_size == 0){
        if(print) fprintf( stderr ,"%s: Zero-sized read buffer\n" ,this_name );
        status |= Copy·WFIt·Status·zero_read_buffer;
      }

      if(it->write_size == 0){
        if(print) fprintf( stderr ,"%s: Zero-sized write buffer\n" ,this_name );
        status |= Copy·WFIt·Status·zero_write_buffer;
      }

      if( Copy·overlap_size_interval(it->read0 ,it->read_size ,it->write0 ,it->write_size) ){
        if(print) fprintf( stderr ,"%s: Read and write buffers overlap!\n" ,this_name );
        status |= Copy·WFIt·Status·overlap;
      }

      return status;
    }

    // consider an 8 byte window that is aligned
    // returns the byte pointer to the least address byte in the window
    Local void *Core·floor64(void *p){
      return (uintptr_t)p & ~(uintptr_t)0x7;
    }

    // consider an 8 byte window that is aligned
    // returns the byte pointer to the greatest address byte in the window
    Local void *Core·ceiling64(void *p){
      return (uintptr_t)p | 0x7;
    }

    // byte array greatest address byte at p1 (inclusive)
    // byte array least address byte at p0 (inclusive)
    // returns pointer to the greatest full 64-bit word-aligned address that is ≤ p1
    // by contract, p1 must be >= p0
    Local uint64_t *Core·greatest_full_64(void *p0 ,void *p1){

      // If p1 - 0x7 moves into a prior word while p0 does not, a prefetch hazard can occur.
      // If p1 and p0 are more than 0x7 apart, they cannot be in the same word,
      // but this does not guarantee a full 64-bit word exists in the range.
      if((uintptr_t)p1 < (uintptr_t)p0 + 0x7) return NULL;

      // Compute the last fully aligned word at or before p1.
      uint64_t *p1_64 = (void *)( ((uintptr_t)p1 - 0x7) & ~(uintptr_t)0x7 );

      // If alignment rounds p1_64 below p0, there is no full word available.
      if(p1_64 < p0) return NULL;

      return p1_64;
    }

    // byte array greatest address byte at p1 (inclusive)
    // byte array least address byte at p0 (inclusive)
    // returns pointer to the least full 64-bit word-aligned address that is ≥ p0
    Local uint64_t *Core·least_full_64(void *p0 ,void *p1){

      // If p0 + 0x7 moves into the next word while p1 does not, a prefetch hazard can occur.
      // If p1 and p0 are more than 0x7 apart, they cannot be in the same word,
      // but this does not guarantee a full 64-bit word exists in the range.
      if(p1 - p0 < 0x7) return NULL;

      // Compute the first fully aligned word at or after p0.
      uint64_t *p0_64 = (void *)( ((uintptr_t)p0 + 0x7) & ~(uintptr_t)0x7 );

      // If alignment rounds p0_64 beyond p1, there is no full word available.
      if(p0_64 > p1) return NULL;

      return p0_64;
    }

    Local void *Core·inc64(void *p ,size_t Δ){
      return (void *)((uint64_t *)p) + Δ;
    }

    Local uint64_t Core·read_word_fwd(uint64_t *r){
      return *r;
    }

    Local uint64_t Core·read_word_rev(uint64_t *r0 ,uint64_t *r1 ,uint64_t *r){
      return __builtin_bswap64(*(Core·floor64(r0 + (r1 - r))));
    }

Local Core·Step·Status Core·step(
   Core·It *it
   ,Core·Step·Status (*fn)(
      uint8_t *r ,uint8_t *r1 ,uint8_t *w ,uint8_t *w1
   )
){
   //----------------------------------------
   // Validate Iterator
   //

   Core·It·Status status = Core·wellformed_it(it);
   if(status != Core·It·Status·valid) return Core·Step·argument_guard;

   //----------------------------------------
   // Setup pointers
   //

   uint8_t *r0 = (uint8_t *)it->read0;
   uint8_t *r1 = r0 + it->read_extent;  // Inclusive bound

   uint8_t *w0 = (uint8_t *)it->write0;
   uint8_t *w1 = w0 + it->write_extent; // Inclusive bound

   //----------------------------------------
   // Apply function iteratively
   //

   Core·Step·Status step_status;
   do{
      step_status = fn(r0 ,r1 ,w0 ,w1);
      if(
         step_status != Core·Step·write_available 
         && step_status != Core·Step·read_surplus
      ) break;

   }while(true);

   //----------------------------------------
   // Update iterator
   //

   it->read0 = r0;
   it->write0 = w0;
   it->read_extent = r1 - r0;
   it->write_extent = w1 - w0;

   return step_status;
}


    Local void *Core·map(
       uint8_t *r0 ,uint8_t *r1 ,uint8_t *w0
       ,size_t read_inc ,size_t write_inc
       ,uint8_t (*read_fn)(uint8_t * ,uint8_t * ,uint8_t *)  
       ,uint8_t (*map_fn)(uint8_t)
    ){
       //----------------------------------------
       // Argument guard
       //

       if(r1<r0) return NULL;

       //----------------------------------------
       // Setup pointers
       //

       uint8_t *r = r0;
       uint8_t *w = w0;

       //----------------------------------------
       // Byte-wise copy with transformation
       //

       do{
          *w = map_fn( read_fn(r0 ,r1 ,r) );
          if(r==r1) break;
          w += write_inc;
          r += read_inc;
       }while(true);

       return w;
    }

    Local void *Core·copy_byte_by_byte(
       uint8_t *r0 ,uint8_t *r1 ,uint8_t *w0 ,bool reverse
    ){
       //----------------------------------------
       // Argument guard
       //

       if(r1<r0) return NULL;

       //----------------------------------------
       // Setup pointers
       //

       uint8_t *r = r0;
       uint8_t *w = w0;

       // Function pointer for dynamic read behavior
       uint8_t (*read_byte)(uint8_t * ,uint8_t * ,uint8_t *)
         = reverse ? Core·read_byte_rev : Core·read_byte_fwd;

       //----------------------------------------
       // Byte-wise copy
       //

       do{
          *w = read_byte(r0 ,r1 ,r);
          if(r==r1) break;
          w++;
          r++;
       }while(true);

       return w;
    }

    Local void *Core·copy_by_word64(void *read0 ,void *read1 ,void *write0 ,bool reverse){

      //----------------------------------------
      // Argument guard

      if(read1 < read0) return NULL;

      //----------------------------------------
      // Setup pointers
      
      // the read interval, for byte arrays
      uint8_t *r0 = (uint8_t *)read0;
      uint8_t *r1 = (uint8_t *)read1; // inclusive upper bound
      uint8_t *w0 = (uint8_t *)write0;

      // the contained word interval, inclusive bounds
      uint64_t *r0_64 = Core·least_full_64(r0 ,r1);
      uint64_t *r1_64 = Core·greatest_full_64(r0 ,r1);

      // swap byte order done by overloading the read function
      uint8_t (*read_byte)(uint8_t * ,uint8_t * ,uint8_t *)  
          = reverse ? Core·read_byte_rev : Core·read_byte_fwd;

      uint64_t (*read_word)(uint64_t * ,uint64_t * ,uint64_t *) 
          = reverse ? Core·read_word_rev : Core·read_word_fwd;

      // If no full words ,perform byte-wise copy
      if(r0_64 == NULL || r1_64 == NULL) return Core·copy_byte_by_byte(r0 ,r1 ,w0 ,reverse);

      //----------------------------------------
      // Align `r` to first full 64-bit word boundary

      uint8_t *w=w0;
      if( !Core·aligned64(r0) ){
        w = Core·copy_byte_by_byte(r0 ,r0_64 - 1 ,w ,reverse);
      }
      uint8_t *r = r0_64;

      //----------------------------------------
      // Bulk word-wise copy

      do{
        *(uint64_t *)w = read_word(r0_64 ,r1_64 ,(uint64_t *)r);
        if(r == (uint8_t *)r1_64) break;
        w = Core·inc64(w ,1);
        r = Core·inc64(r ,1);
      }while(true);

      // If r1 was aligned ,we're done
      if(Core·aligned64(r1)) return w;
      w = Core·inc64(w ,1);
      r = Core·inc64(r ,1);

      //----------------------------------------
      // Ragged tail (byte-wise copy)

      return Core·copy_byte_by_byte(r ,r1 ,w ,reverse);
    }


    /*
      The copy_kernel function is either copy_by_word64, or copy_byte_by_byte.
    */
    Local Core·Status Core·copy_step(
       Core·It *it
       ,void *(*copy_kernel)(void * ,void * ,void * ,bool)
       ,bool reverse
    ){
       uint8_t *r = (uint8_t *)it->read0;
       uint8_t *w = (uint8_t *)it->write0;

       extent_t re = it->read_extent;
       extent_t we = it->write_extent;

       if(we >= re){
          copy_kernel(r ,r + re ,w ,reverse);
          it->read0 += re;
          it->read_extent = 0;
          it->write0 += re;
          it->write_extent -= re;
          if(we == re) return Core·Step·perfect_fit;
          return Core·Step·write_available;
       }

       copy_kernel(r ,r + we ,w ,reverse);
       it->read0 += we;
       it->read_extent -= we;
       it->write_extent = 0;
       it->write0 += we;
       return Core·Step·read_surplus;
    }

  #endif // LOCAL

#endif // IMPLEMENTATION
