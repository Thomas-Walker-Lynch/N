/*
  Core - core memory operations.

  'ATP'  'At This Point' in the code. Used in comments to state assertions.
   by definition an 'extent' is one less than a 'size'.

  'Area' refers to an area (in contrast to a point) in the address space.

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
    void *read0;  // read0 = NULL means no buffer or empty buffer
    extent_t read_extent;
    void *write0;  // write0 = NULL means no buffer or empty buffer.
    extent_t write_extent;
    bool reverse_byte_order;
  } Core·AreaPairinng;

  typedef enum{
    Core·AreaPairing·Status·valid = 0
    ,Core·AreaPairing·Status·null = 1
    ,Core·AreaPairing·Status·overlap = 2
    ,Core·AreaPairing·Status·empty_read_buffer = 4
    ,Core·AreaPairing·Status·empty_write_buffer = 8
  } Core·AreaPairing·Status;


  typedef enum{
    Core·Step·perfect_fit = 0
    ,Core·Step·argument_guard // something wrong with the arguments to step
    ,Core·Step·read_surplus
    ,Core·Step·read_surplus_write_gap
    ,Core·Step·write_available
    ,Core·Step·write_gap
  } Core·Step·Status;

  typedef Core·Step·Fn (*Core·Step·Fn)();

  typedef struct{

    // Area predicates
    bool Area·encloses_pt(void *pt ,void *pt0 ,extent_t e);
    bool Area·encloses_pt_strictly(void *pt ,void *pt0 ,extent_t e);
    bool Area·encloses_area(void *pt00 ,extent_t e0 ,void *pt10 ,extent_t e1);
    bool Area·encloses_area_strictly(void *pt00 ,extent_t e0 ,void *pt10 ,extent_t e1);
    bool Area·overlap(void *pt00 ,extent_t s0 ,void *pt10 ,extent_t e1);

    // 64 bit word operations
    bool is_aligned_64(void *p);
    void *floor_64(void *p);
    void *ceiling_64(void *p);
    uint64_t *greatest_full_64(void *p0 ,void *p1);
    uint64_t *least_full_64(void *p0 ,void *p1);
    void *inc_64(void *p ,size_t Δ);

    Core·AreaPairing·Status wellformed_it(Core·AreaPairing *ap);

    Core·Step·Status Core·step(Core·Step·Fn fn ,Core·AreaPairing *ap);
    Core·Step·Fn copy_8;
    Core·Step·Fn copy_16;
    Core·Step·Fn Step·write_hex;
    Core·Step·Fn Step·read_hex;

  } Core·M;

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef Core·IMPLEMENTATION

  // declarations available only to IMPLEMENTATION 

  #ifdef Core·DEBUG
    #include <stdio.h>
  #endif

  typedef uint8_t (*ReadFn8)(uint8_t *, uint8_t *, uint8_t *);
  typedef uint64_t (*ReadFn64)(uint64_t *, uint64_t *, uint64_t *);

  typedef struct {
    Core·Step·Status status;
    Core·AreaPairing *ap;
    struct {
      ReadFn8 read_fn;
    } copy_8;
    union {
      struct {
        ReadFn64 read_fn;
        uint64_t *r0_64;
        uint64_t *r1_64;
      } copy_64;
      struct {
        union {
          uint16_t (*byte_to_hex)(uint8_t);
          uint8_t (*hex_to_byte)(uint16_t);
        } convert;
      } hex;
    };
  } Core·Tableau;

  extern __thread Core·Tableau tableau

  // this part goes into Copylib.a
  // yes this is empty, so there is no Copylib.a
  #ifndef LOCAL
    __thread Core·Tableau tableau
  #endif 

  #ifdef LOCAL

    //----------------------------------------
    // Area predicates

    Local bool Core·Area·encloses_pt(void *pt ,void *pt0 ,extent_t e){
      return (pt >= pt0) && (pt <= pt0 + e); // Inclusive bounds
    }
    Local bool Core·Area·encloses_pt_strictly(void *pt ,void *pt0 ,extent_t e){
      return (pt > pt0) && (pt < pt0 + e); // Strictly inside
    }
    // Area 0 encloses Area 1
    Local bool Core·Area·encloses_area(
      void *pt0 ,extent_t e0 ,void *pt1 ,extent_t e1
    ){
      return (pt1 >= pt0) && (pt1 + e1 <= pt0 + e0);
    }
    // Area 0 strictly encloses Area 1
    Local bool Core·Area·encloses_area_strictly(
      void *pt0 ,extent_t e0 ,void *pt1 ,extent_t e1
    ){
      return (pt1 > pt0) && (pt1 + e1 < pt0 + e0);
    }

    // Possible cases of overlap, including just touching
    // 1. interval 0 to the right of interval 1, just touching p00 == p11
    // 2. interval 0 to the left of interval 1, just touching p01 == p10
    // 3. interval 0 wholly contained in interval 1
    // 4. interval 0 wholly contains interval 1
    Local bool Core·Area·overlap(void *pt0 ,extent_t e0 ,void *pt1 ,extent_t e1){
      return pt1 <= pt0 + e0 && pt0 <= pt1 + e1;
    }

    //----------------------------------------
    // support for aligned uint64_t in a world of bytes

    Local bool Core·is_aligned_64(void *p){
      return ((uintptr_t)p & 0x7) == 0;
    }
    // find the lowest address in an 8 byte aligned window
    // returns the byte pointer to the least address byte in the window
    Local void *Core·floor_64(void *p){
      return (uintptr_t)p & ~(uintptr_t)0x7;
    }

    // find the largest address in an 8 byte aligned window
    // returns the byte pointer to the greatest address byte in the window
    Local void *Core·ceiling_64(void *p){
      return (uintptr_t)p | 0x7;
    }

    // consider that a maximally sized interval of uint64_t is in a byte interval
    // this returns the inclusive upper bound address pointing to aligned uint64_t
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

    // consider that a maximally sized interval of uint64_t is in a byte interval
    // this returns the inclusive lower bound address pointing to aligned uint64_t
    // by contract, p1 must be >= p0
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

    // point to the next uint64_t in an array of uint64_t
    // the increment can be negative
    Local void *Core·inc_64(void *p ,size_t Δ){
      return (void *)((uint64_t *)p) + Δ;
    }

    Local uint8_t Core·read_8_fwd(void *r0 ,void *r1 ,void *r){
      return *(uint8_t *)r;
    }

    Local uint8_t Core·read_8_rev(void *r0 ,void *r1 ,void *r){
      return *((uint8_t *)r0 + ((uint8_t *)r1 - (uint8_t *)r));
    }

    // Given a pointer to the least address byte of a uint64_t, return the value
    Local uint64_t Core·read_64_fwd(void *r){
      return *(uint64_t *)r;
    }

    // Given a pointer to the greatest address byte of a uint64_t, return the value
    Local uint64_t Core·read_64_rev(void *r0 ,void *r1 ,void *r){
      return __builtin_bswap64(
        *(uint64_t *)Core·floor_64( (uint8_t *)r0 + ((uint8_t *)r1 - (uint8_t *)r) )
      );
    }

    //----------------------------------------
    // AreaPairing

    Local Core·AreaPairing·Status Core·AreaPairing·wellformed(Core·AreaPairing *ap){

      bool print = false;
      #ifdef Core·DEBUG
        print = true;
      #endif 
      
      char *this_name = "Core·AreaPairing·wellformed";
      Copy·WFIt·Status status = Copy·WFIt·Status·valid;

      if(it == NULL){
        if(print) fprintf( stderr ,"%s: NULL read pointer\n" ,this_name );
        return Core·AreaPairing·Status·null;
      }

      if(ap->read0 == NULL){
        if(print) fprintf( stderr ,"%s: empty read buffer\n" ,this_name );
        status |= Copy·WFIt·Status·empty_read_buffer;
      }
      if(ap->write0 == NULL){
        if(print) fprintf( stderr ,"%s: empty write buffer\n" ,this_name );
        status |= Copy·WFIt·Status·empty_write_buffer;
      }
      if( Copy·overlap_size_interval(ap->read0 ,ap->read_size ,ap->write0 ,ap->write_size) ){
        if(print) fprintf( stderr ,"%s: Read and write buffers overlap!\n" ,this_name );
        status |= Copy·WFIt·Status·overlap;
      }
      return status;

    }


    //----------------------------------------
    // Step

    // Function pointer type
    typedef Core·Step·Fn (*Core·Step·Fn)();

    // Step function using trampoline execution model
    Local Core·Step·Status Core·step(Core·Step·Fn fn ,Core·AreaPairing *ap){
      if(
         fn != Core·copy_64 && fn != Core·copy_8
         ||
         Core·AreaPairing·wellformed(it) != Core·AreaPairing·Status·valid
      ) 
        return Core·Step·argument_guard;

      Core·tableau.it = it;
      while(fn) fn = fn();
      return tableau->status;
    }

    //----------------------------------------
    // step copy_8 

    Core·Step·Fn Core·copy_8;;
    Core·Step·Fn Core·Copy8·bulk;

    Local Core·Step·Fn Core·copy_8(){

      // Assign the correct read function based on byte order
      if(Core·tableau.ap->reverse_byte_order)
        Core·tableau.copy_8.read_fn = Core·read_8_rev;
      else
        Core·tableau.copy_8.read_fn = Core·read_8_fwd;

      // Determine the appropriate case and dispatch
      if(Core·tableau.ap->read_extent == Core·tableau.ap->write_extent)
        return Core·Copy8·perfect_fit;

      if(Core·tableau.ap->read_extent > Core·tableau.ap->write_extent)
        return Core·Copy8·read_surplus;

      return Core·Copy8·write_available;
    }

    Local Core·Step·Fn Core·Copy8·perfect_fit(){
      uint8_t *r = (uint8_t *) Core·tableau.ap->read0;
      uint8_t *r1 = (uint8_t *) (r + Core·tableau.ap->read_extent);
      uint8_t *w = (uint8_t *) Core·tableau.ap->write0;

      do{
        *w = Core·tableau.copy_8.read_fn(Core·tableau.ap->read0, r1, r);
        if(r == r1) break;
        r++;
        w++;
      }while(true);

      Core·tableau.ap->read0 = NULL;  // Buffer exhausted
      Core·tableau.ap->write0 = NULL; // Buffer exhausted
      Core·tableau.status = Core·Step·perfect_fit;
      return NULL;
    }

    Local Core·Step·Fn Core·Copy8·read_surplus(){
      uint8_t *r = (uint8_t *) Core·tableau.ap->read0;
      uint8_t *r1 = (uint8_t *) (r + Core·tableau.ap->write_extent);
      uint8_t *w = (uint8_t *) Core·tableau.ap->write0;

      do{
        *w = Core·tableau.copy_8.read_fn(Core·tableau.ap->read0, r1, r);
        if(r == r1) break;
        r++;
        w++;
      }while(true);

      Core·tableau.ap->read0 = r; // Advance read pointer
      Core·tableau.ap->read_extent -= Core·tableau.ap->write_extent;
      Core·tableau.ap->write0 = NULL; // Write buffer exhausted
      Core·tableau.status = Core·Step·read_surplus;
      return NULL;
    }

    Local Core·Step·Fn Core·Copy8·write_available(){
      uint8_t *r = (uint8_t *) Core·tableau.ap->read0;
      uint8_t *r1 = (uint8_t *) (r + Core·tableau.ap->read_extent);
      uint8_t *w = (uint8_t *) Core·tableau.ap->write0;
      uint8_t *w1 = (uint8_t *) (w + Core·tableau.ap->write_extent);

      do{
        *w = Core·tableau.copy_8.read_fn(Core·tableau.ap->read0, r1, r);
        if(w == w1) break;
        r++;
        w++;
      }while(true);

      Core·tableau.ap->write0 = w; // Advance write pointer
      Core·tableau.ap->write_extent -= Core·tableau.ap->read_extent;
      Core·tableau.ap->read0 = NULL; // Read buffer exhausted
      Core·tableau.status = Core·Step·write_available;
      return NULL;
    }


    //----------------------------------------
    // step copy_64

        // 64-bit copy function with updated AreaPairing terminology
    Core·Step·Fn Core·copy_64;
    Core·Step·Fn Core·Copy64·leadin;
    Core·Step·Fn Core·Copy64·bulk;
    Core·Step·Fn Core·Copy64·tail;

    // Initialize the copy_64 process
    Local Core·Step·Fn Core·copy_64(){

      // Assign the correct read function based on byte order
      if(Core·tableau.ap->reverse_byte_order)
        Core·tableau.copy_64.read_fn = Core·read_64_rev;
      else
        Core·tableau.copy_64.read_fn = Core·read_64_fwd;

      // Determine aligned 64-bit word boundaries
      Core·tableau.copy_64.r0_64 = Core·least_full_64(
        Core·tableau.ap->read0, Core·tableau.ap->read0 + Core·tableau.ap->read_extent
        );
      Core·tableau.copy_64.r1_64 = Core·greatest_full_64(
        Core·tableau.ap->read0, Core·tableau.ap->read0 + Core·tableau.ap->read_extent
        );

      // Choose the correct function based on alignment
      if(Core·tableau.copy_64.r0_64 == NULL) return Core·Copy64·tail;
      if(Core·is_aligned_64(Core·tableau.ap->read0)) return Core·Copy64·bulk;
      return Core·Copy64·leadin;
    }

    // Lead-in byte copy (until alignment)
    Local Core·Step·Fn Core·Copy64·leadin(){
      uint8_t *r = (uint8_t *) Core·tableau.ap->read0;
      uint8_t *w = (uint8_t *) Core·tableau.ap->write0;
      uint8_t *r0_64 = (uint8_t *) Core·tableau.copy_64.r0_64;

      do{
        *w++ = Core·tableau.copy_8.read_fn(Core·tableau.ap->read0, r0_64, r);
        if(r == r0_64) break;
        r++;
      }while(1);

      Core·tableau.ap->read0 = r;
      Core·tableau.ap->write0 = w;

      return Core·Copy64·bulk;
    }

    // Bulk word copy
    Local Core·Step·Fn Core·Copy64·bulk(){
      uint64_t *r64 = (uint64_t *) Core·tableau.ap->read0;
      uint64_t *w64 = (uint64_t *) Core·tableau.ap->write0;
      uint64_t *r1_64 = Core·tableau.copy_64.r1_64;

      do{
        *w64++ = Core·tableau.copy_64.read_fn(
            Core·tableau.copy_64.r0_64, Core·tableau.copy_64.r1_64, r64
          );
        if(r64 == r1_64) break;
        r64++;
      }while(1);

      Core·tableau.ap->read0 = r64;
      Core·tableau.ap->write0 = w64;

      return Core·Copy64·tail;
    }

    // Tail byte copy (unaligned trailing bytes)
    Local Core·Step·Fn Core·Copy64·tail(){
      uint8_t *r = (uint8_t *) Core·tableau.ap->read0;
      uint8_t *w = (uint8_t *) Core·tableau.ap->write0;
      uint8_t *r1 = (uint8_t *) Core·tableau.copy_64.r1_64;

      do{
        *w++ = Core·tableau.copy_8.read_fn(Core·tableau.ap->read0, r1, r);
        if(r == r1) break;
        r++;
      }while(1);

      Core·tableau.ap->read0 = r;
      Core·tableau.ap->write0 = w;

      Core·tableau.status = Core·Step·perfect_fit;
      return NULL;
    }

    //----------------------------------------
    // step write hex

    // Forward Declarations
    Core·Step·Fn Core·Step·write_hex;
    Core·Step·Fn Core·Step·write_hex_bulk;
    Core·Step·Fn Core·Step·write_hex_read_surplus;
    Core·Step·Fn Core·Step·write_hex_write_available;

    Core·Step·Fn Core·Step·read_hex;
    Core·Step·Fn Core·Step·read_hex_bulk;
    Core·Step·Fn Core·Step·read_hex_read_surplus;
    Core·Step·Fn Core·Step·read_hex_write_available;

    // Hex Encoding: Initialize Step
    Local Core·Step·Fn Core·Step·write_hex(){
      if(Core·tableau.area_pairing->read_extent == (Core·tableau.area_pairing->write_extent >> 1)){
        return Core·Step·write_hex_bulk;
      }
      if(Core·tableau.area_pairing->read_extent > (Core·tableau.area_pairing->write_extent >> 1)){
        return Core·Step·write_hex_read_surplus;
      }
      return Core·Step·write_hex_write_available;
    }

    // Hex Encoding: Bulk Processing (Perfect Fit)
    Local Core·Step·Fn Core·Step·write_hex_bulk(){
      uint8_t *r = (uint8_t *)Core·tableau.area_pairing->read0;
      uint8_t *r1 = r + Core·tableau.area_pairing->read_extent;
      uint8_t *w = (uint8_t *)Core·tableau.area_pairing->write0;

      do {
        *(uint16_t *)w = Core·tableau.hex.convert.byte_to_hex(*r);
        if(r == r1) break;
        r++;
        w += 2;
      } while(1);

      Core·tableau.area_pairing->read0 = NULL;
      Core·tableau.area_pairing->write0 = NULL;
      Core·tableau.area_pairing->read_extent = 0;
      Core·tableau.area_pairing->write_extent = 0;
      Core·tableau.status = Core·Step·perfect_fit;
      return NULL;
    }

    // Hex Encoding: Read Surplus
    Local Core·Step·Fn Core·Step·write_hex_read_surplus(){
      uint8_t *r = (uint8_t *)Core·tableau.area_pairing->read0;
      uint8_t *w = (uint8_t *)Core·tableau.area_pairing->write0;
      size_t limit = Core·tableau.area_pairing->write_extent >> 1;
      uint8_t *r1 = r + limit;

      do {
        *(uint16_t *)w = Core·tableau.hex.convert.byte_to_hex(*r);
        if(r == r1) break;
        r++;
        w += 2;
      } while(1);

      Core·tableau.area_pairing->read0 = r + 1;
      Core·tableau.area_pairing->read_extent -= limit;
      Core·tableau.area_pairing->write0 = NULL;
      Core·tableau.area_pairing->write_extent = 0;
      Core·tableau.status = Core·Step·read_surplus;
      return NULL;
    }

    // Hex Encoding: Write Available
    Local Core·Step·Fn Core·Step·write_hex_write_available(){
      uint8_t *r = (uint8_t *)Core·tableau.area_pairing->read0;
      uint8_t *w = (uint8_t *)Core·tableau.area_pairing->write0;
      size_t limit = Core·tableau.area_pairing->read_extent;
      uint8_t *r1 = r + limit;

      do {
        *(uint16_t *)w = Core·tableau.hex.convert.byte_to_hex(*r);
        if(r == r1) break;
        r++;
        w += 2;
      } while(1);

      Core·tableau.area_pairing->read0 = NULL;
      Core·tableau.area_pairing->read_extent = 0;
      Core·tableau.area_pairing->write0 = w + 2;
      Core·tableau.area_pairing->write_extent -= limit << 1;
      Core·tableau.status = Core·Step·write_available;
      return NULL;
    }

    //----------------------------------------
    // step read hex

    Local Core·Step·Fn Core·Step·read_hex(){
      if((Core·tableau.area_pairing->read_extent >> 1) == Core·tableau.area_pairing->write_extent){
        return Core·Step·read_hex_bulk;
      }
      if((Core·tableau.area_pairing->read_extent >> 1) > Core·tableau.area_pairing->write_extent){
        return Core·Step·read_hex_read_surplus;
      }
      return Core·Step·read_hex_write_available;
    }

    // Hex Decoding: Bulk Processing (Perfect Fit)
    Local Core·Step·Fn Core·Step·read_hex_bulk(){
      uint8_t *r = (uint8_t *)Core·tableau.area_pairing->read0;
      uint8_t *r1 = r + Core·tableau.area_pairing->read_extent;
      uint8_t *w = (uint8_t *)Core·tableau.area_pairing->write0;

      do {
        *w = Core·tableau.hex.convert.hex_to_byte(*(uint16_t *)r);
        if(r == r1) break;
        r += 2;
        w++;
      } while(1);

      Core·tableau.area_pairing->read0 = NULL;
      Core·tableau.area_pairing->write0 = NULL;
      Core·tableau.area_pairing->read_extent = 0;
      Core·tableau.area_pairing->write_extent = 0;
      Core·tableau.status = Core·Step·perfect_fit;
      return NULL;
    }

    // Hex Decoding: Read Surplus
    Local Core·Step·Fn Core·Step·read_hex_read_surplus(){
      uint8_t *r = (uint8_t *)Core·tableau.area_pairing->read0;
      uint8_t *w = (uint8_t *)Core·tableau.area_pairing->write0;
      size_t limit = Core·tableau.area_pairing->write_extent;
      uint8_t *r1 = r + (limit << 1);

      do {
        *w = Core·tableau.hex.convert.hex_to_byte(*(uint16_t *)r);
        if(r == r1) break;
        r += 2;
        w++;
      } while(1);

      Core·tableau.area_pairing->read0 = r + 2;
      Core·tableau.area_pairing->read_extent -= limit << 1;
      Core·tableau.area_pairing->write0 = NULL;
      Core·tableau.area_pairing->write_extent = 0;
      Core·tableau.status = Core·Step·read_surplus;
      return NULL;
    }

    // Hex Decoding: Write Available
    Local Core·Step·Fn Core·Step·read_hex_write_available(){
      uint8_t *r = (uint8_t *)Core·tableau.area_pairing->read0;
      uint8_t *w = (uint8_t *)Core·tableau.area_pairing->write0;
      size_t limit = Core·tableau.area_pairing->read_extent >> 1;
      uint8_t *r1 = r + (limit << 1);

      do {
        *w = Core·tableau.hex.convert.hex_to_byte(*(uint16_t *)r);
        if(r == r1) break;
        r += 2;
        w++;
      } while(1);

      Core·tableau.area_pairing->read0 = NULL;
      Core·tableau.area_pairing->read_extent = 0;
      Core·tableau.area_pairing->write0 = w + 1;
      Core·tableau.area_pairing->write_extent -= limit;
      Core·tableau.status = Core·Step·write_available;
      return NULL;
    }

  #endif // LOCAL

#endif // IMPLEMENTATION
