/*
  Core - core memory operations.

  'ATP'  Used in comments. 'At This Point' in the code. 

  'Tape' refers to an tape (in contrast to a point) in the address space.

   A non-exist array is said to be non-existent.
   An array with zero elements has 'length == 0' or is 'empty'.

   In contrast, an allocation does not exist if it has zero bytes.

   It is better to separate the tableau than have a union, and let the
   optimizer figure out the memory life times, and what can be reused.

   Nodes include neighbor links, that makes traversal more efficient.


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

  //----------------------------------------
  // fundamental

    #define extentof(x)(sizeof(x) - 1)
    #define extent_t size_t

    // AU == Addressable Unit
    // given an AU is an 8 bit byte, 4AU is 32 bits, and 8 AU is 64 bits.
    #define AU uint8_t;

    void *offset(void *p ,size_t Δ);
    void *offset_8AU(void *p ,size_t Δ);
    bool is_aligned_8AU(void *p);

    // given an 8AU aligned window, return the AU pointer to the least and greatest index
    void *floor_within_aligned_8AU(void *p);
    void *ceiling_within_aligned_8AU(void *p);

  //----------------------------------------
  // Sequencer

    typedef struct Core·Node;

    typedef Core·Node *(*Core·Function)(Core·Node *node);

    typedef struct{
      Core·Function function;
    } Core·Node;

    void call(Core·Node *node){
      while(node) node = node->function(node));
    }

  //----------------------------------------
  // Tape Machine

    // tape

    typedef struct{
      uint8_t *position;
      extent_t extent;
    } Core·Area;

    // tape machine

    typedef struct{
      Core·Area *tape;
      AU *hd;
    } Core·TM;

    // Step

    typedef struct{
      Core·Node node;
      Core·Node *next;
      Core·TM *tm;
    } Core·Step·Node;

    Local Node *Core·step_AU(Node *node){
      Core·Step·Node *step_node = (Core·Step·Node *)node;
      step_node->hd = Core·offset(step_node->tm->hd ,1);
      return node->next;
    }

    Local Node *Core·step_8AU(Node *node){
      Core·Step·Node *step_node = (Core·Step·Node *)node;
      step_node->hd = Core·offset_8AU(step_node->tm->hd ,1);
      return node->next;
    }

    // Status

    typedef struct{
      Core·Node node;
      Core·Node *not_on_tape;
      Core·Node *interim;
      Core·Node *rightmost;
      Core·TM *tm;
    } Core·Status·Node;

    // Copy Cell

    typedef struct{
      Core·Node node;
      Core·Node *not_on_tape;
      Core·Node *interim;
      Core·Node *rightmost;
      Core·TM *tm;
    } Core·CopyCell·Node;

    uint8_t Area·read_8_zero(Core·Area *area ,void *r);
    uint8_t Area·read_8_fwd(Core·Area *area ,void *r);
    uint8_t Area·read_8_rev(Core·Area *area ,uint8_t *r);

typedef struct{
  Core·TM read;
  Core·TM write;
  Core·Function init;
  Core·Function copy_cell;
  Core·Function step;
  Core·Function status;
} Core·Copy·Tableau




// wire these in advance, on the tableau, then insert critical functions before call
void wire_loop_AU(Core·TM *tm_read ,Core·TM *tm_write){
  CopyCell·Node copy_cell;

  Status·Node read_status_node;

  Step·Node read_step_node;
  Step·Node write_step_node;

  copy_cell.node.function = Core·copy_AU;
  copy_cell.next = &read_status_node;

  read_status.node.function = Core·status;
  read_status.not_on_tape = NULL; // needs to be an error node
  read_status.interim = &step_node;
  read_status.rightmost = NULL;
  
  read_step_node.node.function = Core·stepAU;
  read_step_node.next = &write_step_node;

  write_step_node.node.function = Core·stepAU;
  write_step_node.next = &write_step_node;

}

  //----------------------------------------
  // Tape

    typedef struct Core·Area;

    typedef enum {
      Core·Area·Topology·nonexistent  // TapeMachine is unmounted (tape pointer is NULL)
      ,Core·Area·Topology·empty       // Tape exists but contains no elements (extent == 0)
      ,Core·Area·Topology·singleton   // Tape has exactly one element (extent == 1)
      ,Core·Area·Topology·finite      // Tape has a fixed, finite length (extent > 1)
      ,Core·Area·Topology·unbounded   // Tape conceptually extends infinitely
      ,Core·Area·Topology·cycles      // Tape has a repeating pattern but no explicit wraparound
      ,Core·Area·Topology·circular    // Tape is explicitly circular (head can wrap around)
    } Core·Area·Topology;

    typedef struct{
      Core·Area.Topology (*topology)(Tape *tape);
    } Core·Area·M

    typedef struct{
      Core·Area·M tape_m;
      void init_ppe(Core·Area *area ,void *position ,extent_t extent);
      void init_pp(Core·Area *area ,void *position_left ,void *position_right);
      void set_position(Core·Area *area ,uint8_t *new_position);
      void set_position_left(Core·Area *area ,uint8_t *new_position); // synonym
      extent_t set_extent(Core·Area *area);
      void set_position_right(Core·Area *area ,uint8_t *new_position_right);

      // read area properties
      bool empty(Core·Area *area);
      uint8_t *position(Core·Area *area);
      uint8_t *position_left(Core·Area *area); // synonym
      uint8_t *position_right(Core·Area *area);
      extent_t extent(Core·Area *area);
      uint8_t length_Kung(Core·Area *area); // saturates at 3

      // area relationships
      bool encloses_pt(uint8_t *pt ,Core·Area *area);
      bool encloses_pt_strictly(uint8_t *pt ,Core·Area *area);
      bool encloses_area(Core·Area *outer ,Core·Area *inner);
      bool encloses_area_strictly(Core·Area *outer ,Core·Area *inner);
      bool overlap(Core·Area *a ,Core·Area *b);
      void largest_aligned_64(Core·Area *outer ,Core·Area *inner_64);
      uint8_t *complement(Core·Area *area ,uint8_t *r);
    } Core·Area·M





typedef enum {
  Core·Head·Status·uinitialied 
  ,Core·Head·Status·debug
  ,Core·Head·Status·production
} Core·Head·Status;

typedef struct{
  Core·Node parent;
  uint8_t *pt;
} Core·Node·Head;




Local·Topology Area·topology_byte_array(Area *area){
  if(!area) return Core·Area·Topology·nonexistent;
  if(!area->position) return Core·Area·Topology·empty;
  if(area->extent == 0) return Core·Area·Topology·singleton;
  return Core·Area·Topology·finite;
}


  typedef uint8_t (*ReadFn8)(Area * ,uint8_t *);
  typedef uint64_t (*ReadFn64)(Area * ,uint64_t *);

  typedef enum {
    Core·Copy·Read·Status·argument_guard=0     // No data available (invalid read)
    ,Core·Copy·Read·Status·empty     // No data available (invalid read)
    ,Core·Copy·Read·Status·singleton // Only one value exists
    ,Core·Copy·Read·Status·first     // First value in a sequence
    ,Core·Copy·Read·Status·interim   // Not first, not last  
    ,Core·Copy·Read·Status·last       // Last value in a sequence
  } Core·Copy·Read·Status;

  typedef enum{
    Core·Copy·Status·uninitialized = 0
    ,Core·Copy·Status·argument_guard = 1
    ,Core·Copy·Status·perfect_fit = 2
    ,Core·Copy·Status·read_surplus = 4
    ,Core·Copy·Status·read_surplus_write_gap = 8
    ,Core·Copy·Status·write_available = 16
    ,Core·Copy·Status·write_gap = 32
  } Core·Copy·Status;




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
  Core·TableauFace Core·tf;

  typedef Core·Copy·Fn (*Core·Copy·Fn)();

  typedef struct{

    // void * pointer operations
    void *offset_8(void *p ,size_t Δ);
    void *offset_64(void *p ,size_t Δ);
    bool is_aligned_64(void *p);
    void *floor_64(void *p);
    void *ceiling_64(void *p);

    // initialize an area

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
    Copy·Status Core·copy(Core·Copy·Fn fn);
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
    } Core·Area;

    // I removed the unions, as they are debugging hazards, and also might confuse the optimizer
    typedef struct{
        struct{
        } byte_by_byte;
        struct{
          Area area_64;
        } copy_64;
        struct{
        } read_hex;
        struct{
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

    Local uint8_t Core·Area·read_8_fwd(){
      Core·Area a = Core·tf.read;
      uint8_t **r = &Core·tf.read_pt;

      #ifdef Core·Debug
        if(!a || !*r){
          fprintf(stderr ,"Core·Area·read_8_fwd:: read read_pt: %p %p\n" ,a ,*r); 
          return Core·Copy·Read·Status·argument_guard;
        }
        if( !Core·Area·enclose_pt(area ,r) ){
          fprintf(stderr,"Core·Area·read_8_fwd:: out of interval read\n");
        }
      #endif
      return *(uint8_t *)r;
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
        return Core·Copy·argument_guard;
      }
      if(
         true
         && fn != Core·Copy·by_word_64
         && fn != Core·Copy·byte_by_byte
         && fn != Core·write_hex
         && fn != Core·read_hex
      ){
        fprintf(stderr,"Core·copy:: unrecognized copy function\n");
        return Core·Copy·argument_guard;
      ) 
      #endif

      while(fn) fn = fn();
      return tf.copy.status;
    }

    //----------------------------------------
    // copy byte_by_byte 

    Core·Copy·Fn Core·Copy·Copy·ByteByByte·perfect_fit;
    Core·Copy·Fn Core·Copy·Copy·ByteByByte·read_surplus;
    Core·Copy·Fn Core·Copy·Copy·ByteByByte·write_available;

    Local Core·Copy·Fn Core·Copy·byte_by_byte(){
      if(Core·Area·extent(Core·tf.copy.read) == Core·Area·extent(Core·tf.copy.write))
        return Core·Copy·ByteByByte·perfect_fit;

      if(Core·Area·extent(Core·tf.copy.read) > Core·Area·extent(Core·tf.copy.write))
        return Core·Copy·ByteByByte·read_surplus;

      return Core·Copy·ByteByByte·write_available;
    }

    Local Core·Copy·Fn Core·Copy·ByteByByte·perfect_fit(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,*r);
        if(*r == r1) break;
        (*r)++;
        (*w)++;
      }while(true);

      Core·tf.copy.status = Core·Copy·Status·perfect_fit;
      return NULL;
    }

    Local Core·Copy·Fn Core·Copy·ByteByByte·read_surplus(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;
      uint8_t *w1 = Core·Area·position_right(Core·tf.copy.write);

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,*r);
        if(*w == w1) break;
        (*r)++;
        (*w)++;
      }while(true);

      Core·tf.copy.status = Core·Copy·Status·write_available;
      return NULL;
    }

    Local Core·Copy·Fn Core·Copy·ByteByByte·write_avalable(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,*r);
        if(*r == r1) break;
        (*r)++;
        (*w)++;
      }while(true);

      Core·tf.copy.status = Core·Copy·Status·read_surplus;
      return NULL;
    }

    //----------------------------------------
    // copy copy_64

    // 64-bit copy function with updated TableauFace terminology
    Core·Copy·Fn Core·Copy·by_word_64;
    Core·Copy·Fn Core·Copy·ByWord64·leadin;
    Core·Copy·Fn Core·Copy·ByWord64·bulk;
    Core·Copy·Fn Core·Copy·ByWord64·tail;

    // Initialize the copy_64 process
    Local Core·Copy·Fn Core·Copy·by_word_64(){
      // Determine the largest 64-bit aligned region within the read area
      Core·Area·largest_aligned_64(Core·tf.copy.read ,&Core·tl.copy_64.area_64);

      // Choose the correct function based on alignment
      if(Core·Area·empty(&Core·tl.copy_64.area_64)) return Core·Copy·ByWord64·tail;
      if(Core·is_aligned_64(Core·Area·position(Core·tf.copy.read))) return Core·Copy·ByWord64·bulk;
      return Core·Copy·ByWord64·leadin;
    }

    // Lead-in byte copy (until alignment)
    Local Core·Copy·Fn Core·Copy·ByWord64·leadin(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r0_64 = Core·Area·position(&Core·tl.copy_64.area_64);
      uint8_t **w = &Core·tf.copy.write_pt;

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,r0_64 ,*r);
        if(*r == r0_64) break;
        (*r)++;
        (*w)++;
      }while(true);

      return Core·Copy·ByWord64·bulk;
    }

    // Bulk word copy
    Local Core·Copy·Fn Core·Copy·ByWord64·bulk(){
      uint64_t **r = (uint64_t **)&Core·tf.copy.read_pt;
      uint64_t **w = (uint64_t **)&Core·tf.copy.write_pt;
      uint64_t *r1_64 = Core·Area·position_right(&Core·tl.copy_64.area_64);

      do{
        **w = Core·tf.copy.read_fn_64(Core·tf.copy.read ,r1_64 ,*r);
        if(*r == r1_64) break;
        (*r)++;
        (*w)++;
      }while(true);

      return Core·Copy·ByWord64·tail;
    }

    // Tail byte copy (unaligned trailing bytes)
    Local Core·Copy·Fn Core·Copy·ByWord64·tail(){
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
    Core·Copy·Fn Core·Copy·WriteHex·perfect_fit;
    Core·Copy·Fn Core·Copy·WriteHex·read_surplus;
    Core·Copy·Fn Core·Copy·WriteHex·write_available;

    // Hex Encoding: Initialize Copy
    Local Core·Copy·Fn Core·Copy·write_hex(){
      if(Core·Area·extent(Core·tf.copy.read) == (Core·Area·extent(Core·tf.copy.write) >> 1)){
        return Core·Copy·WriteHex·perfect_fit;
      }
      if(Core·Area·extent(Core·tf.copy.read) > (Core·Area·extent(Core·tf.copy.write) >> 1)){
        return Core·Copy·WriteHex·read_surplus;
      }
      return Core·Copy·WriteHex·write_available;
    }

    Local Core·Copy·Fn Core·Copy·WriteHex·perfect_fit(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;

      do {
        *(uint16_t *)*w = Core·hex.byte_to_hex(**r);
        if(*r == r1) break;
        (*r)++;
        (*w) += 2;
      } while(true);

      Core·tf.copy.status = Core·Copy·Status·perfect_fit;
      return NULL;
    }

    // Hex Encoding: Read Surplus
    Local Core·Copy·Fn Core·Copy·WriteHex·read_surplus(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.write);
      uint8_t **w = &Core·tf.copy.write_pt;

      do {
        *(uint16_t *)*w = Core·write_hex.byte_to_hex(**r);
        if(*r == r1) break;
        (*r)++;
        (*w) += 2;
      } while(true);

      Core·tf.copy.status = Core·Copy·Status·read_surplus;
      return NULL;
    }

    // Hex Encoding: Write Available
    Local Core·Copy·Fn Core·Copy·WriteHex·write_available(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;
      uint8_t *w1 = Core·Area·position_right(Core·tf.copy.write);

      do {
        *(uint16_t *)*w = Core·write_hex.byte_to_hex(**r);
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
    Core·Copy·Fn Core·Copy·ReadHex·perfect_fit;
    Core·Copy·Fn Core·Copy·ReadHex·read_surplus;
    Core·Copy·Fn Core·Copy·ReadHex·write_available;

    Local Core·Copy·Fn Core·Copy·read_hex(){
      if((Core·Area·extent(Core·tf.copy.read) >> 1) == Core·Area·extent(Core·tf.copy.write)){
        return Core·Copy·ReadHex·perfect_fit;
      }
      if((Core·Area·extent(Core·tf.copy.read) >> 1) > Core·Area·extent(Core·tf.copy.write)){
        return Core·Copy·ReadHex·read_surplus;
      }
      return Core·Copy·ReadHex·write_available;
    }

    Local Core·Copy·Fn Core·Copy·ReadHex·perfect_fit(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;

      do {
        **w = Core·hex_to_byte(*(uint16_t *)*r);
        if(*r == r1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Copy·Status·perfect_fit;
      return NULL;
    }

    Local Core·Copy·Fn Core·Copy·ReadHex·read_surplus(){
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

    Local Core·Copy·Fn Core·Copy·ReadHex·write_available(){
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
    // copy read hex

    Core·Copy·Fn Core·Copy·read_hex;
    Core·Copy·Fn Core·Copy·ReadHex·perfect_fit;
    Core·Copy·Fn Core·Copy·ReadHex·read_surplus;
    Core·Copy·Fn Core·Copy·ReadHex·write_available;

    Local Core·Copy·Fn Core·Copy·read_hex(){
      if((Core·Area·extent(Core·tf.copy.read) >> 1) == Core·Area·extent(Core·tf.copy.write)){
        return Core·Copy·ReadHex·perfect_fit;
      }
      if((Core·Area·extent(Core·tf.copy.read) >> 1) > Core·Area·extent(Core·tf.copy.write)){
        return Core·Copy·ReadHex·read_surplus;
      }
      return Core·Copy·ReadHex·write_available;
    }

    Local Core·Copy·Fn Core·Copy·ReadHex·perfect_fit(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;

      do {
        **w = Core·hex_to_byte(*(uint16_t *)*r);
        if(*r == r1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Copy·Status·perfect_fit;
      return NULL;
    }

    Local Core·Copy·Fn Core·Copy·ReadHex·read_surplus(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.write);
      uint8_t **w = &Core·tf.copy.write_pt;

      do {
        **w = Core·hex_to_byte(*(uint16_t *)*r);
        if(*r == r1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Copy·Status·read_surplus;
      return NULL;
    }

    Local Core·Copy·Fn Core·Copy·ReadHex·write_available(){
      uint8_t **r = &Core·tf.copy.read_pt;
      uint8_t *r1 = Core·Area·position_right(Core·tf.copy.read);
      uint8_t **w = &Core·tf.copy.write_pt;
      uint8_t *w1 = Core·Area·position_right(Core·tf.copy.write);

      do {
        **w = Core·hex_to_byte(*(uint16_t *)*r);
        if(*w == w1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Copy·Status·write_available;
      return NULL;
    }

    //----------------------------------------
    // Initialization Blocks

    //----------------------------------------
    // Tableaux

    Core·TableauFace tf = {
       .copy = {
          .read = NULL
          ,.write = NULL
          ,.read_fn_8 = Core·Area·read_8_fwd
          ,.read_fn_64 = Core·Area·read_64_fwd
          ,.read_pt = NULL
          ,.write_pt = NULL
          ,.status = Core·Copy·Status·uninitialized
       }
    };

    Core·TableauLocal tl = {
      .copy_64 = {
        .area_64 = {NULL ,0}
      }
    };

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

      ,.copy = Core·copy
      ,.Copy·byte_by_byte = Core·Copy·byte_by_byte
      ,.Copy·by_word_64 = Core·Copy·by_word_64
      ,.Copy·write_hex = Core·Copy·write_hex
      ,.Copy·read_hex = Core·Copy·read_hex
    };

  #endif // LOCAL

#endif // IMPLEMENTATION
