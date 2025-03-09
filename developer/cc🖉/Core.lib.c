/*
  Core - core memory operations.

  'ATP'  Used in comments. 'At This Point' in the code. 

  'Area' refers to an area (in contrast to a point) in the address space.

   A non-exist array is said to be non-existent.
   An array with zero elements has 'length == 0' or is 'empty'.

   In contrast, an allocation does not exist if it has zero bytes.

   It is better to separate the tableau than have a union, and let the
   optimizer figure out the memory life times, and what can be reused.

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

  typedef enum{
    ,Core·Copy·Status·uninitialied = 0
    ,Core·Copy·Status·argument_guard = 1
    ,Core·Copy·Status·perfect_fit = 2
    ,Core·Copy·Status·read_surplus = 4
    ,Core·Copy·Status·read_surplus_write_gap = 8
    ,Core·Copy·Status·write_available = 16
    ,Core·Copy·Status·write_gap = 32
  }Core·Copy·Status;

  struct{
    struct{
      Core·Area *read;
      Core·Area *write;
      ReadFn8 read_fn_8;
      ReadFn64 read_fn_64;
      uint8_t *read_pt;
      uint8_t *write_pt;
      Core·Copy·Status status;
    }copy;
  } Core·TableauFace
  Core·TableauFace tf;

  typedef Core·Copy·Fn (*Core·Copy·Fn)();

  typedef struct{

    // void * pointer operations
    void *offset_8(void *p ,size_t Δ);
    void *offset_64(void *p ,size_t Δ);
    bool is_aligned_64(void *p);
    void *floor_64(void *p);
    void *ceiling_64(void *p);

    // initialize an area
    void Area·init_pe(Core·Area *area ,void *position ,extent_t extent);
    void Area·init_pp(Core·Area *area ,void *position_left ,void *position_right);
    void Area·set_position(Core·Area *area ,uint8_t *new_position);
    void Area·set_position_left(Core·Area *area ,uint8_t *new_position); // synonym
    extent_t Area·set_extent(Core·Area *area);
    void Area·set_position_right(Core·Area *area ,uint8_t *new_position_right);

    // read area properties
    bool Area·empty(Core·Area *area);
    uint8_t *Area·position(Core·Area *area);
    uint8_t *Area·position_left(Core·Area *area); // synonym
    uint8_t *Area·position_right(Core·Area *area);
    extent_t Area·extent(Core·Area *area);
    uint8_t Area·length_Kung(Core·Area *area); // saturates at 3

    // area relationships
    bool Area·encloses_pt(uint8_t *pt ,Core·Area *area);
    bool Area·encloses_pt_strictly(uint8_t *pt ,Core·Area *area);
    bool Area·encloses_area(Core·Area *outer ,Core·Area *inner);
    bool Area·encloses_area_strictly(Core·Area *outer ,Core·Area *inner);
    bool Area·overlap(Core·Area *a ,Core·Area *b);
    void Area·largest_aligned_64(Core·Area *outer ,Core·Area *inner_64);
    uint8_t *Area·complement(Core·Area *area ,uint8_t *r);

    // read from an area
    uint8_t Area·read_8_zero(Core·Area *area ,void *r);
    uint8_t Area·read_8_fwd(Core·Area *area ,void *r);
    uint8_t Area·read_8_rev(Core·Area *area ,uint8_t *r);

    uint64_t Area·read_64_zero(Core·Area *area ,void *r);
    uint64_t Area·read_64_fwd(Core·Area *area ,void *r);
    uint64_t Area·read_64_rev(Core·Area *area_64 ,uint8_t *r);

    // hex conversion
    uint16_t byte_to_hex(uint8_t byte);
    uint8_t hex_to_byte(uint16_t hex);

    // copy one area to another, possibly with a transformation
    Copy·Status Core·copy(Core·Copy·Fn fn ,Core·TableauFace *ap);
    Copy·Fn Copy·byte_by_byte;
    Copy·Fn Copy·by_word_64;
    Copy·Fn Copy·write_hex;
    Copy·Fn Copy·read_hex;

  } Core·M;
  Core·M m; // initialized in the implementation;


#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef Core·IMPLEMENTATION
  // declarations available to all of the IMPLEMENTATION go here
  //
    #ifdef Core·DEBUG
      #include <stdio.h>
    #endif

    typedef struct{
      uint8_t *position;
      extent_t extent;
    } Core·Area

    // I removed the unions, as they are debugging hazards, and also might confuse the optimizer
    typedef struct{
        struct{
          ReadFn8 read_fn;
        } byte_by_byte;
        struct{
          ReadFn64 read_fn;
          Area area_64;
        } copy_64;
        struct{
          uint8_t (*hex_to_byte)(uint16_t);
        } read_hex;
        struct{
          uint16_t (*byte_to_hex)(uint8_t);
        } write_hex;
    } Core·TableauLocal;

  // this part goes into Copylib.a
  #ifndef LOCAL
  #endif 

  #ifdef LOCAL

    //----------------------------------------
    // Position/Pointer/Address whatever you want to call it

    Local void *Core·offset_8(void *p ,size_t Δ){
      #ifdef Core·Debug
      if(!p){
        fprintf(stderr,"Core·offset_8:: given NULL `p'");
        return NULL;
      }
      #endif
      return (void *)((uint8_t *)p) + Δ;
    }

    Local void *Core·offset_64(void *p ,size_t Δ){
      #ifdef Core·Debug
      if(!p){
        fprintf(stderr,"Core·offset_64:: given NULL `p'");
        return NULL;
      }
      #endif
      return (void *)((uint64_t *)p) + Δ;
    }

    Local bool Core·is_aligned_64(void *p){
      #ifdef Core·Debug
      if(!p){
        fprintf(stderr,"Core·is_aligned_64:: given NULL `p'");
        return false;
      }
      #endif
      return ((uintptr_t)p & 0x7) == 0;
    }

    // find the lowest address in an 8 byte aligned window
    // returns the byte pointer to the least address byte in the window
    Local void *Core·floor_64(void *p){
      #ifdef Core·Debug
      if(!p){
        fprintf(stderr,"Core·floor_64:: given NULL `p'");
        return NULL;
      }
      #endif
      return (void *)((uintptr_t)p & ~(uintptr_t)0x7);
    }

    // find the largest address in an 8 byte aligned window
    // returns the byte pointer to the greatest address byte in the window
    Local void *Core·ceiling_64(void *p){
      #ifdef Core·Debug
      if(!p){
        fprintf(stderr,"Core·ceiling_64:: given NULL `p'");
        return NULL;
      }
      #endif
      return (void *)((uintptr_t)p | 0x7);
    }

    //----------------------------------------
    // Area

    // initialize an area

    Local void Core·Area·set_position(Core·Area *area ,void *new_position){
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

    // read area properties

    Local bool Core·Area·empty(Core·Area *area){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·empty:: given NULL area");
        return true;
      }
      #endif
      return area->position == NULL;
    }

    // Requesting a NULL position is a logical error, because a NULL position
    // means the Area is empty and has no position. Instead, use the `empty`
    // predicate.
    Local uint8_t *Core·Area·position(Core·Area *area){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·position:: given NULL area");
        return NULL;
      }
      if(!area->position){
        fprintf(stderr,"Core·Area·position:: request for position when it is NULL");
      }
      #endif
      return area->position;
    }


    Local uint8_t *Core·Area·position_right(Core·Area *area){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·position_right:: given NULL area");
        return NULL;
      }
      #endif
      return area->position + area->extent;
    }
    Local extent_t Core·Area·extent(Core·Area *area){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·extent:: given NULL area");
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
        Core·Area·position_right(area0) >= Core·Area·position_left(area1)
        && Core·Area·position_left(area0) <= Core·Area·position_right(area1);
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
    // read functions

    // consider instead using `copy_zero`
    Local uint8_t Core·Area·read_8_zero(Core·Area *area ,void *r){
      return 0;
    }
    Local uint64_t Core·Area·read_64_zero(Core·Area *area ,void *r){
      return 0;
    }

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
      return *(Core·complement(area ,r));
    }

    Local uint64_t Core·Area·read_64_rev(Core·Area *area_64 ,uint8_t *r){
      return __builtin_bswap64( *(uint64_t *)Core·floor_64(Core·complement(area_64 ,r)) );
    }

    //----------------------------------------
    // Copy

    // Copy function using trampoline execution model
    Local Core·Copy·Status Core·copy(Core·Copy·Fn fn){
      #ifdef Core·Debug
      if(!fn){
        fprintf(stderr,"Core·copy:: given null function");
        return Core·Copy·argument_guard
      }
      if(
         true
         && fn != Core·Copy·by_word_64
         && fn != Core·Copy·byte_by_byte
         && fn != Core·write_hex
         && fn != Core·read_hex
      ){
        fprintf(stderr,"Core·copy:: unrecognized copy function\n");
        return Core·Copy·argument_guard
      ) 
      #endif

      while(fn) fn = fn();
      return tableau->status;
    }

    //----------------------------------------
    // copy byte_by_byte 

    Core·Copy·Fn Core·Copy·byte_by_byte;
    Core·Copy·Fn Core·Copy·Copy8·bulk;

    Local Core·Copy·Fn Core·Copy·byte_by_byte(){
      if(Core·Area·extent(Core·tf.copy.read) == Core·Area·extent(Core·tf.copy.write))
        return Core·Copy8·perfect_fit;

      if(Core·Area·extent(Core·tf.copy.read) > Core·Area·extent(Core·tf.copy.write))
        return Core·Copy8·read_surplus;

      return Core·Copy8·write_available;
    }

    Core·Copy·Fn Core·Copy·byte_by_byte;
    Core·Copy·Fn Core·Copy·Copy8·bulk;

    Local Core·Copy·Fn Core·Copy·byte_by_byte(){
      if(Core·Area·extent(Core·tf.copy.read) == Core·Area·extent(Core·tf.copy.write)){
        return Core·Copy8·perfect_fit;
      }
      if(Core·Area·extent(Core·tf.copy.read) > Core·Area·extent(Core·tf.copy.write)){
        return Core·Copy8·read_surplus;
      }
      return Core·Copy8·write_available;
    }

    Local Core·Copy·Fn Core·Copy8·perfect_fit(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,r1 ,*r);
        if(*r == r1) break;
        (*r)++;
        (*w)++;
      }while(true);

      Core·tf.copy.status = Core·Copy·Status·perfect_fit;
      return NULL;
    }

    Local Core·Copy·Fn Core·Copy8·read_surplus(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,r1 ,*r);
        if(*r == r1) break;
        (*r)++;
        (*w)++;
      }while(true);

      Core·tf.copy.status = Core·Copy·Status·read_surplus;
      return NULL;
    }

    Local Core·Copy·Fn Core·Copy8·write_available(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;
      uint8_t *w1 = Core·Area·position_right(Core·tf.copy.write);

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,r1 ,*r);
        if(*w == w1) break;
        (*r)++;
        (*w)++;
      }while(true);

      Core·tf.copy.status = Core·Copy·Status·write_available;
      return NULL;
    }

    //----------------------------------------
    // copy copy_64

    //----------------------------------------
    // copy copy_64

    // 64-bit copy function with updated TableauFace terminology
    Core·Copy·Fn Core·Copy·by_word_64;
    Core·Copy·Fn Core·Copy64·leadin;
    Core·Copy·Fn Core·Copy64·bulk;
    Core·Copy·Fn Core·Copy64·tail;

    //----------------------------------------
    // copy copy_64

    // 64-bit copy function with updated TableauFace terminology
    Core·Copy·Fn Core·Copy·by_word_64;
    Core·Copy·Fn Core·Copy64·leadin;
    Core·Copy·Fn Core·Copy64·bulk;
    Core·Copy·Fn Core·Copy64·tail;

    // Initialize the copy_64 process
    Local Core·Copy·Fn Core·Copy·by_word_64(){
      // Determine the largest 64-bit aligned region within the read area
      Core·Area·largest_aligned_64(Core·tf.copy.read ,&Core·tl.copy_64.area_64);

      // Choose the correct function based on alignment
      if(Core·Area·empty(&Core·tl.copy_64.area_64)) return Core·Copy64·tail;
      if(Core·is_aligned_64(Core·Area·position(Core·tf.copy.read))) return Core·Copy64·bulk;
      return Core·Copy64·leadin;
    }

    // Lead-in byte copy (until alignment)
    Local Core·Copy·Fn Core·Copy64·leadin(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r0_64 = Core·Area·position(&Core·tl.copy_64.area_64);
      uint8_t **w = &Core·tf.copy.write_pt;

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,r0_64 ,*r);
        if(*r == r0_64) break;
        (*r)++;
        (*w)++;
      }while(true);

      return Core·Copy64·bulk;
    }

    // Bulk word copy
    Local Core·Copy·Fn Core·Copy64·bulk(){
      uint64_t **r = (uint64_t **)&Core·tf.copy.read_pt;
      uint64_t **w = (uint64_t **)&Core·tf.copy.write_pt;
      uint64_t *r1_64 = Core·Area·position_right(&Core·tl.copy_64.area_64);

      do{
        **w = Core·tf.copy.read_fn_64(Core·tf.copy.read ,r1_64 ,*r);
        if(*r == r1_64) break;
        (*r)++;
        (*w)++;
      }while(true);

      return Core·Copy64·tail;
    }

    // Tail byte copy (unaligned trailing bytes)
    Local Core·Copy·Fn Core·Copy64·tail(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(&Core·tl.copy_64.area_64);
      uint8_t **w = &Core·tf.copy.write_pt;

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,r1 ,*r);
        if(*r == r1) break;
        (*r)++;
        (*w)++;
      }while(true);

      Core·tf.copy.status = Core·Copy·Status·perfect_fit;
      return NULL;
    }

    //----------------------------------------
    // copy write hex

    Local uint16_t Core·byte_to_hex(uint8_t byte){
      static const char hex_digits[] = "0123456789ABCDEF";
      return 
          (hex_digits[byte >> 4] << 8) 
        | hex_digits[byte & 0x0F];
    }

    // Forward Declarations
    Core·Copy·Fn Core·Copy·write_hex;
    Core·Copy·Fn Core·Copy·write_hex_bulk;
    Core·Copy·Fn Core·Copy·write_hex_read_surplus;
    Core·Copy·Fn Core·Copy·write_hex_write_available;

    // Hex Encoding: Initialize Copy
    Local Core·Copy·Fn Core·Copy·write_hex(){
      if(Core·Area·extent(Core·tf.copy.read) == (Core·Area·extent(Core·tf.copy.write) >> 1)){
        return Core·Copy·write_hex_bulk;
      }
      if(Core·Area·extent(Core·tf.copy.read) > (Core·Area·extent(Core·tf.copy.write) >> 1)){
        return Core·Copy·write_hex_read_surplus;
      }
      return Core·Copy·write_hex_write_available;
    }

    // Hex Encoding: Bulk Processing (Perfect Fit)
    Local Core·Copy·Fn Core·Copy·write_hex_bulk(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;

      do {
        *(uint16_t *)*w = Core·tf.write_hex.byte_to_hex(**r);
        if(*r == r1) break;
        (*r)++;
        (*w) += 2;
      } while(true);

      Core·tf.copy.status = Core·Copy·Status·perfect_fit;
      return NULL;
    }

    // Hex Encoding: Read Surplus
    Local Core·Copy·Fn Core·Copy·write_hex_read_surplus(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.write);
      uint8_t **w = &Core·tf.copy.write_pt;

      do {
        *(uint16_t *)*w = Core·tf.write_hex.byte_to_hex(**r);
        if(*r == r1) break;
        (*r)++;
        (*w) += 2;
      } while(true);

      Core·tf.copy.status = Core·Copy·Status·read_surplus;
      return NULL;
    }

    // Hex Encoding: Write Available
    Local Core·Copy·Fn Core·Copy·write_hex_write_available(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;
      uint8_t *w1 = Core·Area·position_right(Core·tf.copy.write);

      do {
        *(uint16_t *)*w = Core·tf.write_hex.byte_to_hex(**r);
        if(*w == w1) break;
        (*r)++;
        (*w) += 2;
      } while(true);

      Core·tf.copy.status = Core·Copy·Status·write_available;
      return NULL;
    }

    //----------------------------------------
    // copy read hex

    Local uint8_t Core·hex_to_byte(uint16_t hex){
      uint8_t high = hex >> 8;
      uint8_t low = hex & 0xFF;

      high = 
          (high >= '0' && high <= '9') ? (high - '0')
        : (high >= 'A' && high <= 'F') ? (high - 'A' + 10)
        : (high >= 'a' && high <= 'f') ? (high - 'a' + 10)
        : 0;

      low = 
          (low >= '0' && low <= '9') ? (low - '0')
        : (low >= 'A' && low <= 'F') ? (low - 'A' + 10)
        : (low >= 'a' && low <= 'f') ? (low - 'a' + 10)
        : 0;

      return (high << 4) | low;
    }

    Core·Copy·Fn Core·Copy·read_hex;
    Core·Copy·Fn Core·Copy·read_hex_bulk;
    Core·Copy·Fn Core·Copy·read_hex_read_surplus;
    Core·Copy·Fn Core·Copy·read_hex_write_available;

    Local Core·Copy·Fn Core·Copy·read_hex(){
      if((Core·Area·extent(Core·tf.copy.read) >> 1) == Core·Area·extent(Core·tf.copy.write)){
        return Core·Copy·read_hex_bulk;
      }
      if((Core·Area·extent(Core·tf.copy.read) >> 1) > Core·Area·extent(Core·tf.copy.write)){
        return Core·Copy·read_hex_read_surplus;
      }
      return Core·Copy·read_hex_write_available;
    }

    // Hex Decoding: Bulk Processing (Perfect Fit)
    Local Core·Copy·Fn Core·Copy·read_hex_bulk(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;

      do {
        **w = Core·tf.read_hex.hex_to_byte(*(uint16_t *)*r);
        if(*r == r1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Copy·Status·perfect_fit;
      return NULL;
    }

    // Hex Decoding: Read Surplus
    Local Core·Copy·Fn Core·Copy·read_hex_read_surplus(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.write);
      uint8_t **w = &Core·tf.copy.write_pt;

      do {
        **w = Core·tf.read_hex.hex_to_byte(*(uint16_t *)*r);
        if(*r == r1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Copy·Status·read_surplus;
      return NULL;
    }

    // Hex Decoding: Write Available
    Local Core·Copy·Fn Core·Copy·read_hex_write_available(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;
      uint8_t *w1 = Core·Area·position_right(Core·tf.copy.write);

      do {
        **w = Core·tf.read_hex.hex_to_byte(*(uint16_t *)*r);
        if(*w == w1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Copy·Status·write_available;
      return NULL;
    }

    //----------------------------------------
    // Tableaux

    Core·TableauFace tf = {
       .Copy.read = NULL
       ,Copy.write = NULL
       ,Copy..read_fn_8 = read_8_fwd
       ,Copy..read_fn_64 = read_64_fwd
       ,Copy..read_pt = NULL
       ,Copy..write_pt = NULL
       ,Copy..status = Core·Copy·Status·uninitialied
    }

    //----------------------------------------
    // copy read hex

    Core·Copy·Fn Core·Copy·read_hex;
    Core·Copy·Fn Core·Copy·read_hex_bulk;
    Core·Copy·Fn Core·Copy·read_hex_read_surplus;
    Core·Copy·Fn Core·Copy·read_hex_write_available;

    Local Core·Copy·Fn Core·Copy·read_hex(){
      if((Core·Area·extent(Core·tf.copy.read) >> 1) == Core·Area·extent(Core·tf.copy.write)){
        return Core·Copy·read_hex_bulk;
      }
      if((Core·Area·extent(Core·tf.copy.read) >> 1) > Core·Area·extent(Core·tf.copy.write)){
        return Core·Copy·read_hex_read_surplus;
      }
      return Core·Copy·read_hex_write_available;
    }

    // Hex Decoding: Bulk Processing (Perfect Fit)
    Local Core·Copy·Fn Core·Copy·read_hex_bulk(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;

      do {
        **w = Core·tf.read_hex.hex_to_byte(*(uint16_t *)*r);
        if(*r == r1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Copy·Status·perfect_fit;
      return NULL;
    }

    // Hex Decoding: Read Surplus
    Local Core·Copy·Fn Core·Copy·read_hex_read_surplus(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.write);
      uint8_t **w = &Core·tf.copy.write_pt;

      do {
        **w = Core·tf.read_hex.hex_to_byte(*(uint16_t *)*r);
        if(*r == r1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Copy·Status·read_surplus;
      return NULL;
    }

    // Hex Decoding: Write Available
    Local Core·Copy·Fn Core·Copy·read_hex_write_available(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;
      uint8_t *w1 = Core·Area·position_right(Core·tf.copy.write);

      do {
        **w = Core·tf.read_hex.hex_to_byte(*(uint16_t *)*r);
        if(*w == w1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Copy·Status·write_available;
      return NULL;
    }

    //----------------------------------------
    // Initialization Block

    Core·M m = {
      .Area·init_pe = Core·Area·init_pe
      ,.Area·init_pp = Core·Area·init_pp
      ,.Area·set_position = Core·Area·set_position
      ,.Area·set_position_left = Core·Area·set_position
      ,.Area·set_position_right = Core·Area·set_position_right
      ,.Area·set_extent = Core·Area·set_extent
      ,.Area·position = Core·Area·position
      ,.Area·position_left = Core·Area·position
      ,.Area·position_right = Core·Area·position_right
      ,.Area·extent = Core·Area·extent
      ,.Area·length_Kung = Core·Area·length_Kung
      ,.Area·empty = Core·Area·empty

      ,.Area·encloses_pt = Core·Area·encloses_pt
      ,.Area·encloses_pt_strictly = Core·Area·encloses_pt_strictly
      ,.Area·encloses_area = Core·Area·encloses_area
      ,.Area·encloses_area_strictly = Core·Area·encloses_area_strictly
      ,.Area·overlap = Core·Area·overlap
      ,.Area·largest_aligned_64 = Core·Area·largest_aligned_64
      ,.Area·complement = Core·Area·complement

      ,.Area·read_8_zero = Core·Area·read_8_zero
      ,.Area·read_8_fwd = Core·Area·read_8_fwd
      ,.Area·read_8_rev = Core·Area·read_8_rev
      ,.Area·read_64_zero = Core·Area·read_64_zero
      ,.Area·read_64_fwd = Core·Area·read_64_fwd
      ,.Area·read_64_rev = Core·Area·read_64_rev

      ,.is_aligned_64 = Core·is_aligned_64
      ,.floor_64 = Core·floor_64
      ,.ceiling_64 = Core·ceiling_64
      ,.offset_8 = Core·offset_8
      ,.offset_64 = Core·offset_64

      ,.byte_to_hex = Core·byte_to_hex
      ,.hex_to_byte = Core·hex_to_byte

      ,.Core·copy = Core·copy
      ,.Copy·byte_by_byte = Core·Copy·byte_by_byte
      ,.Copy·by_word_64 = Core·Copy·by_word_64
      ,.Copy·write_hex = Core·Copy·write_hex
      ,.Copy·read_hex = Core·Copy·read_hex
    };

  #endif // LOCAL

#endif // IMPLEMENTATION
