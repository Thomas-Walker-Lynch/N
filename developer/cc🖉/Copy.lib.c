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

  typedef struct{
    void *read0; 
    size_t read_size;
    void *write0;
    size_t write_size;
  } Copy·it;

  typedef enum{
    Copy·Status·perfect_fit = 0
    ,Copy·Status·argument_guard
    ,Copy·Status·read_surplus
    ,Copy·Status·read_surplus_write_gap
    ,Copy·Status·write_available
    ,Copy·Status·write_gap // write allocation has a terminal gap
  } Copy·Status;

  typedef struct{
    void *region(void *read0 ,void *read1 ,void *write0);
    void *reverse_byte_order(void *read0 ,void *read1 ,void *write0);
  } Copy·M;


#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef Copy·IMPLEMENTATION

  // this part goes into Nlib.a
  #ifndef LOCAL
  #endif 

  #ifdef LOCAL




      #ifdef Copy·DEBUG
        #include <stdio.h> // Only for debug prints, not used in production.

typedef enum{
   Copy·StatusWFIt·none                  = 0x00
  ,Copy·StatusWFIt·null_read             = 0x01
  ,Copy·StatusWFIt·null_write            = 0x02
  ,Copy·StatusWFIt·zero_read_size        = 0x04
  ,Copy·StatusWFIt·zero_write_size       = 0x08
  ,Copy·StatusWFIt·write_too_small_hex   = 0x10
  ,Copy·StatusWFIt·read_too_small_hex    = 0x20
  ,Copy·StatusWFIt·read_larger_than_write = 0x40
  ,Copy·StatusWFIt·overlapping_buffers   = 0x80
} Copy·StatusWFIt;

typedef enum{
   Copy·ModeWFIt·none       = 0x00
  ,Copy·ModeWFIt·bytes      = 0x01
  ,Copy·ModeWFIt·reverse    = 0x02
  ,Copy·ModeWFIt·write_hex  = 0x03
  ,Copy·ModeWFIt·from_hex   = 0x04
} Copy·ModeWFIt;




#endif


    /*
      Copy·region - Copies a memory region while preserving byte order.
      - Aligns reads for performance.
      - Writes are assumed to be buffered and do not require alignment.
      - Returns the updated write pointer.
    */
    Local void *Copy·bytes(void *read0 ,void *read1 ,void *write0){

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
      - Reads from read1 down
      - writes from write0 up
      - Uses `__builtin_bswap64` for efficient 64-bit swaps.
      - Returns the updated write pointer.
    */
    Local void *Copy·bytes_reverse_order(void *read0 ,void *read1 ,void *write0){

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

    /* 
       Read buffer is read from the lowest address, working toward higher addresses.

       Write buffer is written from the lowest address, working to higher addresses.

       To force data to be left in the read buffer, or for capacity to be left in the
       write buffer, reduce sizes.
    */
    Local Copy·Status Copy·step(
      Copy·it *it
    ){
      uint8_t *r = (uint8_t *)it->read0;
      uint8_t *w = (uint8_t *)it->write0;

      size_t rs = it->read_size;
      size_t ws = it->write_size;

      if(ws >= rs){
        Copy·bytes(r ,r + rs ,w);
        it->read0 += rs;
        it->read_size = 0;
        it->write0 += rs;
        it->write_size -= rs;
        if(ws == rs) return Copy·Status·perfect_fit;
        return Copy·Status·write_available;;
      }

      // ws < rs
      Copy·bytes(r ,r + ws ,w);
      it->read0 += ws;
      it->read_size -= ws;
      it->write_size = 0;
      it->write0 += ws;
      return Copy·Status·read_surplus;
   }

    /* 
       Read buffer is read from top down. Start with the largest address
       just above the read buffer. Continue into lower addresses.

       write buffer is written from bottom up. Start with the lowest address,
       continue into higher addresses.
    */
    Local Copy·Status Copy·step_reverse_order(Copy·it *it){
      // How many bytes remain to be read/written
      if( it->read_size  == 0) return Copy·Status·complete;
      size_t rs = it->read_size;
      uint8_t *r1   = (uint8_t *)it->read0 + rs;
      size_t ws = it->write_size;
      uint8_t *w0 = (uint8_t *)it->write0;

      if(ws >= rs){
        uint8_t *r0 = (uint8_t *)it->read0;
        Copy·bytes_reverse_order(r0, r1, w0);
        it->read_size = 0;
        it->write0 += rs;
        it->write_size -= rs;
        if(it->write_size == 0) return Copy·Status·perfect_fit;
        return Copy·Status·write_available;
      }

      // ws < rs
      uint8_t *r0 = r1 - ws;
      Copy·bytes_reverse_order(r0, r1, w0);
      it->read0 -= ws;
      it->read_size -= ws;
      it->write_size = 0;
      it->write0 += ws;
      return Copy·Status·read_surplus;
    }

    /*
      Read bytes, write hex pairs.
      Read and write are low address to high address.
      Each read byte value -> 2 write allocation bytes
    */
    Local Copy·Status Copy·step_write_hex(
      Copy·it *it
    ){

      uint8_t *r = (uint8_t *)it->read0;
      size_t rs = it->read_size;

      uint8_t *w = (uint8_t *)it->write0;
      size_t  ws = it->write_size & ~1; // even number write_size 
      size_t ews = it->write_size >> 1; // effective write size 

      // If ews >= rs, read bytes all coped
      if(ews >= rs){
        size_t ers = it->read_size << 1; // effective read size
        it->write0 += ers;
        it->write_size -= ers;
        while(rs--){
          *(uint16_t *)w = Copy·byte_to_hex(*r++);
          w += 2;
        }
        it->read0 = r;
        it->read_size = 0;

        if(it->write_size == 0) return Copy·Status·perfect_fit;
        if(it->write_size == 1) return Copy·Status·write_gap;
        return Copy·Status·write_available;
      }

      // ews < rs, write allocation all used, read bytes surplus
      it->read0 += ews;
      it->read_size -= ews;
      while(ews--){
        *(uint16_t *)w = Copy·byte_to_hex(*r++);
        w += 2;
      }
      it->write0 = w;
      it->write_size -= ws;

      if(it->write_size == 1) return Copy·Status·read_surplus_write_gap;
      return Copy·Status·read_surplus;
    }

    /*
      Read hex pairs, write bytes.
      Read is low address to high address.
      Write is low address to high address.
      Each read hex pair -> 1 write byte.
    */
    Local Copy·Status Copy·step_from_hex(
      Copy·it *it
    ){
      uint8_t *r = (uint8_t *)it->read0;
      size_t rs = it->read_size & ~1; // Must be even for hex pairs.
      size_t ers = rs >> 1; // Effective read size: half the number of bytes.

      uint8_t *w = (uint8_t *)it->write0;
      size_t ws = it->write_size; // Write size already in bytes.

      // If ws >= ers, all hex values are processed
      if(ws >= ers){
        while(ers--){
          *w++ = Copy·hex_to_byte(*(uint16_t *)r);
          r += 2;
        }

        it->read0 = r;
        it->read_size -= rs;
        it->write0 = w;
        it->write_size -= rs >> 1; // Each byte consumes two hex chars.

        if(it->write_size == 0) return Copy·Status·perfect_fit;
        return Copy·Status·write_available;
      }

      // ws < ers, read allocation surplus
      while(ws--){
        *w++ = Copy·hex_to_byte(*(uint16_t *)r);
        r += 2;
      }

      it->read0 = r;
      it->read_size -= ws << 1; // Each write byte consumes two hex chars.
      it->write0 = w;
      it->write_size = 0;

      return Copy·Status·read_surplus;
    }


  #endif // LOCAL


#endif // IMPLEMENTATION
