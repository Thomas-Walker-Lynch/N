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
#define FAC
#endif 

//--------------------------------------------------------------------------------
// Interface

#ifndef Core·FACE
#define Core·FACE

  #include <stdint.h>
  #include <stddef.h>

  //----------------------------------------
  // memory

    #define extentof(x)(sizeof(x) - 1)
    #define extent_t size_t

    // AU == Addressable Unit
    // given an AU is an 8 bit byte, 4AU is 32 bits, and 8 AU is 64 bits.
    #define AU uint8_t;

  //----------------------------------------
  // model

    typedef enum{
      Core·Status·mu = 0
      ,Core·Status·good
      ,Core·Status·bad
    }Core·Status;

    typedef struct{
    }Core·Tableau;

    typedef struct{
      Core·Tableau tableau;
      unint status;
    }Core·Tableau·Face;

    typedef struct Core·Action;
    typedef struct Core·Link;

    typedef Core·Link *(*Core·Fn)(Core·Link *);

    typedef struct{
      Core·Fn good;
      Core-Fn bad;
    }Core·Action;

    typedef struct{
      Core·Tableau *face;
      Core·Tableau *State;
      Core·Action *act;
    }Core·Link;

    // Then define the functions separately
    Local Core·Link *Core·Action·good_function(Core·Link *lnk){
      lnk->face->status = Core·Status·good;
      return NULL;
    }

    Local void call(Core·Link *lnk){
      while(lnk) lnk = lnk->act(lnk);
    }

    Local Core·Link *Core·Action·bad_function(Core·Link *lnk){
      lnk->face->status = Core·Status·bad;
      return NULL;
    }

    // Assign the function pointers to a struct
    Local Core·Action action = {
      .good = Core·Action·good_function,
      .bad = Core·Action·bad_function
    };


    // -----implementation

    typedef struct{
      Core·Tableau tableau;
    }Core·Tableau·State;



  //----------------------------------------
  // utility

    struct{
      void *offset(void *p ,size_t Δ);
      void *offset_8AU(void *p ,size_t Δ);

      // given an 8AU window aligned on an 8AU boundary
      bool is_aligned_on_8AU(void *p);
      void *floor_within_aligned_8AU(void *p);
      void *ceiling_within_aligned_8AU(void *p);
    }Core;

  //----------------------------------------
  // Area model

    typedef struct Core·Tape;

    typedef struct{
      Core·Tableau·Face;
      Core·Tape *tape;
      extent_t extent;
      Core·Link extent_mu;
      Core·Link extent_next;
    }Core·Tape·Tableau·Face;

    typedef enum{
      Core·Area·Topology·mu
      ,Core·Area·Topology·nonexistent // pointer to tape is NULL
      ,Core·Area·Topology·empty      // tape has no cells
      ,Core·Area·Topology·singleton  // extent is zero
      ,Core·Area·Topology·segment    // finite non-singleton tape
      ,Core·Area·Topology·circle     // initial location recurs
      ,Core·Area·Topology·cyclic     // a location recurs
      ,Core·Area·Topology·infinite   // exists, not empty, no cycle, no rightmost
    }Core·Tape·Topology;

    typedef struct{
      Core·Fn
      Core·Tape·Topology (*topology)(Core·Tape *tape);
      extent_t extent(Core·Area *area); 

    }Core·Tape·Action;

    typedef struct Core·Area; // extends Tape

    typedef struct{
      Core·Tape·Tableau·Face tape_tableau;
      Core·Area *area;
      void *position_left;
      void *position_right;

    }Core·Area·Tableau·Face;



    typedef struct{
      Core·Tape·Action tape;

      void init_pe(Core·Area *area ,void *position ,extent_t extent);
      void init_pp(Core·Area *area ,void *position_left ,void *position_right);
      void set_position(Core·Area *area ,AU *new_position);
      void set_position_left(Core·Area *area ,AU *new_position); // synonym
      void set_position_right(Core·Area *area ,AU *new_position_right);
      void set_extent(Core·Area *area ,extent_t extent); 

      // read area properties
      AU *position(Core·Area *area);
      AU *position_left(Core·Area *area); // synonym
      AU *position_right(Core·Area *area);

      AU *complement(Core·Area *area ,AU *r);

      // area relationships
      bool encloses_pt(AU *pt ,Core·Area *area);
      bool encloses_pt_strictly(AU *pt ,Core·Area *area);
      bool encloses_area(Core·Area *outer ,Core·Area *inner);
      bool encloses_area_strictly(Core·Area *outer ,Core·Area *inner);
      bool overlap(Core·Area *a ,Core·Area *b);
      void largest_aligned_64(Core·Area *outer ,Core·Area *inner_64);

    } Core·Area·Action;


  //----------------------------------------
  // Tape Machine

    typedef struct Core·TM_NX;

    // if tape machine does not support step left, then Status·leftmost will be reported as Status·interim
    typedef enum{
      Core·TM·Head·Status·mu
      ,Core·TM·Head·Status·not_on_tape = 1
      ,Core·TM·Head·Status·on_leftmost    = 1 << 1
      ,Core·TM·Head·Status·in_interim     = 1 << 2
      ,Core·TM·Head·Status·on_rightmost   = 1 << 3
    }Core·TM·Head·Status;

    const uint Core·TM·Head·Status·bad =
      Core·TM·Head·Status·mu 
      | Core·TM·Head·Status·not_on_tape
      ;

    const uint Core·TM·Head·Status·good =
      Core·TM·Head·Status·leftmost   
      | Core·TM·Head·Status·interim   
      | Core·TM·Head·Status·rightmost 
      ;

    typedef struct{
      Core·Tableau·Face
      Core·Area·M *a;
      Core·TM·Head·Status status; // perhaps integrate this with Core·tableau status
      Core·Tape·Topology topology;
    }Core·TM_NX·Tableau·Face;

    // default Tableau
    Local Core·TM_NX·Tableau Core·TM_NX·t;

    typedef struct{
      void mount(Core·TM_NX·Tableau *);
      void rewind(Core·TM_NX·Tableau *);
      void step(Core·TM_NX·Tableau *);
      void step_left(Core·TM_NX·Tableau *);
      void status(Core·TM_NX·Tableau *);
      void topology(Core·TM_NX·Tableau *);
    } Core·TM_NX·Action;
    // default actions table
    Local Core·TM_NX·Action Core·TM_NX·action;

    // default link
    Core·Link Core·TM_NX·link;


  //----------------------------------------
  // Map

    typedef enum{
      Core·Map·Status·mu = 0
      ,Core·Map·Status·no_tape
      ,Core·Map·Status·not_computable
      ,Core·Map·Status·complete
    } Core·Map·Status;

    typedef enum{
      Core·Map·Completion·mu = 0
      ,Core·Map·Completion·no_tape                
      ,Core·Map·Completion·not_computable         
      ,Core·Map·Completion·failed                 
      ,Core·Map·Completion·perfect_fit            
      ,Core·Map·Completion·read_surplus           
      ,Core·Map·Completion·read_surplus_write_gap 
      ,Core·Map·Completion·write_available        
      ,Core·Map·Completion·write_gap              
    } Core·Map·Completion;

    const uint Core·Map·Completion·bad =
      Core·Map·Completion·no_tape                
      | Core·Map·Completion·not_computable         
      | Core·Map·Completion·failed
      ;

    const uint Core·Map·Completion·good =
      Core·Map·Completion·perfect_fit            
      | Core·Map·Completion·read_surplus           
      | Core·Map·Completion·read_surplus_write_gap 
      | Core·Map·Completion·write_available        
      | Core·Map·Completion·write_gap              
      ;

    typedef Core·Map·Fn (*Core·Map·Fn)();

    // Link for Map
    typedef struct{
        Core·Link *domain;
        Core·Link *range;
        Core·Map·Fn *fn;
        Core·Map·Status status;
    } Core·Map·Tableau;

    void Core·map(Core·Map·Tableau t);

    // true if function enters the map loop, otherwise false.
    void Core·map(Core·Map·Tableau *t){
      #ifdef Core·DEBUG
        if(!t){
          fprintf(stderr, "Core·Map·Tableau:: given NULL t");
          return;
        }
        uint error = 0;
        if( t->status & Core·Map·Completion·bad != 0 ){
          fprintf(stderr, "Core·Map:: prior map completion status is bad.");
        }
        call(status ,t->domain);
        if(t->domain->tableau->status & Core·TM·Head·Status·good == 0){ 
          fprintf(stderr, "Core·Map:: domain is not good.");
          error++;
        }
        call(status ,t->range);
        if(t->range->tableau->status & Core·TM·Head·Status·good == 0){ 
          fprintf(stderr, "Core·Map:: range is not good.");
          error++;
        }
        if(error > 0) return;
      #endif
      

    }

  //----------------------------------------
  // Copy

    typedef enum{
      Core·Copy·Status·mu = 0
      ,Core·Copy·Status·argument_guard = 1
      ,Core·Copy·Status·perfect_fit = 2
      ,Core·Copy·Status·read_surplus = 4
      ,Core·Copy·Status·read_surplus_write_gap = 8
      ,Core·Copy·Status·write_available = 16
      ,Core·Copy·Status·write_gap = 32
    } Core·Copy·Status;

    typedef struct{
      Core·TM read;
      Core·TM write;
      Core·Function init;
      Core·Function copy_cell;
      Core·Function step;
      Core·Function status;
    } Core·Copy·Link;

    typedef struct{

      uint8AU_t Area·read_8AU_zero(Core·Area *area ,void *r);
      uint8AU_t Area·read_8AU_fwd(Core·Area *area ,void *r);
      uint8AU_t Area·read_8AU_rev(Core·Area *area_8AU ,void *r);

      // hex conversion
      uint16_t byte_to_hex(uint8_t byte);
      uint8_t hex_to_byte(uint16_t hex);

      // copy one area to another, possibly with a transformation
      Map·Status Core·map(Core·Map·Fn fn);
      Map·Fn Map·AU_by_AU;
      Map·Fn Map·by_8AU;
      Map·Fn Map·write_hex;
      Map·Fn Map·read_hex;

    } Core·MapFn·Face;


#endif

//--------------------------------------------------------------------------------
// Implementation

  typedef AU (*ReadFn8)(Area * ,AU *);
  typedef uint64_t (*ReadFn64)(Area * ,uint64_t *);


Local·Topology Area·topology_byte_array(Area *area){
  if(!area) return Core·Area·Topology·nonexistent;
  if(!area->position) return Core·Area·Topology·empty;
  if(area->extent == 0) return Core·Area·Topology·singleton;
  return Core·Area·Topology·finite;
}


    typedef struct{
      AU *position;
      extent_t extent;
    } Core·Area;



    typedef struct{
      Core·Area *area;
      AU *hd;
    } Core·TM·Array;


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






#ifdef Core·IMPLEMENTATION
  // declarations available to all of the IMPLEMENTATION go here
  //
    #ifdef Core·DEBUG
      #include <stdio.h>
    #endif

    typedef struct{
      AU *position;
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

  // this part goes into Maplib.a
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
      return (void *)((AU *)p) + Δ;
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

    Local bool Core·is_aligned_on_64(void *p){
      #ifdef Core·Debug
      if(!p){
        fprintf(stderr,"Core·is_aligned_on_64:: given NULL `p'");
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
    Local void Core·Area·init_pe(Core·Area *area ,AU *position ,extent_t extent){
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
    Local AU *Core·Area·position(Core·Area *area){
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


    Local AU *Core·Area·position_right(Core·Area *area){
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
    Local AU Core·Area·length_Kung(Core·Area *area){
      if(!Core·Area·position_left(area)) return 0;
      if(Core·Area·extent(area) >= 2) return 3;
      return Core·Area·extent(area) + 1;
    }

    Local bool Core·Area·encloses_pt(Core·Area *area ,AU *pt){
      return 
        (pt >= Core·Area·position_left(area)) 
        && (pt <= Core·Area·position_right(area));
    }
    Local bool Core·Area·encloses_pt_strictly(Core·Area *area ,AU *pt){
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

      AU *p0_64 = (AU *)( (p0 + 0x7) & ~(uintptr_t)0x7 );
      AU *p1_64 = (AU *)( (p1 - 0x7) & ~(uintptr_t)0x7 );

      if(p1_64 < p0_64){
        Core·Area·set_position(inner_64 ,NULL);
      }else{
        Core·Area·init_pp(inner_64 ,p0_64 ,p1_64);
      }
    }

    // complement against the extent of the area (reverse direction)
    // works for byte pointer
    // works for aligned word pointer
    Local AU *Core·Area·complement(Core·Area *area ,AU *r){
      return Core·Area·position_left(area) + (Core·Area·position_right(area) - r);
    }

    //----------------------------------------
    // read functions

    // consider instead using `copy_zero`
    Local AU Core·Area·read_8_zero(Core·Area *area ,void *r){
      return 0;
    }
    Local uint64_t Core·Area·read_64_zero(Core·Area *area ,void *r){
      return 0;
    }

    Local AU Core·Area·read_8_fwd(){
      Core·Area a = Core·tf.read;
      AU **r = &Core·tf.read_pt;

      #ifdef Core·Debug
        if(!a || !*r){
          fprintf(stderr ,"Core·Area·read_8_fwd:: read read_pt: %p %p\n" ,a ,*r); 
          return Core·Map·Read·Status·argument_guard;
        }
        if( !Core·Area·enclose_pt(area ,r) ){
          fprintf(stderr,"Core·Area·read_8_fwd:: out of interval read\n");
        }
      #endif
      return *(AU *)r;
    }

    Local AU Core·Area·read_8_fwd(Core·Area *area ,void *r){
      #ifdef Core·Debug
      if(!area || !r){
        fprintf(stderr,"Core·Area·read_8_fwd:: area r: %p %p\n" ,area ,r);
        return 0;
      }
      if( !Core·Area·enclose_pt(area ,r) ){
        fprintf(stderr,"Core·Area·read_8_fwd:: out of interval read\n");
      }
      #endif
      return *(AU *)r;
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

    Local AU Core·Area·read_8_rev(Core·Area *area ,AU *r){
      return *(Core·complement(area ,r));
    }

    Local uint64_t Core·Area·read_64_rev(Core·Area *area_64 ,AU *r){
      return __builtin_bswap64( *(uint64_t *)Core·floor_64(Core·complement(area_64 ,r)) );
    }

    //----------------------------------------
    // Map

    // Map function using trampoline execution model
    Local Core·Map·Status Core·map(Core·Map·Fn fn){
      #ifdef Core·Debug
      if(!fn){
        fprintf(stderr,"Core·map:: given null function");
        return Core·Map·argument_guard;
      }
      if(
         true
         && fn != Core·Map·by_8AU
         && fn != Core·Map·AU_by_AU
         && fn != Core·write_hex
         && fn != Core·read_hex
      ){
        fprintf(stderr,"Core·map:: unrecognized copy function\n");
        return Core·Map·argument_guard;
      ) 
      #endif

      while(fn) fn = fn();
      return tf.copy.status;
    }

    //----------------------------------------
    // copy byte_by_byte 

    Core·Map·Fn Core·Map·Map·ByteByByte·perfect_fit;
    Core·Map·Fn Core·Map·Map·ByteByByte·read_surplus;
    Core·Map·Fn Core·Map·Map·ByteByByte·write_available;

    Local Core·Map·Fn Core·Map·AU_by_AU(){
      if(Core·Area·extent(Core·tf.copy.read) == Core·Area·extent(Core·tf.copy.write))
        return Core·Map·ByteByByte·perfect_fit;

      if(Core·Area·extent(Core·tf.copy.read) > Core·Area·extent(Core·tf.copy.write))
        return Core·Map·ByteByByte·read_surplus;

      return Core·Map·ByteByByte·write_available;
    }

    Local Core·Map·Fn Core·Map·ByteByByte·perfect_fit(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,*r);
        if(*r == r1) break;
        (*r)++;
        (*w)++;
      }while(true);

      Core·tf.copy.status = Core·Map·Status·perfect_fit;
      return NULL;
    }

    Local Core·Map·Fn Core·Map·ByteByByte·read_surplus(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;
      AU *w1 = Core·Area·position_right(Core·tf.copy.write);

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,*r);
        if(*w == w1) break;
        (*r)++;
        (*w)++;
      }while(true);

      Core·tf.copy.status = Core·Map·Status·write_available;
      return NULL;
    }

    Local Core·Map·Fn Core·Map·ByteByByte·write_avalable(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,*r);
        if(*r == r1) break;
        (*r)++;
        (*w)++;
      }while(true);

      Core·tf.copy.status = Core·Map·Status·read_surplus;
      return NULL;
    }

    //----------------------------------------
    // copy copy_64

    // 64-bit copy function with updated TableauFace terminology
    Core·Map·Fn Core·Map·by_8AU;
    Core·Map·Fn Core·Map·ByWord64·leadin;
    Core·Map·Fn Core·Map·ByWord64·bulk;
    Core·Map·Fn Core·Map·ByWord64·tail;

    // Initialize the copy_64 process
    Local Core·Map·Fn Core·Map·by_8AU(){
      // Determine the largest 64-bit aligned region within the read area
      Core·Area·largest_aligned_64(Core·tf.copy.read ,&Core·tl.copy_64.area_64);

      // Choose the correct function based on alignment
      if(Core·Area·empty(&Core·tl.copy_64.area_64)) return Core·Map·ByWord64·tail;
      if(Core·is_aligned_on_64(Core·Area·position(Core·tf.copy.read))) return Core·Map·ByWord64·bulk;
      return Core·Map·ByWord64·leadin;
    }

    // Lead-in byte copy (until alignment)
    Local Core·Map·Fn Core·Map·ByWord64·leadin(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r0_64 = Core·Area·position(&Core·tl.copy_64.area_64);
      AU **w = &Core·tf.copy.write_pt;

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,r0_64 ,*r);
        if(*r == r0_64) break;
        (*r)++;
        (*w)++;
      }while(true);

      return Core·Map·ByWord64·bulk;
    }

    // Bulk word copy
    Local Core·Map·Fn Core·Map·ByWord64·bulk(){
      uint64_t **r = (uint64_t **)&Core·tf.copy.read_pt;
      uint64_t **w = (uint64_t **)&Core·tf.copy.write_pt;
      uint64_t *r1_64 = Core·Area·position_right(&Core·tl.copy_64.area_64);

      do{
        **w = Core·tf.copy.read_fn_64(Core·tf.copy.read ,r1_64 ,*r);
        if(*r == r1_64) break;
        (*r)++;
        (*w)++;
      }while(true);

      return Core·Map·ByWord64·tail;
    }

    // Tail byte copy (unaligned trailing bytes)
    Local Core·Map·Fn Core·Map·ByWord64·tail(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(&Core·tl.copy_64.area_64);
      AU **w = &Core·tf.copy.write_pt;

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,r1 ,*r);
        if(*r == r1) break;
        (*r)++;
        (*w)++;
      }while(true);

      Core·tf.copy.status = Core·Map·Status·perfect_fit;
      return NULL;
    }

    //----------------------------------------
    // copy write hex

    Local uint16_t Core·byte_to_hex(AU byte){
      static const char hex_digits[] = "0123456789ABCDEF";
      return 
          (hex_digits[byte >> 4] << 8) 
        | hex_digits[byte & 0x0F];
    }

    // Forward Declarations
    Core·Map·Fn Core·Map·write_hex;
    Core·Map·Fn Core·Map·WriteHex·perfect_fit;
    Core·Map·Fn Core·Map·WriteHex·read_surplus;
    Core·Map·Fn Core·Map·WriteHex·write_available;

    // Hex Encoding: Initialize Map
    Local Core·Map·Fn Core·Map·write_hex(){
      if(Core·Area·extent(Core·tf.copy.read) == (Core·Area·extent(Core·tf.copy.write) >> 1)){
        return Core·Map·WriteHex·perfect_fit;
      }
      if(Core·Area·extent(Core·tf.copy.read) > (Core·Area·extent(Core·tf.copy.write) >> 1)){
        return Core·Map·WriteHex·read_surplus;
      }
      return Core·Map·WriteHex·write_available;
    }

    Local Core·Map·Fn Core·Map·WriteHex·perfect_fit(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;

      do {
        *(uint16_t *)*w = Core·hex.byte_to_hex(**r);
        if(*r == r1) break;
        (*r)++;
        (*w) += 2;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·perfect_fit;
      return NULL;
    }

    // Hex Encoding: Read Surplus
    Local Core·Map·Fn Core·Map·WriteHex·read_surplus(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.write);
      AU **w = &Core·tf.copy.write_pt;

      do {
        *(uint16_t *)*w = Core·write_hex.byte_to_hex(**r);
        if(*r == r1) break;
        (*r)++;
        (*w) += 2;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·read_surplus;
      return NULL;
    }

    // Hex Encoding: Write Available
    Local Core·Map·Fn Core·Map·WriteHex·write_available(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;
      AU *w1 = Core·Area·position_right(Core·tf.copy.write);

      do {
        *(uint16_t *)*w = Core·write_hex.byte_to_hex(**r);
        if(*w == w1) break;
        (*r)++;
        (*w) += 2;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·write_available;
      return NULL;
    }

    //----------------------------------------
    // copy read hex

    Local AU Core·hex_to_byte(uint16_t hex){
      AU high = hex >> 8;
      AU low = hex & 0xFF;

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

    Core·Map·Fn Core·Map·read_hex;
    Core·Map·Fn Core·Map·ReadHex·perfect_fit;
    Core·Map·Fn Core·Map·ReadHex·read_surplus;
    Core·Map·Fn Core·Map·ReadHex·write_available;

    Local Core·Map·Fn Core·Map·read_hex(){
      if((Core·Area·extent(Core·tf.copy.read) >> 1) == Core·Area·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·perfect_fit;
      }
      if((Core·Area·extent(Core·tf.copy.read) >> 1) > Core·Area·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·read_surplus;
      }
      return Core·Map·ReadHex·write_available;
    }

    Local Core·Map·Fn Core·Map·ReadHex·perfect_fit(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;

      do {
        **w = Core·hex_to_byte(*(uint16_t *)*r);
        if(*r == r1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·perfect_fit;
      return NULL;
    }

    Local Core·Map·Fn Core·Map·ReadHex·read_surplus(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.write);
      AU **w = &Core·tf.copy.write_pt;

      do {
        **w = Core·tf.read_hex.hex_to_byte(*(uint16_t *)*r);
        if(*r == r1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·read_surplus;
      return NULL;
    }

    Local Core·Map·Fn Core·Map·ReadHex·write_available(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;
      AU *w1 = Core·Area·position_right(Core·tf.copy.write);

      do {
        **w = Core·tf.read_hex.hex_to_byte(*(uint16_t *)*r);
        if(*w == w1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·write_available;
      return NULL;
    }

    //----------------------------------------
    // copy read hex

    Core·Map·Fn Core·Map·read_hex;
    Core·Map·Fn Core·Map·ReadHex·perfect_fit;
    Core·Map·Fn Core·Map·ReadHex·read_surplus;
    Core·Map·Fn Core·Map·ReadHex·write_available;

    Local Core·Map·Fn Core·Map·read_hex(){
      if((Core·Area·extent(Core·tf.copy.read) >> 1) == Core·Area·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·perfect_fit;
      }
      if((Core·Area·extent(Core·tf.copy.read) >> 1) > Core·Area·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·read_surplus;
      }
      return Core·Map·ReadHex·write_available;
    }

    Local Core·Map·Fn Core·Map·ReadHex·perfect_fit(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;

      do {
        **w = Core·hex_to_byte(*(uint16_t *)*r);
        if(*r == r1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·perfect_fit;
      return NULL;
    }

    Local Core·Map·Fn Core·Map·ReadHex·read_surplus(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.write);
      AU **w = &Core·tf.copy.write_pt;

      do {
        **w = Core·hex_to_byte(*(uint16_t *)*r);
        if(*r == r1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·read_surplus;
      return NULL;
    }

    Local Core·Map·Fn Core·Map·ReadHex·write_available(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;
      AU *w1 = Core·Area·position_right(Core·tf.copy.write);

      do {
        **w = Core·hex_to_byte(*(uint16_t *)*r);
        if(*w == w1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·write_available;
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
          ,.status = Core·Map·Status·uninitialized
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

      ,.is_aligned_on_64 = Core·is_aligned_on_64
      ,.floor_64 = Core·floor_64
      ,.ceiling_64 = Core·ceiling_64
      ,.offset_8 = Core·offset_8
      ,.offset_64 = Core·offset_64

      ,.byte_to_hex = Core·byte_to_hex
      ,.hex_to_byte = Core·hex_to_byte

      ,.copy = Core·map
      ,.Map·AU_by_AU = Core·Map·AU_by_AU
      ,.Map·by_8AU = Core·Map·by_8AU
      ,.Map·write_hex = Core·Map·write_hex
      ,.Map·read_hex = Core·Map·read_hex
    };

  #endif // LOCAL

#endif // IMPLEMENTATION
