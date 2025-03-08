/*
  Core - core memory operations.

  'ATP'  Used in comments. 'At This Point' in the code. 

  'Area' refers to an area (in contrast to a point) in the address space.

   A non-exist array is said to be non-existent.
   An array with zero elements has 'length == 0' or is 'empty'.

   In contrast, an allocation does not exist if it has zero bytes.


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

  #define extentof(x)(sizeof(x) - 1)
  #define extent_t size_t

  typedef struct Core·Area;

  typedef uint8_t (*ReadFn8)(Area * ,uint8_t *);
  typedef uint64_t (*ReadFn64)(Area * ,uint64_t *);

  typedef struct {
    Core·Area *read;
    Core·Area *write;
    ReadFn8 read_fn_8;
    ReadFn64 read_fn_64;
    uint8_t *read_pt;
    uint8_t *write_pt;
    Core·Step·Status status;
  } Core·TableauFace;

  extern __thread Core·TableauFace tl; 

  typedef enum{
    Core·Step·perfect_fit = 0
    ,Core·Step·argument_guard // something wrong with the arguments to step
    ,Core·Step·read_surplus
    ,Core·Step·read_surplus_write_gap
    ,Core·Step·write_available
    ,Core·Step·write_gap
  } Core·Step·Status;

  typedef Core·Step·Fn (*Core·Step·Fn)();

  typdef struct Core·Area;

  typedef struct{

    void *offset_8(void *p ,size_t Δ);
    void *offset_64(void *p ,size_t Δ);
    bool is_aligned_64(void *p);
    void *floor_64(void *p);
    void *ceiling_64(void *p);

    void Core·Area·init_pe(Core·Area *area ,void *position ,extent_t extent);
    void Core·Area·init_pp(Core·Area *area ,void *position_left ,void *position_right);
    void Core·Area·set_position(Core·Area *area ,uint8_t *new_position);
    void Core·Area·set_position_left(Core·Area *area ,uint8_t *new_position);
    extent_t Core·Area·set_extent(Core·Area *area);
    void Core·Area·set_position_right(Core·Area *area ,uint8_t *new_position_right);

    uint8_t *Core·Area·position(Core·Area *area);
    uint8_t *Core·Area·position_left(Core·Area *area);
    uint8_t *Core·Area·position_right(Core·Area *area);
    extent_t Core·Area·extent(Core·Area *area);
    uint8_t Core·Area·length_Kung(Core·Area *area); // saturates at 3

    bool Core·Area·encloses_pt(uint8_t *pt ,Core·Area *area);
    bool Core·Area·encloses_pt_strictly(uint8_t *pt ,Core·Area *area);
    bool Core·Area·encloses_area(Core·Area *outer ,Core·Area *inner);
    bool Core·Area·encloses_area_strictly(Core·Area *outer ,Core·Area *inner);
    bool Core·Area·overlap(Core·Area *a ,Core·Area *b);
    bool Core·Area·nonexistent(Core·Area *area);
    void Area·largest_aligned_64(Core·Area *outer ,Core·Area *inner_64);
    uint8_t *Area·complement(Core·Area *area ,uint8_t *r);


    Core·Step·Status Core·step(Core·Step·Fn fn ,Core·TableauFace *ap);
    Core·Step·Fn Step·copy_8;
    Core·Step·Fn Step·copy_16;
    Core·Step·Fn Step·write_hex;
    Core·Step·Fn Step·read_hex;

  } Core·M;

  Core·M m; // initialized in the implementation;


#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef Core·IMPLEMENTATION

  // declarations available only to IMPLEMENTATION 

  #ifdef Core·DEBUG
    #include <stdio.h>
  #endif

  typedef struct{
    uint8_t *position;
    extent_t extent;
  } Core·Area

  typedef struct {
    union {
      struct {
        ReadFn8 read_fn;
      } copy_8;
      struct {
        ReadFn64 read_fn;
        Area *area_64;
      } copy_64;
      struct {
        uint8_t (*hex_to_byte)(uint16_t);
      } read_hex;
      struct {
        uint16_t (*byte_to_hex)(uint8_t);
      } write_hex;
    };
  } Core·TableauLocal;

  // this part goes into Copylib.a
  // yes this is empty ,so there is no Copylib.a
  #ifndef LOCAL
    __thread Core·TableauFace tf;
    __thread Core·TableauLocal tl;
  #endif 

  #ifdef LOCAL
    extern __thread Core·TableauLocal tl; 

    //----------------------------------------
    // Position/Pointer/Address whatever you want to call it

    // point to the next uint64_t in an array of uint64_t
    // the increment can be negative
    Local void *Core·offset_8(void *p ,size_t Δ){
      return (void *)((uint8_t *)p) + Δ;
    }
    Local void *Core·offset_64(void *p ,size_t Δ){
      return (void *)((uint64_t *)p) + Δ;
    }

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


    //----------------------------------------
    // Area

    Local void Core·Area·set_position(Core·Area *area ,void *new_position){
      area->position = new_position;
    }
    Local void Core·Area·set_position_left(Core·Area *area ,void *new_position){
      area->position = new_position;
    }
    Local extent_t Core·Area·set_extent(Core·Area *area ,exent_t extent){
      return area->extent = extent;
    }
    Local void Core·Area·set_position_right(Core·Area *area ,void *new_position_right){
      Core·Area·set_extent(new_position_right - area->position);
    }
    Local void Core·Area·init_pe(Core·Area *area ,uint8_t *position ,extent_t extent){
      Core·Area·set_position(position);
      Core·Area·set_extent(extent);
    }
    Local void Core·Area·init_pp(Core·Area *area ,void *position_left ,void *position_right){
      Core·Area·set_position_left(position_left);
      Core·Area·set_position_right(position_right);
    }

    Local uint8_t *Core·Area·position(Core·Area *area){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·position:: given NULL area);
        return NULL;
      }
      #endif
      return area->position;
    }
    Local uint8_t *Core·Area·position_left(Core·Area *area){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·position_left:: given NULL area);
        return NULL;
      }
      #endif
      return area->position;
    }
    Local uint8_t *Core·Area·position_right(Core·Area *area){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·position_right:: given NULL area);
        return NULL;
      }
      #endif
      return area->position + area->extent;
    }
    Local extent_t Core·Area·extent(Core·Area *area){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·extent:: given NULL area);
        return 0;
      }
      #endif
      return area->extent;
    }
    Local uint8_t Core·Area·length_Kung(Core·Area *area){
      if(!Core·Area·position_left(area)) return 0;
      if(Core·Area·extent(area) >= 2) return 3;
      return Core·Area·extent(area) + 1;
    }

    Local bool Core·Area·encloses_pt(Core·Area *area ,uint8_t *pt){
      return 
        (pt >= Core·Area·position_left(area)) 
        && (pt <= Core·Area·position_right(area));
    }
    Local bool Core·Area·encloses_pt_strictly(Core·Area *area ,uint8_t *pt){
      return 
        (pt > Core·Area·position_left(area)) 
        && (pt < Core·Area·position_right(area));
    }
    Local bool Core·Area·encloses_area(Core·Area *outer ,Core·Area *inner){
      return 
        (Core·Area·position_left(inner) >= Core·Area·position_left(outer)) 
        && (Core·Area·position_right(inner) <= Core·Area·position_right(outer));
    }
    Local bool Core·Area·encloses_area_strictly(Core·Area *outer ,Core·Area *inner){
      return 
        (Core·Area·position_left(inner) > Core·Area·position_left(outer)) 
        && (Core·Area·position_right(inner) < Core·Area·position_right(outer));
    }

    // Possible cases of overlap ,including just touching
    // 1. interval 0 to the right of interval 1 ,just touching p00 == p11
    // 2. interval 0 to the left of interval 1 ,just touching p01 == p10
    // 3. interval 0 wholly contained in interval 1
    // 4. interval 0 wholly contains interval 1
    Local bool Core·Area·overlap(Core·Area *area0 ,Core·Area *area1){
      return 
        Core·Area·position_right(area0) >= (Core·Area·position_left(area1)
        && Core·Area·position_left(area0) <= Core·Area·position_right(area1);
    }

    Local bool Core·Area·nonexistent(Core·Area *area){
      return !!Core·Area·position_left(area);
    }

     // find the largest contained interval aligned on 64 bit boundaries
    static void Core·Area·largest_aligned_64(Core·Area *outer ,Core·Area *inner_64){
      uintptr_t p0 = (uintptr_t)Core·Area·position_left(outer);
      uintptr_t p1 = (uintptr_t)Core·Area·position_right(outer);

      uint8_t *p0_64 = (uint8_t *)( (p0 + 0x7) & ~(uintptr_t)0x7 );
      uint8_t *p1_64 = (uint8_t *)( (p1 - 0x7) & ~(uintptr_t)0x7 );

      if(p1_64 < p0_64){
        Core·Area·set_position(inner_64 ,NULL);
      }else{
        Core·Area·init_pp(inner_64 ,p0_64 ,p1_64);
      }
    }

    // complement against the extent of the area (reverse direction)
    // works for byte pointer
    // works for aligned word pointer
    Local uint8_t *Core·Area·complement(Core·Area *area ,uint8_t *r){
      return Core·Area·position_left(area) + (Core·Area·position_right(area) - r);
    }

    //----------------------------------------
    // step-able functions

    Local uint8_t Core·Area·read_8_fwd(Core·Area *area ,void *r){
      #ifdef Core·Debug
      if(!area || !r){
        fprintf(stderr,"Core·Area·read_8_fwd:: area r: %p %p\n" ,area ,r);
        return 0;
      }
      if( !Core·Area·enclose_pt(area ,r) ){
        fprintf(stderr,"Core·Area·read_8_fwd:: out of interval read\n");
      }
      #endif
      return *(uint8_t *)r;
    }

    // Given a pointer to the least address byte of a uint64_t, return the value
    Local uint64_t Core·Area·read_64_fwd(Core·Area *area ,void *r){
      #ifdef Core·Debug
      if(!area || !r){
        fprintf(stderr,"Core·Area·read_8_fwd:: area r: %p %p\n" ,area ,r);
        return 0;
      }
      if(!Core·Area·enclose_pt(area ,r) ){
        fprintf(stderr,"Core·Area·read_8_fwd:: out of interval read\n");
      }
      #endif
      return *(uint64_t *)r;
    }

    Local uint8_t Core·Area·read_8_rev(Core·Area *area ,uint8_t *r){
      #ifdef Core·Debug
      if(!area || !r){
        fprintf(stderr,"Core·Area·read_8_fwd:: area r: %p %p\n" ,area ,r);
        return 0;
      }
      if(!Core·Area·enclose_pt(area ,r) ){
        fprintf(stderr,"Core·Area·read_8_rev:: out of interval read\n");
      }
      #endif
      return *(Core·complement(area ,r));
    }

    Local uint64_t Core·Area·read_64_rev(Core·Area *area_64 ,uint8_t *r){
      #ifdef Core·Debug
      if(!area || !r){
        fprintf(stderr,"Core·Area·read_8_fwd:: area r: %p %p\n" ,area ,r);
        return 0;
      }
      if(!Core·Area·enclose_pt(area ,r) ){
        fprintf(stderr,"Core·Area·read_64_rev:: out of interval read\n");
      }
      #endif
      return __builtin_bswap64( *(uint64_t *)Core·floor_64(Core·complement(area_64 ,r)) );
    }

    //----------------------------------------
    // Step

    // Step function using trampoline execution model
    Local Core·Step·Status Core·step(Core·Step·Fn fn){
      #ifdef Core·Debug
      if(!fn || !ap){
        fprintf(stderr,"Core·step:: fn ap: %p %p\n" ,fn ,ap);
        return Core·Step·argument_guard
      }
      if(
         fn != Core·copy_8_fwd
         && fn != Core·copy_8_rev
         && fn != Core·copy_64_fwd
         && fn != Core·copy_64_rev
      ){
        fprintf(stderr,"Core·step:: unrecognized step-able function\n");
        return Core·Step·argument_guard
      ) 
      #endif

      while(fn) fn = fn();
      return tableau->status;
    }

    //----------------------------------------
    // step copy_8 

    Core·Step·Fn Core·Step·copy_8;;
    Core·Step·Fn Core·Step·Copy8·bulk;

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
        *w = Core·tableau.copy_8.read_fn(Core·tableau.ap->read0 ,r1 ,r);
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
        *w = Core·tableau.copy_8.read_fn(Core·tableau.ap->read0 ,r1 ,r);
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
        *w = Core·tableau.copy_8.read_fn(Core·tableau.ap->read0 ,r1 ,r);
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

        // 64-bit copy function with updated TableauFace terminology
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
        Core·tableau.ap->read0 ,Core·tableau.ap->read0 + Core·tableau.ap->read_extent
        );
      Core·tableau.copy_64.r1_64 = Core·greatest_full_64(
        Core·tableau.ap->read0 ,Core·tableau.ap->read0 + Core·tableau.ap->read_extent
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
        *w++ = Core·tableau.copy_8.read_fn(Core·tableau.ap->read0 ,r0_64 ,r);
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
            Core·tableau.copy_64.r0_64 ,Core·tableau.copy_64.r1_64 ,r64
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
        *w++ = Core·tableau.copy_8.read_fn(Core·tableau.ap->read0 ,r1 ,r);
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
        *(uint16_t *)w = Core·tableau.write_hex.byte_to_hex(*r);
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
        *(uint16_t *)w = Core·tableau.write_hex.byte_to_hex(*r);
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
        *(uint16_t *)w = Core·tableau.write_hex.byte_to_hex(*r);
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
        *w = Core·tableau.read_hex.hex_to_byte(*(uint16_t *)r);
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
        *w = Core·tableau.read_hex.hex_to_byte(*(uint16_t *)r);
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
        *w = Core·tableau.read_hex.hex_to_byte(*(uint16_t *)r);
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

    Core·M m = {
      .Area·init_pe = Core·Area·init_pe
      ,.Area·init_pp = Core·Area·init_pp
      ,.Area·position = Core·Area·position
      ,.Area·position_left = Core·Area·position_left
      ,.Area·position_right = Core·Area·position_right
      ,.Area·set_position = Core·Area·set_position
      ,.Area·extent = Core·Area·extent
      ,.Area·set_extent = Core·Area·set_extent
      ,.Area·set_position = Core·Area·set_exent
      ,.Area·length_Kung = Core·Area·length_Kung

      .Area·encloses_pt = Core·Area·encloses_pt
      ,.Area·encloses_pt_strictly = Core·Area·encloses_pt_strictly
      ,.Area·encloses_area = Core·Area·encloses_area
      ,.Area·encloses_area_strictly = Core·Area·encloses_area_strictly
      ,.Area·overlap = Core·Area·overlap
      ,.Core·Area·nonexistent = Core·Area·nonexistent
      ,.Core·Area·length_Kung = Core·Area·length_Kung

      ,.is_aligned_64 = Core·is_aligned_64
      ,.floor_64 = Core·floor_64
      ,.ceiling_64 = Core·ceiling_64
      ,.greatest_full_64 = Core·greatest_full_64
      ,.least_full_64 = Core·least_full_64
      ,.offset_8 = Core·offset_8
      ,.offset_64 = Core·offset_64

      ,.Core·step = Core·step

      ,.copy_8 = Core·copy_8
      ,.copy_16 = Core·copy_16
      ,.Step·write_hex = Core·Step·write_hex
      ,.Step·read_hex = Core·Step·read_hex
    };

  #endif // LOCAL

#endif // IMPLEMENTATION
