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

   FG FunctionsGiven<type>, e.g. FunctionsGiven<Core·Area>, abbreviate as FG, or fg.
   Nice abbreviation as f and g are common one letter abreviaions for a functions.

   U - short for Utility
  
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

    // AU == Addressable Unit
    // given an AU is an 8 bit byte, 4AU is 32 bits, and 8 AU is 64 bits.
    // uint8_t is the current industry de-facto standard
    #define AU uint8_t

    // sizeof, and thus extent_t are measures of AU
    #define extentof(x)(sizeof(x) - 1)
    #define extent_t size_t

    typedef enum{
       Core·Status·mu = 0
      ,Core·Status·on_track
      ,Core·Status·derailed
    }Core·Status;

  //----------------------------------------
  // argument guard

    typedef void (*Flag·Function)(uint8_t *flag ,uint err);

    void Core·Flag·count(uint8_t *flag ,uint err){
      if(err >= 0xFF){ *flag = 0xFF; return;}

      //*flag + err > 0xFF
      if(*flag > 0xFF - err){ *flag = 0xFF; return;}

      (*flag) += err;
    }

    void Core·Flag·collect(uint8_t *flag ,uint err){
      (*flag) |= err;
    }

    typedef struct {
      char *name;
      Flag·Function flag_function;
      uint8_t flag;
    } Core·Guard;

    typedef struct {
      void (*init)(Core·Guard *chk ,const char *name ,Flag·Function af);
      void (*reset)(Core·Guard *chk);
      void (*check)(
         Core·Guard *chk
        ,uint err
        ,bool condition
        ,char *message
      );
    } Core·Guard·FG;

    // Default guard function table
    // initialized in the implementation section below
    Local Core·Guard·FG Core·Guard·fg;

    #define Core·Guard·if_return(chk) if( chk.flag ) return Core·Status·derailed;
    #define Core·Guard·return(chk)\
      if( chk.flag ) return Core·Status·derailed;\
      else return Core·Status·on_track;

  //----------------------------------------
  // functions

    // no state, this is merely a namespace

    typedef struct{
      Core·Status (*on_track)();
      Core·Status (*derailed)();

      void *(*offset)(void *p ,size_t Δ);
      void *(*offset_8AU)(void *p ,size_t Δ);

      bool (*is_aligned_on_8AU)(void *p);
      void *(*floor_within_aligned_8AU)(void *p);
      void *(*ceiling_within_aligned_8AU)(void *p);

    } Core·F;

    Local Core·F Core·f;

  //----------------------------------------
  // Tape Machine

    typedef struct Core·TM_NX;
    typedef struct Core·TM_NX·Address;
    typedef struct Core·TM_NX·Remote;

    typedef enum{
       Core·Tape·Topo·mu = 0 
      ,Core·Tape·Topo·nonexistent = 1 
      ,Core·Tape·Topo·empty       = 1 << 1
      ,Core·Tape·Topo·singleton   = 1 << 2
      ,Core·Tape·Topo·segment     = 1 << 3
      ,Core·Tape·Topo·circle      = 1 << 4
      ,Core·Tape·Topo·tail_cyclic = 1 << 5
      ,Core·Tape·Topo·infinite    = 1 << 6
    }Core·Tape·Topo;

    const Core·Tape·Topo Core·Tape·Topo·finite_nz = 
      Core·Tape·Topo·singleton | Core·Tape·Topo·segment
      ;

    // If tape machine does not support step left, then Status·leftmost 
    // will be reported as Status·interim.
    typedef enum{
       Core·TM·Head·Status·mu = 0
      ,Core·TM·Head·Status·not_on_tape = 1
      ,Core·TM·Head·Status·origin    = 1 << 1
      ,Core·TM·Head·Status·interim   = 1 << 2
      ,Core·TM·Head·Status·rightmost = 1 << 3
    } Core·TM·Head·Status;

    const Core·TM·Head·Status Core·TM·Head·Status·on_tape = 
      Core·TM·Head·Status·origin
      | Core·TM·Head·Status·interim
      | Core·TM·Head·Status·rightmost
      ;

    typedef struct{

      // these are for types that need initialization and cleanup
      Core·TM·Head·Status (*mount)(Core·TM_NX *tm);
      Core·TM·Head·Status (*dismount)(Core·TM_NX *tm);

      Core·TM·Head·Status (*status)(Core·TM_NX *tm);
      Core·Status (*can_read)(Core·TM_NX *tm ,bool *flag);

      Core·Status (*on_origin)(Core·TM_NX *tm ,bool *flag);
      // only for bounded tapes
      Core·Status (*on_rightmost)(Core·TM_NX *tm ,bool *flag);

      Core·Status (*read)(
         Core·TM_NX *tm 
        ,Core·TM_NX·Address *a 
        ,Core·Tape·Remote *remote
      );
      Core·Status (*write)(
         Core·TM_NX *tm 
        ,Core·TM_NX·Address *a 
        ,Core·Tape·Remote *remote
      );

      Core·Status (*rewind)(Core·TM_NX *tm);
      Core·Status (*step)(Core·TM_NX *tm);
      Core·Status (*step_left)(Core·TM_NX *tm);
      Core·Status (*step_right)(Core·TM_NX *tm); // Synonym for step

      struct {
        Core·Tape·Topo (*topo)(Core·TM_NX *tm);
        Core·Status (*extent)(Core·TM_NX *tm ,extent_t *extent_pt);

        // Initialize TM area
        Core·Status (*mount_pe)(Core·TM_NX *tm ,void *position ,extent_t extent);
        Core·Status (*mount_pp)(Core·TM_NX *tm ,void *position_left ,void *position_right);
        Core·Status (*dismount)(Core·TM_NX *tm);

        // Read TM properties
        Core·TM_NX·Address *(*position)(Core·TM_NX *tm);
        Core·TM_NX·Address *(*position_left)(Core·TM_NX *tm); // synonym
        Core·TM_NX·Address *(*position_right)(Core·TM_NX *tm);

        Core·TM_NX·Address *(*complement)(Core·TM_NX *tm ,Core·TM_NX·Address *r);

        // TM relationships
        bool (*encloses_pt)(Core·TM_NX *tm ,Core·TM_NX·Address *pt);
        bool (*encloses_pt_strictly)(Core·TM_NX *tm ,Core·TM_NX·Address *pt);
        bool (*encloses_tm)(Core·TM_NX *outer ,Core·TM_NX *inner);
        bool (*encloses_tm_strictly)(Core·TM_NX *outer ,Core·TM_NX *inner);
        bool (*overlap)(Core·TM_NX *a ,Core·TM_NX *b);
        void (*largest_aligned_64)(Core·TM_NX *outer ,Core·TM_NX *inner_64);
      } area;

    } Core·TM_NX·FG;


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

    const uint Core·Map·Completion·derailed =
        Core·Map·Completion·no_tape                
      | Core·Map·Completion·not_computable         
      | Core·Map·Completion·failed
      ;

    const uint Core·Map·Completion·on_track =
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
      /*
        if(!t){
          fprintf(stderr, "Core·Map·Tableau:: given NULL t");
          return;
        }
        uint error = 0;
        if( (t->status & Core·Map·Completion·derailed) != 0 ){
          fprintf(stderr, "Core·Map:: prior map completion status is derailed.");
        }
        call(status ,t->domain);
        if( (t->domain->tableau->status & Core·TM·Head·Status·on_track) == 0 ){ 
          fprintf(stderr, "Core·Map:: domain is not on_track.");
          error++;
        }
        call(status ,t->range);
        if( (t->range->tableau->status & Core·TM·Head·Status·on_track) == 0 ){ 
          fprintf(stderr, "Core·Map:: range is not on_track.");
          error++;
        }
        if(error > 0) return;
      */
      #endif
      

    }


#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef Core·IMPLEMENTATION
  // declarations available to all of the IMPLEMENTATION go here
  //
    #ifdef Core·DEBUG
      #include <stdio.h>
    #endif

  // implementation to go into the lib.a file
  //
    #ifndef LOCAL
    #endif 

  #ifdef LOCAL


  //----------------------------------------
  // argument guard
  //----------------------------------------


    Local void Core·Guard·init(Core·Guard *chk ,Flag·Function af){
      if( !chk ) return;
      chk->flag_function = af;
      chk->flag = 0;
    }

    Local void Core·Guard·reset(Core·Guard *chk){
      if( !chk ) return;
      chk->flag = 0;
    }

    Local void Core·Guard·check(
       Core·Guard *chk
      ,uint err
      ,bool condition
      ,const char *message
    ){
      if( !chk || !chk->flag_function ) return;
      if( condition ) return;
      fprintf(stderr ,"%s\n" ,message);
      chk->flag_function(&chk->flag ,err);
    }

    Local Core·Guard·FG Core·Guard·fg = {
       .init = Core·Guard·init
      ,.reset = Core·Guard·reset
      ,.check = Core·Guard·check
    };


  //----------------------------------------
  // Functions
  //----------------------------------------


    Core·Status Core·on_track(){ return Core·Status·on_track; }
    Core·Status Core·derailed(){ return Core·Status·derailed; }

    Local void *Core·offset(void *p ,size_t Δ){
      #ifdef Core·Debug
      if(!p){
        fprintf(stderr ,"Core·offset:: given NULL `p'");
        return NULL;
      }
      #endif
      return (void *)( (AU *)p ) + Δ;
    }

    Local void *Core·offset_8AU(void *p ,size_t Δ){
      #ifdef Core·Debug
      if(!p){
        fprintf(stderr ,"Core·offset_8AU:: given NULL `p'");
        return NULL;
      }
      #endif
      return (void *)( (uint64_t *)p ) + Δ;
    }

    Local bool Core·is_aligned_on_8AU(void *p){
      #ifdef Core·Debug
      if(!p){
        fprintf(stderr ,"Core·is_aligned_on_8AU:: given NULL `p'");
        return false;
      }
      #endif
      return ( (uintptr_t)p & 0x7 ) == 0;
    }

    // find the lowest address in an 8 byte aligned window
    // returns the byte pointer to the least address byte in the window
    Local void *Core·floor_within_aligned_8AU(void *p){
      #ifdef Core·Debug
      if(!p){
        fprintf(stderr ,"Core·floor_8AU:: given NULL `p'" );
        return NULL;
      }
      #endif
      return (void *)( (uintptr_t)p & ~(uintptr_t)0x7 );
    }

    // find the largest address in an 8 byte aligned window
    // returns the byte pointer to the greatest address byte in the window
    Local void *Core·ceiling_within_aligned_8AU(void *p){
      #ifdef Core·Debug
      if(!p){
        fprintf(stderr ,"Core·ceiling_64:: given NULL `p'" );
        return NULL;
      }
      #endif
      return (void *)( (uintptr_t)p | 0x7 );
    }

    // Struct instance initialization
    Core·U = Core·u = {
       .on_track = Core·on_track
      ,.derailed = Core·derailed
      ,.offset = Core·offset
      ,.offset_8AU = Core·offset_8AU
      ,.is_aligned_on_8AU = Core·is_aligned_on_8AU
      ,.floor_within_aligned_8AU = Core·floor_within_aligned_8AU
      ,.ceiling_within_aligned_8AU = Core·ceiling_within_aligned_8AU
    };

  //----------------------------------------
  // TM_NX 
  //----------------------------------------

  // The TM_NX has no destructive operations, so the tape will never get longer or shorter. Calls to 'mount_pe'  or 'mount_pp' are initialization operations.
  // hd == 0 is used to signal that the tape is unmounted or empty.

    typedef union{
      struct{
        AU *address;
      }array;
    }Core·TM_MX·Address;

    typedef union{
      struct{
        AU *position;
        extent_t extent;
        AU *hd;
      }array;
      struct{
        uint64_t *position;
        extent_t extent;
        uint64_t *hd;
      }array8AU;
    }Core·TM_NX;

  //----------------------------------------
  // TM_NX_Array: Array-Based Implementation
  //----------------------------------------

    // common error messages
    const char *Core·TM_NX_Array·Msg·tm = "given NULL tm";
    const char *Core·TM_NX_Array·Msg·flag = "given NULL flag pointer";
    const char *Core·TM_NX_Array·Msg·address = "given NULL address";
    const char *Core·TM_NX_Array·Msg·address_on_tape =
      "given address is not on the  tape"
      ;
    const char *Core·TM_NX_Array·Msg·extent = "given NULL extent pointer";

    // Area functions within Core·TM_NX_Array

    Local Core·Tape·Topo Core·TM_NX_Array·topo(Core·TM_NX *tm){
      if(!tm) return Core·Tape·Topo·nonexistent;
      if(!tm->position) return Core·Tape·Topo·empty;
      if(!tm->extent ) return Core·Tape·Topo·singleton;
      return Core·Tape·Topo·segment;
    }

    Local Core·Status Core·TM_NX_Array·extent(Core·TM_NX *tm ,extent_t *extent){
      Core·Tape·Topo t = Core·TM_NX_Array·topo(tm) & Core·Tape·Topo·finite_nz;
      #ifdef Core·Debug
        Core·Guard chk;
        Core·Guard·fg.init(&chk ,"Core·TM_NX_Array·extent" ,Core·Flag·count);
        Core·Guard·fg.check(
          &chk ,0 ,t 
          ,"Tape does not exist or topology does not have an extent."
        );
      #endif
      if(!t) return Core·Status·derailed;
      *extent = tm->extent;
      return Core·Status·on_track;
    }

    Local Core·Status Core·Area·mount_pe(
      Core·TM_NX *tm ,Core·TM_NX·Address *position ,extent_t extent
    ){
      #ifdef Core·Debug
        Core·Guard chk;
        Core·Guard·fg.init(&chk ,"Core·TM_NX_Array·mount_pp" ,Core·Flag·count);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,position ,"given NULL position");
        Core·Guard·if_return(chk);
      #endif
      area->array.position = position->array.address;
      area->array.extent = extent;
      return Core·Status·on_track;
    }

    Local Core·Status Core·Area·mount_pp(
      Core·TM_NX *tm ,Core·TM_NX·Address *position_left ,Core·TM_NX·Address *position_right
    ){
      #ifdef Core·Debug
        Core·Guard chk;
        Core·Guard·fg.init(&chk ,"Core·TM_NX_Array·mount_pp" ,Core·Flag·count);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,position_left ,"given NULL position_left");
        Core·Guard·fg.check(&chk ,1 ,position_right ,"given NULL position_right");
        Core·Guard·if_return(chk);
      #endif
      tm->array.position = position_left->array.address;
      tm->array.extent = 
          (extent_t)((uintptr_t)position_right->array.address
        - (uintptr_t)position_left->array.address)
        ;

      return Core·Status·on_track;
    }

    bool Core·TM_NX_Array·encloses_pt(Core·TM_NX *tm ,Core·TM_NX·Address *a){
      #ifdef Core·Debug
        Core·Guard chk;
        Core·Guard·fg.init(&chk ,"Core·TM_NX_Array·encloses_pt" ,Core·Flag·count);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,pt ,"given NULL pt");
        Core·Guard·if_return(chk);
      #endif

      return 
           (a->array.address >= tm->position) 
        && (a->array.address <= tm->position + tm->extent)
        ;
    }

    Local bool Core·TM_NX_Array·encloses_pt_strictly(Core·TM_NX *tm ,Core·TM_NX·Address *a){
      #ifdef Core·Debug
        Core·Guard chk;
        Core·Guard·fg.init(&chk ,"Core·TM_NX_Array·encloses_pt_strictly" ,Core·Flag·count);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,a ,Core·TM_NX_Array·Msg·address);
        Core·Guard·if_return(chk);
      #endif

      return (a->array.address > tm->array.position) 
          && (a->array.address < tm->array.position + tm->array.extent);
    }

    Local bool Core·TM_NX_Array·encloses_tm(Core·TM_NX *outer ,Core·TM_NX *inner){
      #ifdef Core·Debug
        Core·Guard chk;
        Core·Guard·fg.init(&chk ,"Core·TM_NX_Array·encloses_tm" ,Core·Flag·count);
        Core·Guard·fg.check(&chk ,1 ,outer ,"given NULL outer TM");
        Core·Guard·fg.check(&chk ,1 ,inner ,"given NULL inner TM");
        Core·Guard·if_return(chk);
      #endif

      return (inner->array.position >= outer->array.position) 
          && (inner->array.position + inner->array.extent <= outer->array.position + outer->array.extent);
    }

    Local bool Core·TM_NX_Array·encloses_tm_strictly(Core·TM_NX *outer ,Core·TM_NX *inner){
      #ifdef Core·Debug
        Core·Guard chk;
        Core·Guard·fg.init(&chk ,"Core·TM_NX_Array·encloses_tm_strictly" ,Core·Flag·count);
        Core·Guard·fg.check(&chk ,1 ,outer ,"given NULL outer TM");
        Core·Guard·fg.check(&chk ,1 ,inner ,"given NULL inner TM");
        Core·Guard·if_return(chk);
      #endif

      return (inner->array.position > outer->array.position) 
          && (inner->array.position + inner->array.extent < outer->array.position + outer->array.extent);
    }

    Local bool Core·TM_NX_Array·overlap(Core·TM_NX *a ,Core·TM_NX *b){
      #ifdef Core·Debug
        Core·Guard chk;
        Core·Guard·fg.init(&chk ,"Core·TM_NX_Array·overlap" ,Core·Flag·count);
        Core·Guard·fg.check(&chk ,1 ,a ,"given NULL TM A");
        Core·Guard·fg.check(&chk ,1 ,b ,"given NULL TM B");
        Core·Guard·if_return(chk);
      #endif

      return (a->array.position < b->array.position + b->array.extent) 
          && (b->array.position < a->array.position + a->array.extent);
    }

    Local Core·Status Core·TM_NX_Array·largest_aligned_64(
      Core·TM_NX *outer ,Core·TM_NX *inner_64
    ){
      #ifdef Core·Debug
        Core·Guard chk;
        Core·Guard·fg.init(&chk ,"Core·TM_NX_Array·largest_aligned_64" ,Core·Flag·count);
        Core·Guard·fg.check(&chk ,1 ,outer ,"given NULL outer TM");
        Core·Guard·fg.check(&chk ,1 ,inner_64 ,"given NULL inner TM");
        Core·Guard·if_return(chk);
      #endif

      uintptr_t p0 = (uintptr_t)outer->array.position;
      uintptr_t p1 = (uintptr_t)outer->array.position + outer->array.extent;

      AU *p0_64 = (AU *)( (p0 + 0x7) & ~(uintptr_t)0x7 );
      AU *p1_64 = (AU *)( (p1 - 0x7) & ~(uintptr_t)0x7 );

      if(p1_64 < p0_64){
        inner_64->array.position = NULL;
        inner_64->array.extent = 0;
        return Core·Status·on_track;
      }else{
        inner_64->array.position = p0_64;
        inner_64->array.extent = (extent_t)(p1_64 - p0_64);
        return Core·Status·on_track;
      }
    }



    //-----------------------------------
    // base Tape Machine operations

    Core·TM·Head·Status *Core·TM_NX_Array·mount(Core·TM_NX *tm){
      #ifdef Core·Debug
        Core·Guard chk;
        Core·Guard·fg.init(&chk ,"Core·TM_NX_Array·mount" ,Core·Flag·count);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·if_return(chk);
      #endif

      // Implementation-specific mounting logic
      tm->hd = tm->position;
      return &tm->status;
    }

    Core·TM·Head·Status *Core·TM_NX_Array·dismount(Core·TM_NX *tm){
      #ifdef Core·Debug
        Core·Guard chk;
        Core·Guard·fg.init(&chk ,"Core·TM_NX_Array·dismount" ,Core·Flag·count);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·if_return(chk);
      #endif

      // Reset head position upon dismount
      tm->hd = NULL;
      return &tm->status;
    }

    Core·TM·Head·Status Core·TM_NX_Array·status(Core·TM_NX *tm){
      if(!tm) return Core·TM·Head·Status·mu;
      if(tm->hd == NULL) return Core·TM·Head·Status·not_on_tape;
      if(tm->hd == tm->position) return Core·TM·Head·Status·origin;
      if( tm->hd == (tm->position + tm->extent - 1) ) 
        return Core·TM·Head·Status·rightmost;
      return Core·TM·Head·Status·interim;
    }

    Local Core·Status Core·TM_NX_Array·can_read(Core·TM_NX *tm ,bool *flag){
      *flag = tm && tm->hd != NULL;
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·on_origin(
      Core·TM_NX *tm ,bool *flag
    ){
      #ifdef Core·Debug
        Core·Guard chk; 
        Core·Guard·fg.init(&chk ,"Core·TM_NX_Array·on_origin" ,Core·Flag·count);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,flag ,Core·TM_NX_Array·Msg·flag);
        Core·Guard·if_return(chk);
      #endif

      *flag = (tm->hd == tm->position);
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·on_rightmost(
      Core·TM_NX *tm ,bool *flag
    ){
      #ifdef Core·Debug
        Core·Guard chk; 
        Core·Guard·fg.init(&chk ,"Core·TM_NX_Array·on_rightmost" ,Core·Flag·count);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,flag ,Core·TM_NX_Array·Msg·flag);
        Core·Guard·if_return(chk);
      #endif

      *flag = tm->hd == (tm->address + tm->extent)
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·read(
      Core·TM_NX *tm ,Core·TM_NX·Address *a ,Core·Tape·Remote *remote
    ){
      #ifdef Core·Debug
        Core·Guard chk; 
        Core·Guard·fg.init(&chk ,"Core·TM_NX_Array·on_rightmost" ,Core·Flag·count);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,flag ,Core·TM_NX_Array·Msg·flag);
        Core·Guard·if_return(chk);
      #endif

      #ifdef Core·Debug
        uint error = 0;
        if( !tm ){
          fprintf(stderr,"Core·TM_NX_Array·read:: given NULL tm");
          error++;
        }else if( !tm->address ){
          fprintf(stderr,"Core·TM_NX_Array·read:: read requested for empty tape");
          error++;
        }
        if( !a ){
          fprintf(stderr,"Core·TM_NX_Array·read:: given NULL address");
          error++;
        }else if( !Core·TM_NX_Array·area.encloses_pt(tm ,a) ){
          fprintf(stderr,"Core·TM_NX_Array·read:: address is outside tape bounds");
          error++;
        }
        if( !remote ){
          fprintf(stderr,"Core·TM_NX_Array·read:: given NULL remote");
          error++;
        }
        if( error ) return Core·Status·derailed;
      #endif
      *(AU *)remote->address = *(AU *)a->address;
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·write(
      Core·TM_NX *tm ,Core·TM_NX·Address *a ,Core·Tape·Remote *r
    ){
      #ifdef Core·Debug
        uint error = 0;
        if( !tm ){
          fprintf(stderr,"Core·TM_NX_Array·write:: given NULL tm");
          error++;
        }else if( !tm->address ){
          fprintf(stderr,"Core·TM_NX_Array·write:: write requested for empty tape");
          error++;
        }
        if( !a ){
          fprintf(stderr,"Core·TM_NX_Array·write:: given NULL address");
          error++;
        }else if( !Core·TM_NX_Array·area.encloses_pt(tm ,a) ){
          fprintf(stderr,"Core·TM_NX_Array·write:: address is outside tape bounds");
          error++;
        }
        if( !r ){
          fprintf(stderr,"Core·TM_NX_Array·write:: given NULL remote");
          error++;
        }
        if( error ) return Core·Status·derailed;
      #endif
      *(AU *)a->address = *(AU *)r->address;
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·rewind(Core·TM_NX *tm){
      #ifdef Core·Debug
        uint error = 0;
        if( !tm ){
          fprintf(stderr,"Core·TM_NX_Array·rewind:: given NULL tm");
          error++;
        }else if( !tm->array->position ){
          fprintf(stderr,"Core·TM_NX_Array·rewind:: no tape");
          error++;
        }
        if( error ) return Core·Status·derailed;
      #endif
      tm->array->hd = tm->array->position;
      return Core·Status·on_track;
    }

    Core·Status Core·TM_NX_Array·step(Core·TM_NX *tm){
    }
    Core·Status Core·TM_NX_Array·step_left(Core·TM_NX *tm){
    }
    // step_right is a synonym for step




    

    //----------------------------------------
    // FG Table Initialization for Array Implementation

    Local Core·TM_NX·FG Core·TM_NX_Array·fg = {
       .mount = Core·TM_NX_Array·mount
      ,.dismount = Core·TM_NX_Array·dismount
      ,.rewind = Core·TM_NX_Array·rewind

      ,.can_step = Core·TM_NX_Array·can_step
      ,.on_origin = Core·TM_NX_Array·on_origin
      ,.on_rightmost = Core·TM_NX_Array·on_rightmost

      ,.extent = Core·TM_NX_Array·extent
      ,.can_read = Core·TM_NX_Array·can_read

      ,.step = Core·TM_NX_Array·step
      ,.step_left = Core·TM_NX_Array·step_left
      ,.step_right = Core·TM_NX_Array·step_right
      ,.status = Core·TM_NX_Array·status

      ,.area {
         .topo = Core·TM_NX_Array·topo
        ,.extent = Core·TM_NX_Array·extent
        ,.is_origin = Core·TM_NX_Array·is_origin
        ,.is_rightmost = Core·TM_NX_Array·is_rightmost
        ,.read = Core·TM_NX_Array·read
        ,.write = Core·TM_NX_Array·write

        ,.init_pe = Core·TM_NX_Array·init_pe
        ,.init_pp = Core·TM_NX_Array·init_pp
        ,.set_position = Core·TM_NX_Array·set_position
        ,.set_position_left = Core·TM_NX_Array·set_position
        ,.set_position_right = Core·TM_NX_Array·set_position_right
        ,.set_extent = Core·TM_NX_Array·set_extent

        ,.position = Core·TM_NX_Array·position
        ,.position_left = Core·TM_NX_Array·position // synonym
        ,.position_right = Core·TM_NX_Array·position_right

        ,.complement = Core·TM_NX_Array·complement

        ,.encloses_pt = Core·TM_NX_Array·encloses_pt
        ,.encloses_tm = Core·TM_NX_Array·encloses_tm
        ,.overlap = Core·TM_NX_Array·overlap
        ,.largest_aligned_64 = Core·TM_NX_Array·largest_aligned_64
      }
    };

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
