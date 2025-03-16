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

   FG FunctionsGiven<type>, e.g. FunctionsGiven<Core·TM_NX_Array>, abbreviate as FG, or fg.
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
  // memory interface

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
  // argument guard interface

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

    #define Core·Guard·init_count(chk) \
      Core·Guard chk; \
      Core·Guard·fg.init(&chk ,__func__ ,Core·Flag·count);

    #define Core·Guard·init_collect(chk) \
      Core·Guard chk; \
      Core·Guard·fg.init(&chk ,__func__ ,Core·Flag·collect);

  //----------------------------------------
  // functions interface

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
  // Tape Machine interface

    typedef struct Core·TM_NX;
    typedef struct Core·TM_NX·Address;
    typedef struct Core·TM_NX·Remote;

    /*
      For an Array machine tape, a mounted tape will be singleton or segment.
    */
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
      Core·Status (*mount)(Core·TM_NX *tm);
      Core·Status (*dismount)(Core·TM_NX *tm);

      Core·Status (*status)(Core·TM_NX *tm ,Core·TM·Head·Status *status);
      Core·Status (*can_read)(Core·TM_NX *tm ,bool *flag);
      Core·Status (*on_origin)(Core·TM_NX *tm ,bool *flag);
      Core·Status (*on_rightmost)(Core·TM_NX *tm ,bool *flag);

      Core·Status (*read)(
        Core·TM_NX *tm ,Core·TM_NX·Address *a ,TM_MX·Remote *remote
      );
      Core·Status (*write)(
        Core·TM_NX *tm ,Core·TM_NX·Address *a ,TM_MX·Remote *remote
      );

      Core·Status (*rewind)(Core·TM_NX *tm);
      Core·Status (*step)(Core·TM_NX *tm);
      Core·Status (*step_left)(Core·TM_NX *tm);
      Core·Status (*step_right)(Core·TM_NX *tm); // Synonym for step

      struct {
        Core·Status (*topo)(Core·TM_NX *tm ,Core·Tape·Topo *topo);
        Core·Status (*extent)(Core·TM_NX *tm ,extent_t *extent_pt);

        // Initialize tm
        Core·Status (*mount_pe)(
           Core·TM_NX *tm 
          ,Core·TM_NX·Address *position 
          ,extent_t extent
        );
        Core·Status (*mount_pp)(
           Core·TM_NX *tm 
          ,Core·TM_NX·Address *position_left 
          ,Core·TM_NX·Address *position_right
        );

        // initializes inner_64
        Core·Status (*largest_aligned_64)(Core·TM_NX *outer ,Core·TM_NX *inner_64);

        Core·Status (*encloses_pt)(
          Core·TM_NX *tm ,Core·TM_NX·Address *pt ,bool *result
        );
        Core·Status (*encloses_pt_strictly)(
          Core·TM_NX *tm ,Core·TM_NX·Address *pt ,bool *result
        );
        Core·Status (*encloses_tm)(
          Core·TM_NX *outer ,Core·TM_NX *inner ,bool *flag
        );
        Core·Status (*encloses_tm_strictly)(
          Core·TM_NX *outer ,Core·TM_NX *inner ,bool *flag
        );
        Core·Status (*overlap)(
          Core·TM_NX *a ,Core·TM_NX *b ,bool *result
        );
      } area;

    } Core·TM_NX·FG;


  //----------------------------------------
  // Map interface

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

  /*
  TM_NX_Array are initialized with calls to 'mount_pe' or 'mount_pp'.  These both bind the machine to a tape, and mount the tape. Hence, there is no such thing as an initialized TM_NX_Array which is not bound to a tape. (It is possible to dismount the tm->hd, then remount tm->hd, but the tape remains bound through that process.)

  The tm->position variable can not be used to reliably check if a tape machine has been bound to a tape, because after initialization it will have a value, and before initialization,
  God only knows what value it will carry, and he doesn't tell anyone but the most devout.

  Because the TM_NX has no destructive operations, Once it is initialized the tape will never get longer or shorter. With TM_MX_Array it is not possible to mount an empty tape, because the minimum value of extent is zero.

  It is an error to re-initialize the machine while it is being used, unfortunately this is a contract with the user, as we have no means in C bind the allocation and initialization steps. Of course using an uninitialized TM_NX_Array will lead to unpredictable behavior.
  
  TM_NX.array.position == NULL means the machine has not been bound to a tape.
  TM_NX.array.hd == NULL means the tape is currently not mounted.
  */

    typedef union{
      struct{
        AU *address;
      }array;
    }Core·TM_MX·Address;

    typedef union{
      struct{
        AU *address;
      }array;
    }Core·TM_MX·Remote;

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
    const char *Core·TM_NX_Array·Msg·address = 
      "given NULL address pointer, or address struct holds NULL address"
      ;
    const char *Core·TM_NX_Array·Msg·address_on_tape =
      "given address is not on the tape"
      ;
    const char *Core·TM_NX_Array·Msg·extent = "given NULL extent pointer";
    const char *Core·TM_NX_Array·Msg·position = "Null position. This is only possible when the tape machine has not been initialized.";

    //-----------------------------------
    // Area functions within Core·TM_NX_Array

    Core·Status Core·TM_NX_Array·topo(Core·TM_NX *tm ,Core·Tape·Topo *topo){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,topo ,"topo ptr is NULL, so nowhere to put result");
        Core·Guard·if_return(chk);
      #endif
        if(tm->extent == 0){
          *topo = Core·Tape·Topo·singleton; 
        }else{
          *topo = Core·Tape·Topo·segment;
        }
        return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·extent(Core·TM_NX *tm ,extent_t *extent){
      Core·Tape·Topo topo;
      Core·Status status = Core·TM_NX_Array·topo(tm ,&topo);
      boolean good_topo = 
        (status == Core·Status·on_track) && (topo & Core·Tape·Topo·finite_nz)
        ;

      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,extent ,Core·TM_NX_Array·Msg·extent);
        Core·Guard·fg.check(
          &chk ,0 ,good_topo
          ,"Tape does not exist or topology does not have an extent."
        );
        Core·Guard·if_return(chk);
      #endif

      if(!good_topo) return Core·Status·derailed;
      *extent = tm->array.extent;
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·mount_pe(
      Core·TM_NX *tm ,Core·TM_NX·Address *position ,extent_t extent
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,position ,"given NULL position");
        Core·Guard·if_return(chk);
      #endif
      tm->array.position = position->array.address;
      tm->array.extent = extent;
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·mount_pp(
      Core·TM_NX *tm ,Core·TM_NX·Address *position_left ,Core·TM_NX·Address *position_right
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        bool good_address = position_left && position_left->address;
        Core·Guard·fg.check(&chk ,1 ,good_address ,Core·TM_NX_Array·Msg·address);
        good_address = position_right && position_right->address;
        Core·Guard·fg.check(&chk ,1 ,good_address ,Core·TM_NX_Array·Msg·address);
        if(position_left && position_right){
          Core·Guard·fg.check(
            &chk ,1 ,position_right->array.address >= position_left->array.address
            ,"position_right < position_left"
          );
        }
        Core·Guard·if_return(chk);
      #endif

      extent_t computed_extent = 
          (extent_t)(
            (uintptr_t)position_right->array.address - (uintptr_t)position_left->array.address
          );

      return Core·TM_NX_Array·mount_pe(tm ,position_left ,computed_extent);
    }

    Local Core·Status Core·TM_NX_Array·largest_aligned_64(
      Core·TM_NX *outer ,Core·TM_NX *inner_64
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,outer ,"given NULL outer TM");
        if(outer){
          Core·Guard·fg.check(&chk ,1 ,outer->array.position ,"NULL outer->array.position");
        }
        Core·Guard·fg.check(&chk ,1 ,inner_64 ,"given NULL inner TM");
        Core·Guard·fg.check(&chk ,1 ,outer->array.position ,"outer TM has NULL position");
        Core·Guard·if_return(chk);
      #endif

      uintptr_t p0 = (uintptr_t)outer->array.position;
      uintptr_t p1 = (uintptr_t)outer->array.position + outer->array.extent;

      AU *p0_64 = (AU *)( (p0 + 0x7) & ~(uintptr_t)0x7 );
      AU *p1_64 = (AU *)( (p1 - 0x7) & ~(uintptr_t)0x7 );

      if(p1_64 < p0_64){
        inner_64->array.position = NULL;
        inner_64->array.extent = 0;
        return Core·Status·derailed;
      }

      inner_64->array.position = p0_64;
      inner_64->array.extent = (extent_t)(p1_64 - p0_64);
      return Core·Status·on_track;
    }


    Local Core·Status Core·TM_NX_Array·encloses_pt(
      Core·TM_NX *tm ,Core·TM_NX·Address *a ,bool *result
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        bool good_address = a && a->address;
        Core·Guard·fg.check(&chk ,1 ,good_address ,Core·TM_NX_Array·Msg·address);
        Core·Guard·fg.check(&chk ,1 ,result ,"given NULL result pointer");
        Core·Guard·if_return(chk);
      #endif

      *result = 
           (a->array.address >= tm->array.position) 
        && (a->array.address <= tm->array.position + tm->array.extent);

      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·encloses_pt_strictly(
      Core·TM_NX *tm ,Core·TM_NX·Address *a ,bool *result
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        bool good_address = a && a->address;
        Core·Guard·fg.check(&chk ,1 ,good_address ,Core·TM_NX_Array·Msg·address);
        Core·Guard·fg.check(&chk ,1 ,result ,"given NULL result pointer");
        Core·Guard·if_return(chk);
      #endif

      *result = 
           (a->array.address > tm->array.position) 
        && (a->array.address < tm->array.position + tm->array.extent);

      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·encloses_tm(
      Core·TM_NX *outer ,Core·TM_NX *inner ,bool *flag
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,outer ,"given NULL outer TM");
        if(outer){
          Core·Guard·fg.check(&chk ,1 ,outer->array.position ,"NULL outer->array.position");
        }
        Core·Guard·fg.check(&chk ,1 ,inner ,"given NULL inner TM");
        if(inner){
          Core·Guard·fg.check(&chk ,1 ,inner->array.position ,"NULL inner->array.position");
        }
        Core·Guard·fg.check(&chk ,1 ,flag ,"given NULL flag pointer");
        Core·Guard·if_return(chk);
      #endif

      *flag =
           (inner->array.position >= outer->array.position) 
        && (inner->array.position + inner->array.extent <= outer->array.position + outer->array.extent);

      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·encloses_tm_strictly(
      Core·TM_NX *outer ,Core·TM_NX *inner ,bool *flag
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,outer ,"given NULL outer TM");
        if(outer){
          Core·Guard·fg.check(&chk ,1 ,outer->array.position ,"NULL outer->array.position");
        }
        Core·Guard·fg.check(&chk ,1 ,inner ,"given NULL inner TM");
        if(inner){
          Core·Guard·fg.check(&chk ,1 ,inner->array.position ,"NULL inner->array.position");
        }
        Core·Guard·fg.check(&chk ,1 ,flag ,"given NULL flag pointer");
        Core·Guard·if_return(chk);
      #endif

      *flag =
           (inner->array.position > outer->array.position) 
        && (inner->array.position + inner->array.extent < outer->array.position + outer->array.extent);

      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·overlap(
      Core·TM_NX *a ,Core·TM_NX *b ,bool *result
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,a ,"given NULL TM A");
        if(a){
          Core·Guard·fg.check(&chk ,1 ,a->array.position ,"NULL a->array.position");
        }
        Core·Guard·fg.check(&chk ,1 ,b ,"given NULL TM B");
        if(b){
          Core·Guard·fg.check(&chk ,1 ,b->array.position ,"NULL b->array.position");
        }
        Core·Guard·fg.check(&chk ,1 ,result ,"given NULL result pointer");
        Core·Guard·if_return(chk);
      #endif

      *result =
           (a->array.position < b->array.position + b->array.extent) 
        && (b->array.position < a->array.position + a->array.extent);

      return Core·Status·on_track;
    }


    //-----------------------------------
    // base Tape Machine operations

    Local Core·Status Core·TM_NX_Array·mount(Core·TM_NX *tm){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,tm->array.position ,Core·TM_NX_Array·Msg·position);
        Core·Guard·if_return(chk);
      #endif

      tm->array.hd = tm->array.position;
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·dismount(Core·TM_NX *tm){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·if_return(chk);
      #endif

      // Reset head position upon dismount
      tm->array.hd = NULL;
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·status(
      Core·TM_NX *tm ,Core·TM·Head·Status *status
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,status ,"given NULL status pointer");
        Core·Guard·if_return(chk);
      #endif

      if(tm->array.hd == NULL){
        *status = Core·TM·Head·Status·not_on_tape;
      }else if(tm->array.hd == tm->array.position){
        *status = Core·TM·Head·Status·origin;
      }else if(tm->array.hd == tm->array.position + tm->array.extent){
        *status = Core·TM·Head·Status·rightmost;
      }else{
        *status = Core·TM·Head·Status·interim;
      }

      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·can_read(Core·TM_NX *tm ,bool *flag){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        if(tm){
          // All initialized TM_NX_Array have an initialized position
          // Maybe this catches that the machine is uninitialized?
          Core·Guard·fg.check(&chk ,1 ,tm->array.position ,Core·TM_NX_Array·Msg·position);
        }      
        Core·Guard·fg.check(&chk ,1 ,flag ,Core·TM_NX_Array·Msg·flag);
        Core·Guard·if_return(chk);
      #endif
      *flag = tm && tm->array.hd != NULL;
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·on_origin(
      Core·TM_NX *tm ,bool *flag
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,tm && tm->array.position ,Core·TM_NX_Array·Msg·position);
        Core·Guard·fg.check(&chk ,1 ,flag ,Core·TM_NX_Array·Msg·flag);
        Core·Guard·if_return(chk);
      #endif

      *flag = (tm->array.hd == tm->array.position);
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·on_rightmost(
      Core·TM_NX *tm ,bool *flag
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,flag ,Core·TM_NX_Array·Msg·flag);
        Core·Guard·fg.check(&chk ,1 ,tm && tm->array.position ,Core·TM_NX_Array·Msg·position);
        Core·Guard·if_return(chk);
      #endif

      *flag = tm->array.hd == (tm->array.position + tm->array.extent);
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·read(
      Core·TM_NX *tm ,Core·TM_NX·Address *a ,TM_MX·Remote *r
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,a ,Core·TM_NX_Array·Msg·address);
        Core·Guard·fg.check(&chk ,1 ,r ,Core·TM_NX_Array·Msg·address);
        if(tm && a){
          Core·Guard·fg.check(
            &chk ,1 ,Core·TM_NX_Array·area.encloses_pt(tm ,a) 
            ,"Given address is outside tape bounds"
          );
        }
        Core·Guard·if_return(chk);
      #endif

      *(r->address) = *(a->address);
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·write(
      Core·TM_NX *tm ,Core·TM_NX·Address *a ,TM_MX·Remote *r
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,a ,Core·TM_NX_Array·Msg·address);
        Core·Guard·fg.check(&chk ,1 ,r ,Core·TM_NX_Array·Msg·address);
        if(tm && a){
          Core·Guard·fg.check(
            &chk ,1 ,Core·TM_NX_Array·area.encloses_pt(tm ,a) 
            ,"Given address is outside tape bounds"
          );
        }
        Core·Guard·if_return(chk);
      #endif

      *(a->address) = *(r->address);
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_Array·rewind(Core·TM_NX *tm){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        if(tm){
          Core·Guard·fg.check(&chk ,1 ,tm->array.position ,Core·TM_NX_Array·Msg·position);
        }
        Core·Guard·if_return(chk);
      #endif
      tm->array.hd = tm->array.position;
      return Core·Status·on_track;
    }

    Core·Status Core·TM_NX_Array·step(Core·TM_NX *tm){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,tm->array.position ,"step requested on unbound machine");
        Core·Guard·fg.check(&chk ,1 ,tm->array.hd ,"step requested but tape not mounted");
        Core·Guard·if_return(chk);
      #endif
      if( tm->array.hd < tm->array.position + tm->array.extent ){
        tm->array.hd++;
        return Core·Status·on_track;
      }
      return Core·Status·derailed; // Stepping beyond tape bounds
    }

    Core·Status Core·TM_NX_Array·step_left(Core·TM_NX *tm){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_Array·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,tm->array.position 
          ,"step_left requested on unbound machine"
        );
        Core·Guard·fg.check(&chk ,1 ,tm->array.hd 
          ,"step_left requested with no mounted head"
        );
        Core·Guard·if_return(chk);
      #endif
      if( tm->array.hd > tm->array.position ){
        tm->array.hd--;
        return Core·Status·on_track;
      }
      return Core·Status·derailed; // Stepping beyond left boundary
    }

    // step_right is a synonym for step


    //----------------------------------------
    // Initialization for Core·TM_NX_Array·fg

    Local Core·TM_NX·FG Core·TM_NX_Array·fg = {
       .mount = Core·TM_NX_Array·mount
      ,.dismount = Core·TM_NX_Array·dismount

      ,.status = Core·TM_NX_Array·status
      ,.can_read = Core·TM_NX_Array·can_read
      ,.on_origin = Core·TM_NX_Array·on_origin
      ,.on_rightmost = Core·TM_NX_Array·on_rightmost

      ,.read = Core·TM_NX_Array·read
      ,.write = Core·TM_NX_Array·write

      ,.rewind = Core·TM_NX_Array·rewind
      ,.step = Core·TM_NX_Array·step
      ,.step_left = Core·TM_NX_Array·step_left
      ,.step_right = Core·TM_NX_Array·step_right // Synonym for step

      ,.area = {
         .topo = Core·TM_NX_Array·topo
        ,.extent = Core·TM_NX_Array·extent

        ,.mount_pe = Core·TM_NX_Array·mount_pe
        ,.mount_pp = Core·TM_NX_Array·mount_pp
        ,.largest_aligned_64 = Core·TM_NX_Array·largest_aligned_64

        ,.encloses_pt = Core·TM_NX_Array·encloses_pt
        ,.encloses_pt_strictly = Core·TM_NX_Array·encloses_pt_strictly
        ,.encloses_tm = Core·TM_NX_Array·encloses_tm
        ,.encloses_tm_strictly = Core·TM_NX_Array·encloses_tm_strictly
        ,.overlap = Core·TM_NX_Array·overlap
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
      if(Core·TM_NX_Array·extent(Core·tf.copy.read) == Core·TM_NX_Array·extent(Core·tf.copy.write))
        return Core·Map·ByteByByte·perfect_fit;

      if(Core·TM_NX_Array·extent(Core·tf.copy.read) > Core·TM_NX_Array·extent(Core·tf.copy.write))
        return Core·Map·ByteByByte·read_surplus;

      return Core·Map·ByteByByte·write_available;
    }

    Local Core·Map·Fn Core·Map·ByteByByte·perfect_fit(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·TM_NX_Array·position_right(Core·tf.copy.read);
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
      AU *r1 = Core·TM_NX_Array·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;
      AU *w1 = Core·TM_NX_Array·position_right(Core·tf.copy.write);

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
      AU *r1 = Core·TM_NX_Array·position_right(Core·tf.copy.read);
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
      Core·TM_NX_Array·largest_aligned_64(Core·tf.copy.read ,&Core·tl.copy_64.area_64);

      // Choose the correct function based on alignment
      if(Core·TM_NX_Array·empty(&Core·tl.copy_64.area_64)) return Core·Map·ByWord64·tail;
      if(Core·is_aligned_on_64(Core·TM_NX_Array·position(Core·tf.copy.read))) return Core·Map·ByWord64·bulk;
      return Core·Map·ByWord64·leadin;
    }

    // Lead-in byte copy (until alignment)
    Local Core·Map·Fn Core·Map·ByWord64·leadin(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r0_64 = Core·TM_NX_Array·position(&Core·tl.copy_64.area_64);
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
      uint64_t *r1_64 = Core·TM_NX_Array·position_right(&Core·tl.copy_64.area_64);

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
      AU *r1 = Core·TM_NX_Array·position_right(&Core·tl.copy_64.area_64);
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
      if(Core·TM_NX_Array·extent(Core·tf.copy.read) == (Core·TM_NX_Array·extent(Core·tf.copy.write) >> 1)){
        return Core·Map·WriteHex·perfect_fit;
      }
      if(Core·TM_NX_Array·extent(Core·tf.copy.read) > (Core·TM_NX_Array·extent(Core·tf.copy.write) >> 1)){
        return Core·Map·WriteHex·read_surplus;
      }
      return Core·Map·WriteHex·write_available;
    }

    Local Core·Map·Fn Core·Map·WriteHex·perfect_fit(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·TM_NX_Array·position_right(Core·tf.copy.read);
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
      AU *r1 = Core·TM_NX_Array·position_right(Core·tf.copy.write);
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
      AU *r1 = Core·TM_NX_Array·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;
      AU *w1 = Core·TM_NX_Array·position_right(Core·tf.copy.write);

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
      if((Core·TM_NX_Array·extent(Core·tf.copy.read) >> 1) == Core·TM_NX_Array·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·perfect_fit;
      }
      if((Core·TM_NX_Array·extent(Core·tf.copy.read) >> 1) > Core·TM_NX_Array·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·read_surplus;
      }
      return Core·Map·ReadHex·write_available;
    }

    Local Core·Map·Fn Core·Map·ReadHex·perfect_fit(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·TM_NX_Array·position_right(Core·tf.copy.read);
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
      AU *r1 = Core·TM_NX_Array·position_right(Core·tf.copy.write);
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
      AU *r1 = Core·TM_NX_Array·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;
      AU *w1 = Core·TM_NX_Array·position_right(Core·tf.copy.write);

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
      if((Core·TM_NX_Array·extent(Core·tf.copy.read) >> 1) == Core·TM_NX_Array·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·perfect_fit;
      }
      if((Core·TM_NX_Array·extent(Core·tf.copy.read) >> 1) > Core·TM_NX_Array·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·read_surplus;
      }
      return Core·Map·ReadHex·write_available;
    }

    Local Core·Map·Fn Core·Map·ReadHex·perfect_fit(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·TM_NX_Array·position_right(Core·tf.copy.read);
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
      AU *r1 = Core·TM_NX_Array·position_right(Core·tf.copy.write);
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
      AU *r1 = Core·TM_NX_Array·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;
      AU *w1 = Core·TM_NX_Array·position_right(Core·tf.copy.write);

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
          ,.read_fn_8 = Core·TM_NX_Array·read_8_fwd
          ,.read_fn_64 = Core·TM_NX_Array·read_64_fwd
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
      .Area·init_pe = Core·TM_NX_Array·init_pe
      ,.Area·init_pp = Core·TM_NX_Array·init_pp
      ,.Area·set_position = Core·TM_NX_Array·set_position
      ,.Area·set_position_left = Core·TM_NX_Array·set_position
      ,.Area·set_position_right = Core·TM_NX_Array·set_position_right
      ,.Area·set_extent = Core·TM_NX_Array·set_extent
      ,.Area·position = Core·TM_NX_Array·position
      ,.Area·position_left = Core·TM_NX_Array·position
      ,.Area·position_right = Core·TM_NX_Array·position_right
      ,.Area·extent = Core·TM_NX_Array·extent
      ,.Area·length_Kung = Core·TM_NX_Array·length_Kung
      ,.Area·empty = Core·TM_NX_Array·empty

      ,.Area·encloses_pt = Core·TM_NX_Array·encloses_pt
      ,.Area·encloses_pt_strictly = Core·TM_NX_Array·encloses_pt_strictly
      ,.Area·encloses_area = Core·TM_NX_Array·encloses_area
      ,.Area·encloses_area_strictly = Core·TM_NX_Array·encloses_area_strictly
      ,.Area·overlap = Core·TM_NX_Array·overlap
      ,.Area·largest_aligned_64 = Core·TM_NX_Array·largest_aligned_64
      ,.Area·complement = Core·TM_NX_Array·complement

      ,.Area·read_8_zero = Core·TM_NX_Array·read_8_zero
      ,.Area·read_8_fwd = Core·TM_NX_Array·read_8_fwd
      ,.Area·read_8_rev = Core·TM_NX_Array·read_8_rev
      ,.Area·read_64_zero = Core·TM_NX_Array·read_64_zero
      ,.Area·read_64_fwd = Core·TM_NX_Array·read_64_fwd
      ,.Area·read_64_rev = Core·TM_NX_Array·read_64_rev

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
