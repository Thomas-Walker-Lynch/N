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

  #define extentof(x) (sizeof(x)-1)

  typedef struct{
    void *read0; 
    size_t read_size;
    void *write0;
    size_t write_size;
  } Copy·it;

  // returned from the `step_X` functions
  typedef enum{
     Copy·Step·perfect_fit = 0
    ,Copy·Step·argument_guard 
    ,Copy·Step·read_surplus
    ,Copy·Step·read_surplus_write_gap
    ,Copy·Step·write_availableCopy·Status·
    ,Copy·Step·write_gap;
  } Copy·Status;

  typedef enum{
    Copy·WFIt·Status·valid = 0
    ,Copy·WFIt·Status·null_read
    ,Copy·WFIt·Status·zero_size_read
    ,Copy·WFIt·Status·null_write
    ,Copy·WFIt·Status·zero_size_write
    ,Copy·WFIt·Status·overlap
  } Copy·WFIt·Status;

  // function dictionary
  typedef struct{
    void *bytes(void *read0 ,void *read1 ,void *write0);
    void *reverse_byte_order(void *read0 ,void *read1 ,void *write0);
    Copy·WFIt·Status Copy·wellformed_it(Copy·it *it ,Copy·WFIt·Mode mode);
  } Copy·M;

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef Copy·IMPLEMENTATION

  #ifdef Copy·DEBUG
    #include <stdio.h> // Only for debug prints, not used in production.
  #endif


  // this part goes into Copylib.a
  // yes this is empty, so there is no Copylib.a
  #ifndef LOCAL
  #endif 

  #ifdef LOCAL

    // Interval predicates.
    // Intervals in Copy have an exclusive upper bound

    Local bool Copy·in_pt_interval(void *pt, void *pt0 ,void *pt1){
      return pt >= pt0 && pt < pt1;
    }
    Local bool Copy·in_size_interval(void *pt, void *pt0 ,size_t s){
      return Copy·in_pt_interval(pt ,pt0 ,pt0 + s);
    }

    // interval 0 contains interval 1, overlap on boundaries allowed.
    Local bool Copy·contains_pt_interval(
      void *pt00 ,void *pt01 ,void *pt10 ,void *pt11
    ){
     return
       pt10 >= pt00 && pt11 <= pt01
       ;
    }

    // Possible cases of overlap
    // 1. interval 0 to the left of interval 1
    // 2. interval 0 to the right of interval 1
    // 3. interval 0 wholly contained in interval 1
    // 4. interval 0 wholly contains interval 1
    Local bool Copy·overlap_pt_interval(void *pt00 ,void *pt01, void *pt10 ,void *pt11){
      void *pt01_inclusive = pt01 - 1;
      void *pt11_inclusive = pt11 - 1;
      return 
        Copy·in_pt_interval(pt10 ,pt00 ,pt01) // #1, #4
        ||
        Copy·in_pt_interval(pt00 ,pt10 ,pt11) // #2, #3
        ;
    }
    Local bool Copy·overlap_size_interval(void *pt00 ,size_t s0, void *pt10 ,size_t s1){
      return Copy·overlap_pt_interval(pt00 ,pt00 + s0 ,pt10 ,pt10 + s1);
    }

    Local Copy·WFIt·Status Copy·wellformed_it(Copy·it *it){
      char *this_name = "Copy·wellformed_it";
      Copy·WFIt·Status status = Copy·WFIt·Status·valid;

      if(it->read0 == NULL){
        fprintf( stderr ,"%s: NULL read pointer\n" ,this_name );
        status |= Copy·WFIt·Status·null_read;
      }

      if(it->write0 == NULL){
        fprintf( stderr ,"%s: NULL write pointer\n" ,this_name );
        status |= Copy·WFIt·Status·null_write;
      }

      if(it->read_size == 0){
        fprintf( stderr ,"%s: Zero-sized read buffer\n" ,this_name );
        status |= Copy·WFIt·Status·zero_read_buffer;
      }

      if(it->write_size == 0){
        fprintf( stderr ,"%s: Zero-sized write buffer\n" ,this_name );
        status |= Copy·WFIt·Status·zero_write_buffer;
      }

      if( Copy·overlap_size_interval(it->read0 ,it->read_size ,it->write0 ,it->write_size) ){
        fprintf( stderr ,"%s: Read and write buffers overlap!\n" ,this_name );
        status |= Copy·WFIt·Status·overlap;
      }

      return status;
    }

    /*
      Identity function. read interval values are copied without modification of value
      or order to the write allocation.
      - Aligns reads for performance.
      - Writes are assumed to be buffered and do not require alignment.
      - Returns the updated write pointer.
      - See doc 'Copy.org' for more details.
    */
    Local void *Copy·identity(void *read0 ,void *read1 ,void *write0){

      uint8_t *r = (uint8_t *)read0;
      uint8_t *r1 = (uint8_t *)read1;
      uint8_t *w = (uint8_t *)write0;

      //----------
      // The potentially unaligned initial part (align read pointer).
      if( (uintptr_t)r & 0x7 ){

        // at this point  r == r0, the lower bound of the read interval
        // r0 | `0x7` adds at most six bytes to r.
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
    Local Copy·Status Copy·step(Copy·it *it){
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
        if(ws == rs) return Copy·Step·perfect_fit;
        return Copy·Step·write_available;;
      }

      // ws < rs
      Copy·bytes(r ,r + ws ,w);
      it->read0 += ws;
      it->read_size -= ws;
      it->write_size = 0;
      it->write0 += ws;
      return Copy·Step·read_surplus;
   }

    /* 
       Read buffer is read from top down. Start with the largest address
       just above the read buffer. Continue into lower addresses.

       write buffer is written from bottom up. Start with the lowest address,
       continue into higher addresses.
    */
    Local Copy·Status Copy·step_reverse_order(Copy·it *it){
      // How many bytes remain to be read/written
      if( it->read_size  == 0) return Copy·Step·complete;
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
        if(it->write_size == 0) return Copy·Step·perfect_fit;
        return Copy·Step·write_available;
      }

      // ws < rs
      uint8_t *r0 = r1 - ws;
      Copy·bytes_reverse_order(r0, r1, w0);
      it->read0 -= ws;
      it->read_size -= ws;
      it->write_size = 0;
      it->write0 += ws;
      return Copy·Step·read_surplus;
    }

    /*
      Read bytes, write hex pairs.
      Read and write are low address to high address.
      Each read byte value -> 2 write allocation bytes
    */
    Local Copy·Status Copy·step_write_hex(Copy·it *it){

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

        if(it->write_size == 0) return Copy·Step·perfect_fit;
        if(it->write_size == 1) return Copy·Step·write_gap;
        return Copy·Step·write_available;
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

      if(it->write_size == 1) return Copy·Step·read_surplus_write_gap;
      return Copy·Step·read_surplus;
    }

    /*
      Read hex pairs, write bytes.
      Read is low address to high address.
      Write is low address to high address.
      Each read hex pair -> 1 write byte.
    */
    Local Copy·Status Copy·step_read_hex(Copy·it *it){
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

        if(it->write_size == 0) return Copy·Step·perfect_fit;
        return Copy·Step·write_available;
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

      return Copy·Step·read_surplus;
    }


  #endif // LOCAL


#endif // IMPLEMENTATION
