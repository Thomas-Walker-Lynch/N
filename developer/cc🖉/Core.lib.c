 /*
  Core - core memory operations.

  This file has the template parameter: `CVT`

  `CVT` Cell Value Type`. It is the type of the datum placed in a cell. By default (when the   macro CVT has no definition) the cell value type is taken as AUM. This file must
  be included with CVT undefined, before inclusions with it defined.
 
  `AU` `Addressable Unit for the machine`, on most all machines today this is uint8_t,
  though the C standard has never required this.

  'ATP' `At This Point' in the code. Acronym used in comments usually before pointing
  out certain values variables must have.

  'Tape' is operated on by the Tape Machine.

  'Area' is subset of an address space that is used as a virtual Tape by a machine.

   An Area with zero elements has 'length == 0' or is 'empty'. In contrast, and
   area located (position specified) with a null pointer is said not to exist.

   When all the functions that have a specific type of operand given are group together
   in a table, we call the table a 'Function Given Type X', table. In general we call
   these FG (Function Given) tables. A specific instance of an FG table is an `fg` table.

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
  //----------------------------------------

  #ifndef CVT  

    // Expand namespace with a macro parameter given value.
    // The namespace and the extended value will be evaluated for macros.
    // Greek capital letter 'Xi' for eXtend.
    #define _Ξ(a ,b) a##·##b
    #define Ξ(a ,b) _Ξ(a ,b)

    // reminds me of FORTRAN star types
    // C language did not guarantee 8 bit bytes, which is why Unicode speaks of octets
    #define AU AU
    #define AU2 uint16_t
    #define UA4 uint32_t
    #define UA8 uint64_t

    #define AU_MAX ~(AU)0
    #define AU2_MAX ~(AU2)0
    #define UA4_MAX ~(AU4)0
    #define UA8_MAX ~(AU8)0

    // ask the compiler what this really is
    // when using enums we get this whether we want them or not
    #define WU unsigned int
    #define WU ~(WU)0

    // 'extent of an object' is the maximum AU index within an object. For example the extent of AU4 is 3.
    #define extentof(x)(sizeof(x) - 1)
    #define extent_t size_t

    typedef enum{
       Core·Status·mu = 0
      ,Core·Status·on_track
      ,Core·Status·derailed
    }Core·Status;

  #endif

  //----------------------------------------
  // argument guard interface
  //----------------------------------------

  #ifndef CVT  

    typedef void (*Flag·Function)(WU *flag ,WU err);

    void Core·Flag·count(WU *flag ,WU err){
      if(err == WU_MAX){ *flag = WU_MAX; return;}

      //*flag + err > WU_MAX
      if(*flag > WU_MAX - err){ *flag = WU_MAX; return;}

      (*flag) += err;
    }

    void Core·Flag·collect(WU *flag ,WU err){
      (*flag) |= err;
    }

    typedef struct {
      char *name;
      Flag·Function flag_function;
      WU flag;
    } Core·Guard;

    typedef struct {
      void (*init)(Core·Guard *chk ,const char *name ,Flag·Function af);
      void (*reset)(Core·Guard *chk);
      void (*check)(
         Core·Guard *chk
        ,WU err
        ,bool condition
        ,char *message
      );
    } Core·Guard·FG;

    // Default guard function table
    // initialized in the implementation section below
    Local Core·Guard·FG Core·Guard·fg;

    #define Core·Guard·init_count(chk) \
      Core·Guard chk; \
      Core·Guard·fg.init(&chk ,__func__ ,Core·Flag·count);

    #define Core·Guard·init_collect(chk) \
      Core·Guard chk; \
      Core·Guard·fg.init(&chk ,__func__ ,Core·Flag·collect);

    #define Core·Guard·if_return(chk) if( chk.flag ) return Core·Status·derailed;
    #define Core·Guard·return(chk)\
      if( chk.flag ) return Core·Status·derailed;\
      else return Core·Status·on_track;

  #endif 

  //----------------------------------------
  // functions interface
  //----------------------------------------
  
  // no state, this is merely a namespace

  #ifndef CVT
    typedef struct{
      Core·Status (*on_track)();
      Core·Status (*derailed)();
      AU *(*offset)(AU *p ,size_t Δ);
      bool is_aligned_on(void *p ,extent_t alignment);
    } Core·F;
    Local Core·F Core·f;
  #endif 

  #ifdef CVT
    typedef struct{
      CVT *(*offset)(CVT *p ,size_t Δ);
      AU *(*floor_within_aligned)(CVT *p);
      AU *(*ceiling_within_aligned)(CVT *p);
    } Ξ(Core·F ,CVT);
    Local Ξ(Core·F ,CVT) Ξ(Core·F ,CVT)·f;
  #endif

  //----------------------------------------
  // Tape Machine interface
  //----------------------------------------

  #ifndef CVT

    typedef struct Core·TM_NX;

    /*
      For an CVT machine tape, a mounted tape will be singleton or segment.
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

    // tape and area are included with Tape Machine to facilitate abstract interfaces.
    typedef struct{

      struct {
        Core·Status (*topo)(Core·TM_NX *tm ,Core·Tape·Topo *topo);
        Core·Status (*extent)(Core·TM_NX *tm ,extent_t *extent_pt);
      } tape;

      struct {
        // Initialize tm
        Core·Status (*mount_pe)(Core·TM_NX *tm ,AU position[] ,extent_t extent);
        Core·Status (*mount_pp)(Core·TM_NX *tm ,AU *position_left ,AU *position_right);
  
        // predicates
        Core·Status (*encloses_pt)(Core·TM_NX *tm ,AU *pt ,bool *result);
        Core·Status (*encloses_pt_strictly)(Core·TM_NX *tm ,AU *pt ,bool *result);
        Core·Status (*encloses_area)(Core·TM_NX *outer ,Core·TM_NX *inner ,bool *flag);
        Core·Status (*encloses_area_strictly)(
          Core·TM_NX *outer ,Core·TM_NX *inner ,bool *flag
        );
        Core·Status (*overlap)(Core·TM_NX *a ,Core·TM_NX *b ,bool *result);
      } area;

      // tape machine functions
      Core·Status (*mount)(Core·TM_NX *tm);
      Core·Status (*dismount)(Core·TM_NX *tm);

      Core·Status (*status)(Core·TM_NX *tm ,Core·TM·Head·Status *status);
      Core·Status (*can_read)(Core·TM_NX *tm ,bool *flag);
      Core·Status (*on_origin)(Core·TM_NX *tm ,bool *flag);
      Core·Status (*on_rightmost)(Core·TM_NX *tm ,bool *flag);

      Core·Status (*copy_datum)(Core·TM_NX *tm_read ,Core·TM_NX *tm_write);
      Core·Status (*read)(Core·TM_NX *tm ,AU *read_pt);
      Core·Status (*write)(Core·TM_NX *tm ,AU *write_pt);
      Core·Status (*head_pt)(Core·TM_NX *tm ,AU *head_pt);

      Core·Status (*rewind)(Core·TM_NX *tm);
      Core·Status (*step)(Core·TM_NX *tm);
      Core·Status (*step_left)(Core·TM_NX *tm);
      Core·Status (*step_right)(Core·TM_NX *tm); // Synonym for step

    } Core·TM_NX·FG;

    Core·TM_NX·FG Core·TM_NX·fg;

  #endif // #ifndef CVT

  #ifndef CVT
    typedef struct Ξ(Core·TM_NX ,CVT);

    typedef struct{

      Core·Status (*copy_datum)( Ξ(Core·TM_NX ,CVT) *tm_read ,Ξ(Core·TM_NX ,CVT) *tm_write );
      Core·Status       (*read)( Ξ(Core·TM_NX ,CVT) *tm ,CVT *remote_pt );
      Core·Status      (*write)( Ξ(Core·TM_NX ,CVT) *tm ,CVT *remote_pt );
      Core·Status    (*head_pt)( Ξ(Core·TM_NX ,CVT) *tm ,CVT *head_pt );

      struct {

        // Initialize tm
        Core·Status (*mount_pe)(
           Core·TM_NX tm 
          ,CVT position[]
          ,extent_t extent
        );
        Core·Status (*mount_pp)(
           Core·TM_NX *tm 
          ,CVT *position_left
          ,CVT *position_right
        );

        // initializes inner
        Core·Status (*largest_aligned)(Core·TM_NX_uint8_t *outer ,Ξ(Core·TM_NX ,CVT) *inner);

        Core·Status (*encloses_pt)(Ξ(Core·TM_NX ,CVT) *tm ,CVT *pt ,bool *result);
        Core·Status (*encloses_pt_strictly)(Ξ(Core·TM_NX ,CVT) *tm ,CVT *pt ,bool *result);

        Core·Status (*encloses_area)(
          Ξ(Core·TM_NX ,CVT) *outer ,Ξ(Core·TM_NX ,CVT) *inner ,bool *flag
        );
        Core·Status (*encloses_area_strictly)(
          Ξ(Core·TM_NX ,CVT) *outer ,Ξ(Core·TM_NX ,CVT) *inner ,bool *flag
        );
        Core·Status (*overlap)(
          Ξ(Core·TM_NX ,CVT) *a ,Ξ(Core·TM_NX ,CVT) *b ,bool *result
        );
      } area;

    } Ξ(Core·TM_NX ,CVT)·FG;

    Ξ(Core·TM_NX ,CVT)·FG Ξ(Core·TM_NX ,CVT)·fg;

  #endif // #ifdef CVT

  //----------------------------------------
  // Map interface
  //----------------------------------------

  #ifndef CVT

    typedef enum{
       Core·Map·Status·mu = 0
      ,Core·Map·Status·no_tape
      ,Core·Map·Status·not_computable
      ,Core·Map·Status·complete
    } Core·Map·Status;

    typedef enum{
       Core·Map·Completion·mu = 0
      ,Core·Map·Completion·null_fn                 = 1 
      ,Core·Map·Completion·no_tape_access          = 1 << 1
      ,Core·Map·Completion·failed                  = 1 << 2
      ,Core·Map·Completion·perfect_fit             = 1 << 3
      ,Core·Map·Completion·read_surplus            = 1 << 4
      ,Core·Map·Completion·read_surplus_write_gap  = 1 << 5
      ,Core·Map·Completion·write_available         = 1 << 6
      ,Core·Map·Completion·write_gap               = 1 << 7
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

    typedef Core·Map·Fn (*Core·Map·Fn)(AU *x ,AU *fx);

  #endif // #ifndef CVT

  #ifdef CVT
    typedef Ξ(Core·Map ,CVT)·Fn (*Core·Map·Fn)(CVT *x ,CVT *fx);

  #endif CVT

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
    // argument guard implementation
    //----------------------------------------

    #ifndef CVT
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
    #endif // *ifndef CVG

    //----------------------------------------
    // Functions implementation
    //----------------------------------------

    #ifndef CVT

      Core·Status Core·on_track(){ return Core·Status·on_track; }
      Core·Status Core·derailed(){ return Core·Status·derailed; }

      Local void *Core·offset(AU *p ,size_t Δ){
        #ifdef Core·Debug
        if(!p){
          fprintf(stderr ,"Core·offset:: given NULL `p'");
          return NULL;
        }
        #endif
        return (void *)( (CVT *)p ) + Δ;
      }

      Local Core·Status Core·is_aligned(AU *p ,extent_t alignment ,bool *flag){
        #ifdef Core·Debug
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,p ,"given NULL p");
          Core·Guard·fg.check(&chk ,1 ,flag ,"flag is NULL, so nowhere to write result");
          Core·Guard·if_return(chk);
        #endif
        *flag = ( (uintptr_t)p & alignment ) == 0;
        return Core·Status·on_track;
      }

      Local Core·Status Core·round_down(AU *p ,extent_t alignment ,AU **result){
        #ifdef Core·Debug
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,p ,"given NULL p to round");
          Core·Guard·fg.check(&chk ,1 ,result ,"result is NULL, so nowhere to write result");
          Core·Guard·if_return(chk);
        #endif
        *result = (AU *)( (uintptr_t)p & ~(uintptr_t)alignment );
        return Core·Status·on_track;
      }

      Local Core·Status Core·round_up(AU *p ,extent_t alignment ,AU **result){
        #ifdef Core·Debug
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,p ,"given NULL p to round");
          Core·Guard·fg.check(&chk ,1 ,result ,"result is NULL, so nowhere to write result");
          Core·Guard·if_return(chk);
        #endif
        *result = (AU *)( ( (uintptr_t)p + alignment ) & ~(uintptr_t)alignment );
        return Core·Status·on_track;
      }

      Local Core·F Core·f = {
        .on_track = Core·on_track
        ,.derailed = Core·derailed
        ,.offset = Core·offset
      };

    #endif // #ifndef CVT

    #ifdef CVT

      Local void Ξ(Core·F ,CVT)·offset(CVT *p ,size_t Δ){
        #ifdef Core·Debug
        if(!p){
          fprintf(stderr ,"Core·offset_##CVT:: given NULL `p'");
          return NULL;
        }
        #endif
        return p + Δ;
      }

      // Struct instance initialization
      Ξ(Core·F ,CVT) Ξ(Core·F ,CVT)·f = {
        .offset = Ξ(Core·F ,CVT)·offset
        ,.is_aligned_on = Ξ(Core·F ,CVT)·is_aligned_on
        ,.floor_within_aligned = Ξ(Core·F ,CVT)·floor_within_aligned
        ,.ceiling_within_aligned = Ξ(Core·F ,CVT)·ceiling_within_aligned
      };

    #endif

  //----------------------------------------
  // TM_NX implementation
  //----------------------------------------

  /*
  TM_NX are initialized with calls to 'mount_pe' or 'mount_pp'.  These both bind the machine to a tape, and mount the tape. Hence, there is no such thing as an initialized which is not bound to a tape. (It is possible to dismount the tm->hd, then remount tm->hd, but the tape remains bound through that process.)

  Because the TM_NX has no destructive operations, Once it is initialized the tape will never get shorter. With TM_MX it is not possible to mount an empty tape, because the minimum value of extent is zero. Therefore the tape can never be empty.

  For an initialized TM_NX, TM_NX.array.hd == NULL means the tape is currently not mounted.

  C has no means of binding an operation to allocation apart from the initializer syntax, but the user might not initialize a tape machine after allocating one. There is no deterministic method available to a TM_NX to detect if a machine is uninitialized. However, if it happens that TM_NX.array.position == NULL, the only explanations are 1) a bug  2) the machine was not initialized - and the initialized data in TM_NX.array.position happened to be zero. 

  It is an error to re-initialize the machine while it is being used, unfortunately this is a contract with the user, because the TM_NX has no way to detect if the machine was initialized in the first place. 

  */

  #ifndef CVT
    struct{
      AU *position;
      extent_t extent;
      AU *hd;
    }Core·TM_NX;
  #endif

  #ifdef CVT
    struct{
      CVT *position;
      extent_t extent;
      CVT *hd;
    }Ξ(Core·TM_NX ,CVT);
  #endif

  //----------------------------------------
  // TM_NX
  //----------------------------------------

  #ifndef CVT
    Core·Status Core·TM_NX·topo(Core·TM_NX *tm ,Core·Tape·Topo *topo){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
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
  #endif

  #ifdef CVT

    //-----------------------------------
    // common error messages

    const char *Ξ(Core·TM_NX ,CVT)·Msg·tm="given NULL tm";
    const char *Ξ(Core·TM_NX ,CVT)·Msg·flag="given NULL flag pointer";
    const char *Ξ(Core·TM_NX ,CVT)·Msg·address=
      "given NULL address pointer ,or address struct holds NULL address";
    const char *Ξ(Core·TM_NX ,CVT)·Msg·address_on_tape=
      "given address is not on the tape";
    const char *Ξ(Core·TM_NX ,CVT)·Msg·extent="given NULL extent pointer";
    const char *Ξ(Core·TM_NX ,CVT)·Msg·position=
      "Null position.This is only possible when the tape machine has not been initialized.";

    //-----------------------------------
    // Area functions within Core·TM_NX_##CVT

    Core·Status Core·TM_NX_##CVT·topo(Core·TM_NX *tm ,Core·Tape·Topo *topo){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
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

    Local Core·Status Core·TM_NX_##CVT·extent(Core·TM_NX *tm ,extent_t *extent){
      Core·Tape·Topo topo;
      Core·Status status = Core·TM_NX_##CVT·topo(tm ,&topo);
      boolean good_topo = 
        (status == Core·Status·on_track) && (topo & Core·Tape·Topo·finite_nz)
        ;

      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,extent ,Core·TM_NX_##CVT·Msg·extent);
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

    Local Core·Status Core·TM_NX_##CVT·mount_pe(
      Core·TM_NX *tm ,void *position ,extent_t extent
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,position ,"given NULL position");
        Core·Guard·if_return(chk);
      #endif
      tm->array.position = position->array.address;
      tm->array.extent = extent;
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_##CVT·mount_pp(
      Core·TM_NX *tm ,void *position_left ,void *position_right
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
        bool good_address = position_left && position_left->address;
        Core·Guard·fg.check(&chk ,1 ,good_address ,Core·TM_NX_##CVT·Msg·address);
        good_address = position_right && position_right->address;
        Core·Guard·fg.check(&chk ,1 ,good_address ,Core·TM_NX_##CVT·Msg·address);
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

      return Core·TM_NX_##CVT·mount_pe(tm ,position_left ,computed_extent);
    }

    Local Core·Status Core·TM_NX_##CVT·largest_aligned_64(
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

      CVT *p0_64 = (CVT *)( (p0 + 0x7) & ~(uintptr_t)0x7 );
      CVT *p1_64 = (CVT *)( (p1 - 0x7) & ~(uintptr_t)0x7 );

      if(p1_64 < p0_64){
        inner_64->array.position = NULL;
        inner_64->array.extent = 0;
        return Core·Status·derailed;
      }

      inner_64->array.position = p0_64;
      inner_64->array.extent = (extent_t)(p1_64 - p0_64);
      return Core·Status·on_track;
    }


    Local Core·Status Core·TM_NX_##CVT·encloses_pt(
      Core·TM_NX *tm ,void *a ,bool *result
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
        bool good_address = a && a->address;
        Core·Guard·fg.check(&chk ,1 ,good_address ,Core·TM_NX_##CVT·Msg·address);
        Core·Guard·fg.check(&chk ,1 ,result ,"given NULL result pointer");
        Core·Guard·if_return(chk);
      #endif

      *result = 
           (a->array.address >= tm->array.position) 
        && (a->array.address <= tm->array.position + tm->array.extent);

      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_##CVT·encloses_pt_strictly(
      Core·TM_NX *tm ,void *a ,bool *result
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
        bool good_address = a && a->address;
        Core·Guard·fg.check(&chk ,1 ,good_address ,Core·TM_NX_##CVT·Msg·address);
        Core·Guard·fg.check(&chk ,1 ,result ,"given NULL result pointer");
        Core·Guard·if_return(chk);
      #endif

      *result = 
           (a->array.address > tm->array.position) 
        && (a->array.address < tm->array.position + tm->array.extent);

      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_##CVT·encloses_tm(
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

    Local Core·Status Core·TM_NX_##CVT·encloses_tm_strictly(
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

    Local Core·Status Core·TM_NX_##CVT·overlap(
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

    Local Core·Status Core·TM_NX_##CVT·mount(Core·TM_NX *tm){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,tm->array.position ,Core·TM_NX_##CVT·Msg·position);
        Core·Guard·if_return(chk);
      #endif

      tm->array.hd = tm->array.position;
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_##CVT·dismount(Core·TM_NX *tm){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
        Core·Guard·if_return(chk);
      #endif

      // Reset head position upon dismount
      tm->array.hd = NULL;
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_##CVT·status(
      Core·TM_NX *tm ,Core·TM·Head·Status *status
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
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

    Local Core·Status Core·TM_NX_##CVT·can_read(Core·TM_NX *tm ,bool *flag){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
        if(tm){
          // All initialized TM_NX_##CVT have an initialized position
          // Maybe this catches that the machine is uninitialized?
          Core·Guard·fg.check(&chk ,1 ,tm->array.position ,Core·TM_NX_##CVT·Msg·position);
        }      
        Core·Guard·fg.check(&chk ,1 ,flag ,Core·TM_NX_##CVT·Msg·flag);
        Core·Guard·if_return(chk);
      #endif
      *flag = tm && tm->array.hd != NULL;
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_##CVT·on_origin(
      Core·TM_NX *tm ,bool *flag
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,tm && tm->array.position ,Core·TM_NX_##CVT·Msg·position);
        Core·Guard·fg.check(&chk ,1 ,flag ,Core·TM_NX_##CVT·Msg·flag);
        Core·Guard·if_return(chk);
      #endif

      *flag = (tm->array.hd == tm->array.position);
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_##CVT·on_rightmost(
      Core·TM_NX *tm ,bool *flag
    ){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,flag ,Core·TM_NX_##CVT·Msg·flag);
        Core·Guard·fg.check(&chk ,1 ,tm && tm->array.position ,Core·TM_NX_##CVT·Msg·position);
        Core·Guard·if_return(chk);
      #endif

      *flag = tm->array.hd == (tm->array.position + tm->array.extent);
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_##CVT·copy_datum(Core·TM_NX *tm_read ,Core·TM_NX *tm_write){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm_read ,Core·TM_NX_##CVT·Msg·tm);
        Core·Guard·fg.check(&chk ,1 ,tm_write ,Core·TM_NX_##CVT·Msg·tm);
        if(tm_read && tm_write){
          Core·Guard·fg.check(
            &chk ,1 ,Core·TM_NX_##CVT·area.encloses_pt(tm_read ,tm_read->array.hd) 
            ,"Source address is outside tape bounds"
          );
          Core·Guard·fg.check(
            &chk ,1 ,Core·TM_NX_##CVT·area.encloses_pt(tm_write ,tm_write->array.hd) 
            ,"Destination address is outside tape bounds"
          );
        }
        Core·Guard·if_return(chk);
      #endif

      *(tm_write->array.hd) = *(tm_read->array.hd);
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_##CVT·read(Core·TM_NX *tm ,void *read_pt){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
        if(tm) Core·Guard·fg.check(&chk ,1 ,tm->array.hd ,Core·TM_NX_##CVT·Msg·address);
        if(tm && tm->array.hd){
          Core·Guard·fg.check(
            &chk ,1 ,Core·TM_NX_##CVT·area.encloses_pt(tm ,tm->array.hd) 
            ,"Given address is outside tape bounds"
          );
        }
        Core·Guard·if_return(chk);
      #endif

      *(CVT *)read_pt = *(tm_array.hd);
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_##CVT·write(Core·TM_NX *tm ,void *write_pt){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
        if(tm) Core·Guard·fg.check(&chk ,1 ,tm->array.hd ,Core·TM_NX_##CVT·Msg·address);
        if(tm && tm->array.hd){
          Core·Guard·fg.check(
            &chk ,1 ,Core·TM_NX_##CVT·area.encloses_pt(tm ,tm->array.hd) 
            ,"Given address is outside tape bounds"
          );
        }
        Core·Guard·if_return(chk);
      #endif

      *(tm->array.hd) = *(CVT *)write_pt;
      return Core·Status·on_track;
    }

    Local Core·Status Core·TM_NX_##CVT·rewind(Core·TM_NX *tm){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
        if(tm){
          Core·Guard·fg.check(&chk ,1 ,tm->array.position ,Core·TM_NX_##CVT·Msg·position);
        }
        Core·Guard·if_return(chk);
      #endif
      tm->array.hd = tm->array.position;
      return Core·Status·on_track;
    }

    Core·Status Core·TM_NX_##CVT·step(Core·TM_NX *tm){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
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

    Core·Status Core·TM_NX_##CVT·step_left(Core·TM_NX *tm){
      #ifdef Core·Debug
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,tm ,Core·TM_NX_##CVT·Msg·tm);
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
    // Initialization for Core·TM_NX_##CVT·fg

    Local Core·TM_NX·FG Core·TM_NX_##CVT·fg = {
       .mount = Core·TM_NX_##CVT·mount
      ,.dismount = Core·TM_NX_##CVT·dismount

      ,.status = Core·TM_NX_##CVT·status
      ,.can_read = Core·TM_NX_##CVT·can_read
      ,.on_origin = Core·TM_NX_##CVT·on_origin
      ,.on_rightmost = Core·TM_NX_##CVT·on_rightmost

      ,.read = Core·TM_NX_##CVT·read
      ,.write = Core·TM_NX_##CVT·write

      ,.rewind = Core·TM_NX_##CVT·rewind
      ,.step = Core·TM_NX_##CVT·step
      ,.step_left = Core·TM_NX_##CVT·step_left
      ,.step_right = Core·TM_NX_##CVT·step_right // Synonym for step

      ,.area = {
         .topo = Core·TM_NX_##CVT·topo
        ,.extent = Core·TM_NX_##CVT·extent

        ,.mount_pe = Core·TM_NX_##CVT·mount_pe
        ,.mount_pp = Core·TM_NX_##CVT·mount_pp
        ,.largest_aligned_64 = Core·TM_NX_##CVT·largest_aligned_64

        ,.encloses_pt = Core·TM_NX_##CVT·encloses_pt
        ,.encloses_pt_strictly = Core·TM_NX_##CVT·encloses_pt_strictly
        ,.encloses_tm = Core·TM_NX_##CVT·encloses_tm
        ,.encloses_tm_strictly = Core·TM_NX_##CVT·encloses_tm_strictly
        ,.overlap = Core·TM_NX_##CVT·overlap
      }
    };

    //----------------------------------------
    // Map implementation
    //----------------------------------------


Core·Map·Completion Core·map(TM_MX *tm_read ,TM_MX *tm_write ,Core·Map·Fn fn){

  Core·Guard·init_collect(chk);
  if(!fn) chk.flag |= Core·Map·Completion·null_fn;
  bool tm_read_can_read=false ,tm_write_can_read=false;
  Core·TM·Head·Status tm_read_status ,tm_write_status;
  if(
    tm_read->can_read(tm_read ,&tm_read_can_read) == Core·Status·derailed
    || tm_write->can_read(tm_write ,&tm_write_can_read) == Core·Status·derailed
    || !tm_read_can_read
    || !tm_write_can_read
  ){
    chk.flag |= Core·Map·Completion·no_tape_access;
  }
  Core·Guard·return(chk);

  do{
    TM_MX·Remote d_remote;
    if( tm_read->read(tm_read ,&d_addr ,&d_remote) != Core·Status·on_track ){
      chk.flag |= Core·Map·Completion·failed;
      break;
    }

    TM_MX·Remote r_remote;
    if(map->fn(&d_remote ,&r_remote) != Core·Status·on_track){
      chk.flag |= Core·Map·Completion·not_computable;
      break;
    }

    Core·TM_NX·Address r_addr;
    if( tm_write->write(tm_write ,&r_addr ,&r_remote) != Core·Status·on_track ){
      chk.flag |= Core·Map·Completion·failed;
      break;
    }

    if(
      tm_read->step(tm_read) != Core·Status·on_track
      || tm_write->step(tm_write) != Core·Status·on_track
    ){
      chk.flag |= Core·Map·Completion·failed;
      break;
    }

    tm_read->status(tm_read ,&tm_read_status);
    tm_write->status(tm_write ,&tm_write_status);

    if(
      tm_read_status & Core·TM·Head·Status·rightmost
      && tm_write_status & Core·TM·Head·Status·rightmost
    ){
      chk.flag |= Core·Map·Completion·perfect_fit;
      break;
    }

    if(tm_read_status & Core·TM·Head·Status·rightmost){
      chk.flag |= Core·Map·Completion·read_surplus;
      break;
    }

    if(tm_write_status & Core·TM·Head·Status·rightmost){
      chk.flag |= Core·Map·Completion·write_available;
      break;
    }

  }while(true);

  Core·Guard·return(chk);
}


    // Map function using trampoline execution model
    Local Core·Map·Status Core·map(Core·Map·Fn fn){
      #ifdef Core·Debug
      if(!fn){
        fprintf(stderr,"Core·map:: given null function");
        return Core·Map·argument_guard;
      }
      if(
         true
         && fn != Core·Map·by_##CVT
         && fn != Core·Map·##CVT_by_##CVT
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
    //----------------------------------------

    Core·Map·Fn Core·Map·Map·ByteByByte·perfect_fit;
    Core·Map·Fn Core·Map·Map·ByteByByte·read_surplus;
    Core·Map·Fn Core·Map·Map·ByteByByte·write_available;

    Local Core·Map·Fn Core·Map·##CVT_by_##CVT(){
      if(Core·TM_NX_##CVT·extent(Core·tf.copy.read) == Core·TM_NX_##CVT·extent(Core·tf.copy.write))
        return Core·Map·ByteByByte·perfect_fit;

      if(Core·TM_NX_##CVT·extent(Core·tf.copy.read) > Core·TM_NX_##CVT·extent(Core·tf.copy.write))
        return Core·Map·ByteByByte·read_surplus;

      return Core·Map·ByteByByte·write_available;
    }

    Local Core·Map·Fn Core·Map·ByteByByte·perfect_fit(){
      CVT **r = &Core·tf.copy.read_pt;
      CVT *r1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.read);
      CVT **w = &Core·tf.copy.write_pt;

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
      CVT **r = &Core·tf.copy.read_pt;
      CVT *r1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.read);
      CVT **w = &Core·tf.copy.write_pt;
      CVT *w1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.write);

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
      CVT **r = &Core·tf.copy.read_pt;
      CVT *r1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.read);
      CVT **w = &Core·tf.copy.write_pt;

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
    Core·Map·Fn Core·Map·by_##CVT;
    Core·Map·Fn Core·Map·ByWord64·leadin;
    Core·Map·Fn Core·Map·ByWord64·bulk;
    Core·Map·Fn Core·Map·ByWord64·tail;

    // Initialize the copy_64 process
    Local Core·Map·Fn Core·Map·by_##CVT(){
      // Determine the largest 64-bit aligned region within the read area
      Core·TM_NX_##CVT·largest_aligned_64(Core·tf.copy.read ,&Core·tl.copy_64.area_64);

      // Choose the correct function based on alignment
      if(Core·TM_NX_##CVT·empty(&Core·tl.copy_64.area_64)) return Core·Map·ByWord64·tail;
      if(Core·is_aligned_on_64(Core·TM_NX_##CVT·position(Core·tf.copy.read))) return Core·Map·ByWord64·bulk;
      return Core·Map·ByWord64·leadin;
    }

    // Lead-in byte copy (until alignment)
    Local Core·Map·Fn Core·Map·ByWord64·leadin(){
      CVT **r = &Core·tf.copy.read_pt;
      CVT *r0_64 = Core·TM_NX_##CVT·position(&Core·tl.copy_64.area_64);
      CVT **w = &Core·tf.copy.write_pt;

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
      uint64_t *r1_64 = Core·TM_NX_##CVT·position_right(&Core·tl.copy_64.area_64);

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
      CVT **r = &Core·tf.copy.read_pt;
      CVT *r1 = Core·TM_NX_##CVT·position_right(&Core·tl.copy_64.area_64);
      CVT **w = &Core·tf.copy.write_pt;

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

    Local uint16_t Core·byte_to_hex(CVT byte){
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
      if(Core·TM_NX_##CVT·extent(Core·tf.copy.read) == (Core·TM_NX_##CVT·extent(Core·tf.copy.write) >> 1)){
        return Core·Map·WriteHex·perfect_fit;
      }
      if(Core·TM_NX_##CVT·extent(Core·tf.copy.read) > (Core·TM_NX_##CVT·extent(Core·tf.copy.write) >> 1)){
        return Core·Map·WriteHex·read_surplus;
      }
      return Core·Map·WriteHex·write_available;
    }

    Local Core·Map·Fn Core·Map·WriteHex·perfect_fit(){
      CVT **r = &Core·tf.copy.read_pt;
      CVT *r1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.read);
      CVT **w = &Core·tf.copy.write_pt;

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
      CVT **r = &Core·tf.copy.read_pt;
      CVT *r1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.write);
      CVT **w = &Core·tf.copy.write_pt;

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
      CVT **r = &Core·tf.copy.read_pt;
      CVT *r1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.read);
      CVT **w = &Core·tf.copy.write_pt;
      CVT *w1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.write);

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

    Local CVT Core·hex_to_byte(uint16_t hex){
      CVT high = hex >> 8;
      CVT low = hex & 0xFF;

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
      if((Core·TM_NX_##CVT·extent(Core·tf.copy.read) >> 1) == Core·TM_NX_##CVT·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·perfect_fit;
      }
      if((Core·TM_NX_##CVT·extent(Core·tf.copy.read) >> 1) > Core·TM_NX_##CVT·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·read_surplus;
      }
      return Core·Map·ReadHex·write_available;
    }

    Local Core·Map·Fn Core·Map·ReadHex·perfect_fit(){
      CVT **r = &Core·tf.copy.read_pt;
      CVT *r1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.read);
      CVT **w = &Core·tf.copy.write_pt;

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
      CVT **r = &Core·tf.copy.read_pt;
      CVT *r1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.write);
      CVT **w = &Core·tf.copy.write_pt;

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
      CVT **r = &Core·tf.copy.read_pt;
      CVT *r1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.read);
      CVT **w = &Core·tf.copy.write_pt;
      CVT *w1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.write);

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
      if((Core·TM_NX_##CVT·extent(Core·tf.copy.read) >> 1) == Core·TM_NX_##CVT·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·perfect_fit;
      }
      if((Core·TM_NX_##CVT·extent(Core·tf.copy.read) >> 1) > Core·TM_NX_##CVT·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·read_surplus;
      }
      return Core·Map·ReadHex·write_available;
    }

    Local Core·Map·Fn Core·Map·ReadHex·perfect_fit(){
      CVT **r = &Core·tf.copy.read_pt;
      CVT *r1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.read);
      CVT **w = &Core·tf.copy.write_pt;

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
      CVT **r = &Core·tf.copy.read_pt;
      CVT *r1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.write);
      CVT **w = &Core·tf.copy.write_pt;

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
      CVT **r = &Core·tf.copy.read_pt;
      CVT *r1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.read);
      CVT **w = &Core·tf.copy.write_pt;
      CVT *w1 = Core·TM_NX_##CVT·position_right(Core·tf.copy.write);

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
          ,.read_fn_8 = Core·TM_NX_##CVT·read_8_fwd
          ,.read_fn_64 = Core·TM_NX_##CVT·read_64_fwd
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
      .Area·init_pe = Core·TM_NX_##CVT·init_pe
      ,.Area·init_pp = Core·TM_NX_##CVT·init_pp
      ,.Area·set_position = Core·TM_NX_##CVT·set_position
      ,.Area·set_position_left = Core·TM_NX_##CVT·set_position
      ,.Area·set_position_right = Core·TM_NX_##CVT·set_position_right
      ,.Area·set_extent = Core·TM_NX_##CVT·set_extent
      ,.Area·position = Core·TM_NX_##CVT·position
      ,.Area·position_left = Core·TM_NX_##CVT·position
      ,.Area·position_right = Core·TM_NX_##CVT·position_right
      ,.Area·extent = Core·TM_NX_##CVT·extent
      ,.Area·length_Kung = Core·TM_NX_##CVT·length_Kung
      ,.Area·empty = Core·TM_NX_##CVT·empty

      ,.Area·encloses_pt = Core·TM_NX_##CVT·encloses_pt
      ,.Area·encloses_pt_strictly = Core·TM_NX_##CVT·encloses_pt_strictly
      ,.Area·encloses_area = Core·TM_NX_##CVT·encloses_area
      ,.Area·encloses_area_strictly = Core·TM_NX_##CVT·encloses_area_strictly
      ,.Area·overlap = Core·TM_NX_##CVT·overlap
      ,.Area·largest_aligned_64 = Core·TM_NX_##CVT·largest_aligned_64
      ,.Area·complement = Core·TM_NX_##CVT·complement

      ,.Area·read_8_zero = Core·TM_NX_##CVT·read_8_zero
      ,.Area·read_8_fwd = Core·TM_NX_##CVT·read_8_fwd
      ,.Area·read_8_rev = Core·TM_NX_##CVT·read_8_rev
      ,.Area·read_64_zero = Core·TM_NX_##CVT·read_64_zero
      ,.Area·read_64_fwd = Core·TM_NX_##CVT·read_64_fwd
      ,.Area·read_64_rev = Core·TM_NX_##CVT·read_64_rev

      ,.is_aligned_on_64 = Core·is_aligned_on_64
      ,.floor_64 = Core·floor_64
      ,.ceiling_64 = Core·ceiling_64
      ,.offset_8 = Core·offset_8
      ,.offset_64 = Core·offset_64

      ,.byte_to_hex = Core·byte_to_hex
      ,.hex_to_byte = Core·hex_to_byte

      ,.copy = Core·map
      ,.Map·##CVT_by_##CVT = Core·Map·##CVT_by_##CVT
      ,.Map·write_hex = Core·Map·write_hex
      ,.Map·read_hex = Core·Map·read_hex
    };

  #endif // LOCAL

#endif // IMPLEMENTATION
