/*
  Copy - Memory copy operations with attention to alignment.
  Provides optimized copy and byte order reversal functions.
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

  void *Copy·region(void *read0 ,void *read1 ,void *write0);
  void *Copy·reverse_byte_order(void *read0 ,void *read1 ,void *write0);

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef Copy·IMPLEMENTATION

  // this part goes into Nlib.a
  #ifndef LOCAL
  #endif 

  #ifdef LOCAL

    /*
      Copy·region - Copies a memory region while preserving byte order.
      - Aligns reads for performance.
      - Writes are assumed to be buffered and do not require alignment.
      - Returns the updated write pointer.
    */
    void *Copy·region(void *read0 ,void *read1 ,void *write0){

      uint8_t *r = (uint8_t *)read0;
      uint8_t *r1 = (uint8_t *)read1;
      uint8_t *w = (uint8_t *)write0;

      //----------
      // The potentially unaligned initial part (align read pointer).
      if( (uintptr_t)r & 0x7 ){

        // ORing in `0x7` adds at most six bytes to r.
        uint8_t *r01 = (uint8_t *)((uintptr_t)r | 0x7);

        // If the read interval is very small
        if(r01 >= r1){
          while(r < r1){
            *w++ = *r++;
          }
          return w;
        }

        // Copy up to alignment boundary
        do{ 
          *w++ = *r++;
        }while(r <= r01);
      }
      // r is now aligned, but *r has not yet been copied

      //----------
      // The bulk copy part (w is still possibly unaligned, but r is aligned)
      uint8_t *r10 = (uint8_t *)((uintptr_t)r1 & ~(uintptr_t)0x7);

      while(r < r10){
        *(uint64_t *)w = *(uint64_t *)r;
        w += 8;
        r += 8;
      }

      // If r1 was aligned then r10 == r1 and we are done
      if(r == r1) return w;

      //----------
      // The ragged tail, up to 7 bytes
      do{
        *w++ = *r++;
      }while(r < r1);

      return w;
    }

    /*
      Copy·reverse_byte_order - Copies a memory region while reversing byte order.
      - Reads in reverse order while writing in forward order.
      - Uses `__builtin_bswap64` for efficient 64-bit swaps.
      - Returns the updated write pointer.
    */
    void *Copy·reverse_byte_order(void *read0 ,void *read1 ,void *write0){

      uint8_t *r = (uint8_t *)read1; // Start from the last byte
      uint8_t *r0 = (uint8_t *)read0;
      uint8_t *w = (uint8_t *)write0;

      //----------
      // The potentially unaligned initial part (align read pointer).
      if( (uintptr_t)r & 0x7 ){

        // ANDing with `~0x7` moves it downward to the nearest lower alignment.
        uint8_t *r10 = (uint8_t *)((uintptr_t)r & ~(uintptr_t)0x7);

        // If the read interval is very small
        if(r10 < r0){
          while(r > r0){
            *w++ = *--r;
          }
          return w;
        }

        // Copy down to alignment boundary
        do{
          *w++ = *--r;
        }while(r > r10);
      }
      // r is now aligned, and *r has been copied

      //----------
      // The bulk copy part
      uint8_t *r01 = (uint8_t *)( ((uintptr_t)r0 + (uintptr_t)0x7) & ~(uintptr_t)0x7);

      while(r > r01){
        r -= 8;
        *(uint64_t *)w = __builtin_bswap64(*(uint64_t *)r);
        w += 8;
      }

      // If r0 was aligned then r01 == r0 and we are done
      if(r < r0) return w;

      //----------
      // The ragged tail, up to 7 bytes
      do{
        *w++ = *--r;
      }while(r >= r0);

      return w;
    }

  #endif // LOCAL

#endif // IMPLEMENTATION
