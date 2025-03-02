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
     Core·Step·Status·perfect_fit = 0
    ,Core·Step·Status·argument_guard // something wrong with the arguments to step
    ,Core·Step·Status·read_surplus
    ,Core·Step·Status·read_surplus_write_gap
    ,Core·Step·Status·write_available
    ,Core·Step·Status·write_gap
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



typedef void *(*step_fn_t)(Core·It *it ,void *tableau);

// Function prototypes, for forward referencing
step_fn_t Core·CopyWord64·init ,Core·CopyWord64·leadin ,Core·CopyWord64·bulk ,Core·CopyWord64·tail;

// copy_word64 tableau structure
typedef struct{
  Core·Step·Status status
} Core·Step·tableau_t;

typedef struct{
  Core·Step·Step·Status status
  ,uint64_t *r0_64
  ,uint64_t *r1_64
} Core·CopyWord64·tableau_t;


// Initialize the copy_word64
copy_fn_t Core·Step·CopyWord64·init(Core·Step·It it ,Core·Step·tableau_t *t0){
  copy_step_tableau_t *t = (copy_step_tableau_t *) t0;
  // if iterator not well formed set status and return NULL     
  // initialize the tableau struct from the iterator ..
  // ATP we know at least one byte must be copied
  // if r0_64 or r1_64 are NULL, copy the bytes, set status, and return NULL
  return Core·Step·CopyWord64·leadin;
}

// Lead-in byte copy (until alignment)
void *Core·Step·CopyWord64·leadin(Core·Step·It it ,Core·Step·tableau_t *t0){
  copy_step_tableau_t *t = (copy_step_tableau_t *)t0;
  while(r < (uint8_t *)tableau->r0_64){
    *w++ = *r++;
  }
  return Core·Step·CopyWord64·bulk;
}

// Bulk word copy
void *Core·Step·CopyWord64·bulk(Core·Step·It it ,Core·Step·tableau_t *t0){
  copy_step_tableau_t *t = (copy_step_tableau_t *)t0;
  uint64_t *r64 = (uint64_t *)r;
  uint64_t *r1_64 = tableau->r1_64;
  uint64_t *w64 = (uint64_t *)w;

  while(r64 <= r1_64){
    *w64++ = *r64++;
  }
  // check if read1 is aligned if so, set status and return NULL otherwise
  return Core·Step·CopyWord64·tail;
}

// Tail byte copy
void *Core·Step·CopyWord64·tail(Core·Step·It it ,Core·Step·tableau_t *t){
  while(r <= r1){
    *w++ = *r++;
  }
  // set status on the tableau
  return NULL;
}

// Step function
Core·Step·Status step(Core·Step·It it ,step_fn_t fn ,Core·Step·tableau_t *t){
  while(fn(it ,t));
  return t->status;
}



  #endif // LOCAL

#endif // IMPLEMENTATION
