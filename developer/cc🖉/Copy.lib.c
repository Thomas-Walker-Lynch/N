/*
  Copy - Memory copy operations with attention to alignment.
  Provides optimized copy and byte order reversal functions.

  'ATP'  At This Point in the code. Assertions follow.
*/

#define Copy·DEBUG

#ifndef FACE
#define Copy·IMPLEMENTATION
#define FACE
#endif 

//--------------------------------------------------------------------------------
// Interface

#ifndef Copy·FACE
#define Copy·FACE

  #include <stdint.h>
  #include <stddef.h>

  #define extentof(x) (sizeof(x) - 1)
  #define extent_t size_t

  typedef struct{
    void *read0; 
    extent_t read_extent;
    void *write0;
    extent_t write_extent;
  } Copy·It;

  typedef enum{
    Copy·It·Status·valid = 0
    ,Copy·It·Status·null_read
    ,Copy·It·Status·null_write
    ,Copy·It·Status·overlap
  } Copy·It·Status;

  typedef enum{
     Copy·Step·perfect_fit = 0
    ,Copy·Step·argument_guard 
    ,Copy·Step·read_surplus
    ,Copy·Step·read_surplus_write_gap
    ,Copy·Step·write_available
    ,Copy·Step·write_gap
  } Copy·Status;

  typedef struct{
    bool Copy·IntervalPts·in(void *pt, void *pt0 ,void *pt1);
    bool Copy·IntervalPts·contains(void *pt00 ,void *pt01 ,void *pt10 ,void *pt11);
    bool Copy·IntervalPts·overlap(void *pt00 ,void *pt01, void *pt10 ,void *pt11);

    bool Copy·IntervalPtSize·in(void *pt, void *pt0 ,size_t s);
    bool Copy·IntervalPtSize·overlap(void *pt00 ,size_t s0, void *pt10 ,size_t s1);

    Copy·It·Status Copy·wellformed_it(Copy·It *it)

    void *identity(void *read0 ,void *read1 ,void *write0);
    void *reverse_byte_order(void *read0 ,void *read1 ,void *write0);

    Copy·Status Copy·Step·identity(Copy·It *it);
    Copy·Status Copy·Step·reverse_order(Copy·It *it);
    Copy·Status Copy·Step·write_hex(Copy·It *it);
    Copy·Status Copy·Step·read_hex(Copy·It *it);
  } Copy·M;

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef Copy·IMPLEMENTATION

  #ifdef Copy·DEBUG
    #include <stdio.h>
  #endif

  // this part goes into Copylib.a
  // yes this is empty, so there is no Copylib.a
  #ifndef LOCAL
  #endif 

  #ifdef LOCAL

    // Interval predicates.
    // Intervals in Copy have inclusive bounds

    Local bool Copy·IntervalPts·in(void *pt, void *pt0 ,void *pt1){
      return pt >= pt0 && pt <= pt1; // Inclusive bounds
    }

    Local bool Copy·in_extent_interval(void *pt, void *pt0 ,extent_t e){
      return Copy·IntervalPts·in(pt ,pt0 ,pt0 + e);
    }

    // interval 0 contains interval 1, overlap on boundaries allowed.
    Local bool Copy·IntervalPts·contains(
      void *pt00 ,void *pt01 ,void *pt10 ,void *pt11
    ){
     return pt10 >= pt00 && pt11 <= pt01;
    }

    // interval 0 properly contains interval 1, overlap on boundaries not allowed.
    Local bool Copy·contains_proper_pt_interval(
      void *pt00 ,void *pt01 ,void *pt10 ,void *pt11
    ){
     return pt10 > pt00 && pt11 < pt01;
    }

    // Possible cases of overlap, including just touching
    // 1. interval 0 to the right of interval 1, just touching p00 == p11
    // 2. interval 0 to the left of interval 1, just touching p01 == p10
    // 3. interval 0 wholly contained in interval 1
    // 4. interval 0 wholly contains interval 1
    Local bool Copy·IntervalPts·overlap(void *pt00 ,void *pt01, void *pt10 ,void *pt11){
      return 
        Copy·IntervalPts·in(pt00 ,pt10 ,pt11) // #1, #3
        || Copy·IntervalPts·in(pt10 ,pt00 ,pt01) // #2, #4
        ;
    }

    Local bool Copy·overlap_extent_interval(void *pt00 ,extent_t e0, void *pt10 ,extent_t e1){
      return Copy·IntervalPts·overlap(pt00 ,pt00 + e0 ,pt10 ,pt10 + e1);
    }

    Local Copy·It·Status Copy·It·wellformed(Copy·It *it){
      char *this_name = "Copy·It·wellformed";
      Copy·It·Status status = Copy·It·Status·valid;

      if(it->read0 == NULL){
        fprintf(stderr, "%s: NULL read pointer\n", this_name);
        status |= Copy·It·Status·null_read;
      }

      if(it->write0 == NULL){
        fprintf(stderr, "%s: NULL write pointer\n", this_name);
        status |= Copy·It·Status·null_write;
      }

      if(
        Copy·overlap_extent_interval(it->read0 ,it->read_extent ,it->write0 ,it->write_extent)
        ){
          fprintf(stderr, "%s: Read and write buffers overlap!\n", this_name);
          status |= Copy·It·Status·overlap;
      }

      return status;
    }

    // consider an 8 byte window that is aligned
    // returns the byte pointer to the least address byte in the window
    Local void *Copy·floor_64(void *p){
      return (uintptr_t)p & ~(uintptr_t)0x7;
    }

    // consider an 8 byte window that is aligned
    // returns the byte pointer to the greatest address byte in the window
    Local void *Copy·ceiling_64(void *p){
      return (uintptr_t)p | 0x7;
    }

    // byte array greatest address byte at p1 (inclusive)
    // byte array least address byte at p0 (inclusive)
    // returns pointer to the greatest full 64-bit word-aligned address that is ≤ p1
    // by contract, p1 must be >= p0
    Local uint64_t *Copy·greatest_full_64(void *p0 ,void *p1){

      // If p1 - 0x7 moves into a prior word while p0 does not, a prefetch hazard can occur.
      // If p1 and p0 are more than 0x7 apart, they cannot be in the same word,
      // but this does not guarantee a full 64-bit word exists in the range.
      if (p1 - p0 < 0x7) return NULL;

      // Compute the last fully aligned word at or before p1.
      uint64_t *p1_64 = (void *)( ((uintptr_t)p1 - 0x7) & ~(uintptr_t)0x7 );

      // If alignment rounds p1_64 below p0, there is no full word available.
      if(p1_64 < p0) return NULL;

      return p1_64;
    }

    // byte array greatest address byte at p1 (inclusive)
    // byte array least address byte at p0 (inclusive)
    // returns pointer to the least full 64-bit word-aligned address that is ≥ p0
    Local uint64_t *Copy·least_full_64(void *p0 ,void *p1){

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

    Local void *Copy·inc64(void *p ,size_t Δ){
      return (void *)((uint64_t *)p) + Δ;
    }

    Local void *Copy·identity(void *read0 ,void *read1 ,void *write0){

      //----------------------------------------
      // argument guard

      if(read1 < read0) return NULL;
      // ATP there is at least one byte to be copied

      //----------------------------------------
      // features of the byte arrays, optimizer should move this code around

      uint8_t *r0 = (uint8_t *)read0;
      uint8_t *r1 = (uint8_t *)read1; // inclusive upper bound
      uint8_t *w0 = (uint8_t *)write0;

      uint8_t *r = r0;
      uint8_t *w = w0;

      // the contained uint64_t array
      uint64_t *r0_64 = Copy·least_full_64(r0 ,r1);
      uint64_t *r1_64 = Copy·greatest_full_64(r0 ,r1);

      // ATP there might be unaligned smallest address in the array bytes

      // In fact, r0_64 and r1_64 being NULL will always occur together.
      // .. then there are not many bytes to be copied
      if(r0_64 == NULL || r1_64 == NULL){
        do{
          *w = *r;
          if(r == r1) break;
          w++;
          r++;
        }while(true);
        return w;
      }

      // if needed, align r
      while(r < r0_64){ 
        *w++ = *r++;
      }
      // ATP r == r0_64, though *r has not yet been copied
      // ATP r is uint64_t aligned
      // ATP there is at least one word to be copied
      // ATP w is possibly not aligned

      //----------------------------------------
      // The bulk copy part 

      do{
        *(uint64_t *)w = *(uint64_t *)r;
        if(r == r1_64) break;
        w = Copy·inc64(w ,1);
        r = Copy·inc64(r ,1);
      }while(true);
      // ATP r == r1_64

      // If r1 was aligned the copy is done
      bool aligned_r1 = (uintptr_t)r1 == (uintptr_t)Copy·ceiling_64(r1_64);
      if(aligned_r1) return w;
      r = Copy·inc_64(r ,1);
      w = Copy·inc_64(w ,1);
      // ATP there is at least one trailing unaligned byte to copy
      // *r has not yet been copied, but needs to be

      //----------
      // The ragged tail, up to 7 bytes
      do{
        *w = *r;
        if(r == r1) break;
        w++;
        r++;
      }while(true);

      return w;
    }

    Local void *Copy·reverse_byte_order(void *read0 ,void *read1 ,void *write0){

      //----------------------------------------
      // Argument guard

      if(read1 < read0) return NULL;
      // ATP there is at least one byte to be copied

      //----------------------------------------
      // Features of the byte arrays, optimizer should move this code around

      uint8_t *r0 = (uint8_t *)read0;
      uint8_t *r1 = (uint8_t *)read1; // inclusive upper bound
      uint8_t *w0 = (uint8_t *)write0;

      uint8_t *r = r1; // Start from the last byte
      uint8_t *w = w0;

      // The contained uint64_t array
      uint64_t *r0_64 = Copy·least_full_64(r0 ,r1);
      uint64_t *r1_64 = Copy·greatest_full_64(r0 ,r1);

      // ATP there might be unaligned highest address in the array bytes

      // If no full words exist, fallback to byte-wise copying
      if(r0_64 == NULL || r1_64 == NULL){
        do{
          *w = *r;
          if(r == r0) break;
          w++;
          r--;
        }while(true);
        return w;
      }

      // If needed, align r
      while(r > (uint8_t *)r1_64){
        *w++ = *r--;
      }
      // ATP r == r1_64, though *r has not yet been copied
      // ATP r is uint64_t aligned
      // ATP there is at least one word to be copied
      // ATP w is possibly not aligned

      //----------------------------------------
      // The bulk copy part 

      do{
        *(uint64_t *)w = __builtin_bswap64(*(uint64_t *)r);
        if(r == r0_64) break;
        w = Copy·inc64(w ,1);
        r = Copy·inc64(r ,-1);
      }while(true);
      // ATP r == r0_64

      // If r0 was aligned, the copy is done
      bool aligned_r0 = (uintptr_t)r0 == (uintptr_t)Copy·floor_64(r0_64);
      if(aligned_r0) return w;

      r = Copy·inc64(r ,-1);
      w = Copy·inc64(w ,1);
      // ATP there is at least one trailing unaligned byte to copy
      // *r has not yet been copied, but needs to be

      //----------
      // The ragged tail, up to 7 bytes
      do{
        *w = *r;
        if(r == r0) break;
        w++;
        r--;
      }while(true);

      return w;
    }


    /*
      Read and write pointers are incremented by `extent + 1`, ensuring they do not skip
      past the last valid byte. The previous `+1` was incorrect in cases where
      stepping already processed the last byte.
    */
    Local Copy·Status Copy·Step·identity(Copy·It *it){
      uint8_t *r = (uint8_t *)it->read0;
      uint8_t *w = (uint8_t *)it->write0;

      extent_t re = it->read_extent;
      extent_t we = it->write_extent;

      if(we >= re){
        Copy·bytes(r ,r + re ,w);
        it->read0 += re;  // Fixed stepping logic
        it->read_extent = 0;
        it->write0 += re;
        it->write_extent -= re;
        if(we == re) return Copy·Step·perfect_fit;
        return Copy·Step·write_available;
      }

      Copy·bytes(r ,r + we ,w);
      it->read0 += we;  // Fixed stepping logic
      it->read_extent -= we;
      it->write_extent = 0;
      it->write0 += we;
      return Copy·Step·read_surplus;
    }

  #endif // LOCAL

#endif // IMPLEMENTATION
