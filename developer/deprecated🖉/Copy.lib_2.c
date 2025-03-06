/*
  CoreCopy - Memory copy operations with attention to alignment.
  Provides optimized copy and byte order reversal functions.

  'ATP'  At This Point in the code. Assertions follow.
*/

#define CoreCopy·DEBUG

#ifndef FACE
#define CoreCopy·IMPLEMENTATION
#define FACE
#endif 

//--------------------------------------------------------------------------------
// Interface

#ifndef CoreCopy·FACE
#define CoreCopy·FACE

  #include <stdint.h>
  #include <stddef.h>

  #define extentof(x) (sizeof(x) - 1)
  #define extent_t size_t

  typedef struct{
    void *read0; 
    extent_t read_extent;
    void *write0;
    extent_t write_extent;
  } CoreCopy·It;

  typedef enum{
    CoreCopy·It·Status·valid = 0
    ,CoreCopy·It·Status·null_read
    ,CoreCopy·It·Status·null_write
    ,CoreCopy·It·Status·overlap
  } CoreCopy·It·Status;

  typedef enum{
     CoreCopy·Step·perfect_fit = 0
    ,CoreCopy·Step·argument_guard 
    ,CoreCopy·Step·read_surplus
    ,CoreCopy·Step·read_surplus_write_gap
    ,CoreCopy·Step·write_available
    ,CoreCopy·Step·write_gap
  } CoreCopy·Status;

  typedef struct{
    bool CoreCopy·IntervalPts·in(void *pt, void *pt0 ,void *pt1);
    bool CoreCopy·IntervalPts·contains(void *pt00 ,void *pt01 ,void *pt10 ,void *pt11);
    bool CoreCopy·IntervalPts·overlap(void *pt00 ,void *pt01, void *pt10 ,void *pt11);

    bool CoreCopy·IntervalPtSize·in(void *pt, void *pt0 ,size_t s);
    bool CoreCopy·IntervalPtSize·overlap(void *pt00 ,size_t s0, void *pt10 ,size_t s1);

    CoreCopy·It·Status CoreCopy·wellformed_it(CoreCopy·It *it)

    void *identity(void *read0 ,void *read1 ,void *write0);
    void *reverse_byte_order(void *read0 ,void *read1 ,void *write0);

    CoreCopy·Status CoreCopy·Step·identity(CoreCopy·It *it);
    CoreCopy·Status CoreCopy·Step·reverse_order(CoreCopy·It *it);
    CoreCopy·Status CoreCopy·Step·write_hex(CoreCopy·It *it);
    CoreCopy·Status CoreCopy·Step·read_hex(CoreCopy·It *it);
  } CoreCopy·M;

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef CoreCopy·IMPLEMENTATION

  #ifdef CoreCopy·DEBUG
    #include <stdio.h>
  #endif

  // this part goes into Copylib.a
  // yes this is empty, so there is no Copylib.a
  #ifndef LOCAL
  #endif 

  #ifdef LOCAL

    // Interval predicates.
    // Intervals in Copy have inclusive bounds

    Local bool CoreCopy·aligned64(void *p){
      return ((uintptr_t)p & 0x7) == 0;
    }

    Local bool CoreCopy·IntervalPts·in(void *pt, void *pt0 ,void *pt1){
      return pt >= pt0 && pt <= pt1; // Inclusive bounds
    }

    Local bool CoreCopy·in_extent_interval(void *pt, void *pt0 ,extent_t e){
      return CoreCopy·IntervalPts·in(pt ,pt0 ,pt0 + e);
    }

    // interval 0 contains interval 1, overlap on boundaries allowed.
    Local bool CoreCopy·IntervalPts·contains(
      void *pt00 ,void *pt01 ,void *pt10 ,void *pt11
    ){
     return pt10 >= pt00 && pt11 <= pt01;
    }

    // interval 0 properly contains interval 1, overlap on boundaries not allowed.
    Local bool CoreCopy·contains_proper_pt_interval(
      void *pt00 ,void *pt01 ,void *pt10 ,void *pt11
    ){
     return pt10 > pt00 && pt11 < pt01;
    }

    // Possible cases of overlap, including just touching
    // 1. interval 0 to the right of interval 1, just touching p00 == p11
    // 2. interval 0 to the left of interval 1, just touching p01 == p10
    // 3. interval 0 wholly contained in interval 1
    // 4. interval 0 wholly contains interval 1
    Local bool CoreCopy·IntervalPts·overlap(void *pt00 ,void *pt01, void *pt10 ,void *pt11){
      return 
        CoreCopy·IntervalPts·in(pt00 ,pt10 ,pt11) // #1, #3
        || CoreCopy·IntervalPts·in(pt10 ,pt00 ,pt01) // #2, #4
        ;
    }

    Local bool CoreCopy·overlap_extent_interval(void *pt00 ,extent_t e0, void *pt10 ,extent_t e1){
      return CoreCopy·IntervalPts·overlap(pt00 ,pt00 + e0 ,pt10 ,pt10 + e1);
    }

    Local CoreCopy·It·Status CoreCopy·It·wellformed(CoreCopy·It *it){
      char *this_name = "CoreCopy·It·wellformed";
      CoreCopy·It·Status status = CoreCopy·It·Status·valid;

      if(it->read0 == NULL){
        fprintf(stderr, "%s: NULL read pointer\n", this_name);
        status |= CoreCopy·It·Status·null_read;
      }

      if(it->write0 == NULL){
        fprintf(stderr, "%s: NULL write pointer\n", this_name);
        status |= CoreCopy·It·Status·null_write;
      }

      if(
        CoreCopy·overlap_extent_interval(it->read0 ,it->read_extent ,it->write0 ,it->write_extent)
        ){
          fprintf(stderr, "%s: Read and write buffers overlap!\n", this_name);
          status |= CoreCopy·It·Status·overlap;
      }

      return status;
    }

    // consider an 8 byte window that is aligned
    // returns the byte pointer to the least address byte in the window
    Local void *CoreCopy·floor64(void *p){
      return (uintptr_t)p & ~(uintptr_t)0x7;
    }

    // consider an 8 byte window that is aligned
    // returns the byte pointer to the greatest address byte in the window
    Local void *CoreCopy·ceiling64(void *p){
      return (uintptr_t)p | 0x7;
    }

    // byte array greatest address byte at p1 (inclusive)
    // byte array least address byte at p0 (inclusive)
    // returns pointer to the greatest full 64-bit word-aligned address that is ≤ p1
    // by contract, p1 must be >= p0
    Local uint64_t *CoreCopy·greatest_full_64(void *p0 ,void *p1){

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
    Local uint64_t *CoreCopy·least_full_64(void *p0 ,void *p1){

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

    Local void *CoreCopy·inc64(void *p ,size_t Δ){
      return (void *)((uint64_t *)p) + Δ;
    }

    Local uint64_t CoreCopy·read_word_fwd(uint64_t *r){
      return *r;
    }

    Local uint64_t CoreCopy·read_word_rev(uint64_t *r0, uint64_t *r1, uint64_t *r){
      return __builtin_bswap64(*(CoreCopy·floor64(r0 + (r1 - r))));
    }

    Local void *CoreCopy·byte(
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
       uint8_t (*read_byte)(
          uint8_t * ,uint8_t * ,uint8_t *
       ) = reverse ? CoreCopy·read_byte_rev : CoreCopy·read_byte_fwd;

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

    Local void *CoreCopy·word64(void *read0 ,void *read1 ,void *write0 ,bool reverse){

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
      uint64_t *r0_64 = CoreCopy·least_full_64(r0 ,r1);
      uint64_t *r1_64 = CoreCopy·greatest_full_64(r0 ,r1);

      // swap byte order done by overloading the read function
      uint8_t (*read_byte)(uint8_t * ,uint8_t * ,uint8_t *)  
          = reverse ? CoreCopy·read_byte_rev : CoreCopy·read_byte_fwd;

      uint64_t (*read_word)(uint64_t * ,uint64_t * ,uint64_t *) 
          = reverse ? CoreCopy·read_word_rev : CoreCopy·read_word_fwd;

      // If no full words ,perform byte-wise copy
      if(r0_64 == NULL || r1_64 == NULL) return CoreCopy·byte(r0 ,r1 ,w0 ,reverse);

      //----------------------------------------
      // Align `r` to first full 64-bit word boundary

      uint8_t *w=w0;
      if( !CoreCopy·aligned64(r0) ){
        w = CoreCopy·byte(r0 ,r0_64 - 1 ,w ,reverse);
      }
      uint8_t *r = r0_64;

      //----------------------------------------
      // Bulk word-wise copy

      do{
        *(uint64_t *)w = read_word(r0_64 ,r1_64 ,(uint64_t *)r);
        if(r == (uint8_t *)r1_64) break;
        w = CoreCopy·inc64(w ,1);
        r = CoreCopy·inc64(r ,1);
      }while(true);

      // If r1 was aligned ,we're done
      if(CoreCopy·aligned64(r1)) return w;
      w = CoreCopy·inc64(w ,1);
      r = CoreCopy·inc64(r ,1);

      //----------------------------------------
      // Ragged tail (byte-wise copy)

      return CoreCopy·byte(r ,r1 ,w ,reverse);

    }


    /*
      Read and write pointers are incremented by `extent + 1`, ensuring they do not skip
      past the last valid byte. The previous `+1` was incorrect in cases where
      stepping already processed the last byte.
    */
    Local CoreCopy·Status CoreCopy·Step·identity(CoreCopy·It *it){
      uint8_t *r = (uint8_t *)it->read0;
      uint8_t *w = (uint8_t *)it->write0;

      extent_t re = it->read_extent;
      extent_t we = it->write_extent;

      if(we >= re){
        CoreCopy·bytes(r ,r + re ,w);
        it->read0 += re;  // Fixed stepping logic
        it->read_extent = 0;
        it->write0 += re;
        it->write_extent -= re;
        if(we == re) return CoreCopy·Step·perfect_fit;
        return CoreCopy·Step·write_available;
      }

      CoreCopy·bytes(r ,r + we ,w);
      it->read0 += we;  // Fixed stepping logic
      it->read_extent -= we;
      it->write_extent = 0;
      it->write0 += we;
      return CoreCopy·Step·read_surplus;
    }

  #endif // LOCAL

#endif // IMPLEMENTATION
