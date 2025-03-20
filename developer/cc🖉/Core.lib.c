 /*
  Core - core memory operations.

  Abbreviations used in comments:

    'ATP' `At This Point' in the code. Acronym used in comments usually before pointing
    out certain values variables must have.

    'AToW' - At Time of Writing, also used in comments.

  Abbreviations used in identifiers:
 
  `AU` `Addressable Unit for the machine`. The C standard leaves this open to definition by the architecture and calls it `char`. On most all machines today it is uint8_t;

  We use structs to group functions into a namespace. When all the functions that have a specific type of given argument are group together, we call the table a 'Functions Given Type X table', or 'FG table' for short. A specific instance of an FG table is an `fg` table.

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

  // the template filling macro
  #include <xi.c>

  //----------------------------------------
  // memory interface
  //----------------------------------------

    // Expand namespace with a macro parameter given value.
    // The namespace and the extended value will be evaluated for macros.
    // Greek capital letter 'Xi' for eXtend.
    #define _Ξ(a ,b) a##·##b
    #define Ξ(a ,b) _Ξ(a ,b)

    // ask the machine what this is
    // C language standard left this undefined, probably why unicode uses 'octet'
    // AToW industry uses uint8_t
    // reminds me of FORTRAN star types
    #define AU uint8_t
    #define AU2 uint16_t
    #define AU4 uint32_t
    #define AU8 uint64_t

    #define AU_MAX (~(AU)0)
    #define AU2_MAX (~(AU2)0)
    #define AU4_MAX (~(AU4)0)
    #define AU8_MAX (~(AU8)0)

    // ask the compiler what this is
    // when using enums we get this whether we want it or not
    #define WU unsigned int
    #define WU_MAX (~(WU)0)

    // extent is an address or an index.  It is not a length.
    // The index scaling CVT type is appended to the end of the extent related identifiers..
    // E.g. extent_of·AU(uint64_t) == 7 while extent_of·uint16_t(uint64_t) == 3;
    // extent_of·AU of the address space is ~(uintptr)0;
    #define extent_of·AU(x)(sizeof(x) - 1)
    #define extent_t·AU size_t

    // Funny, we seldom check for this, but maybe someone is running a microcontroller or something, so we will here. Also, too bad that address 0 can't be used.
    #define extent_address_space·AU ~(uintptr)0;

    typedef enum{
       Core·Status·mu = 0
      ,Core·Status·on_track
      ,Core·Status·derailed
    }Core·Status;

  //----------------------------------------
  // flag facility, argument guard facility
  //----------------------------------------

    typedef void (*Core·Flag·Fn)(WU *flag ,WU err);

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
      Core·Flag·Fn flag_function;
      WU flag;
    } Core·Guard;

    typedef struct {
      void (*init)(Core·Guard *chk ,const char *name ,Core·Flag·Fn af);
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
    #define Core·Guard·assert(chk) assert(!chk.flag);

  //----------------------------------------
  // functions interface
  //----------------------------------------
  
    // no state, this is merely a namespace

    typedef struct{
      Core·Status (*on_track)();
      Core·Status (*derailed)();
      Core·Status (*is_aligned)(AU *p ,extent·AU alignment ,bool *flag);
      Core·Status (*round_down)(AU *p ,extent·AU alignment ,AU **result);
      Core·Status (*round_up)(AU *p ,extent·AU alignment ,AU **result);
    } Core·F;
    Local Core·F Core·f;


//--------------------------------------------------------------------------------
// Implementation

#ifdef Core·IMPLEMENTATION
  // declarations available to all of the IMPLEMENTATION go here
  //
    #ifdef Core·DEBUG
      #include <stdio.h>
    #endif

  //--------------------------------------------------------------------------------
  // implementation to go into the lib.a file
  //
    #ifndef LOCAL
    #endif 

  //--------------------------------------------------------------------------------
  #ifdef LOCAL

    //----------------------------------------
    // argument guard implementation
    //----------------------------------------

      Local void Core·Guard·init(Core·Guard *chk, const char *name, Core·Flag·Fn af){
        if(!chk) return;
        chk->name = name;
        chk->flag_function = af;
        chk->flag = 0;
      }

      Local void Core·Guard·reset(Core·Guard *chk){
        if( !chk ) return;
        chk->flag = 0;
      }

      Local void Core·Guard·check(
         Core·Guard *chk
        ,WU err
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
    // Functions implementation
    //----------------------------------------

      Core·Status Core·on_track(){ return Core·Status·on_track; }
      Core·Status Core·derailed(){ return Core·Status·derailed; }

      Local Core·Status Core·is_aligned(AU *p ,extent·AU alignment ,bool *flag){
        #ifdef Core·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,p ,"given NULL p");
          Core·Guard·fg.check(&chk ,1 ,flag ,"flag is NULL, so nowhere to write result");
          Core·Guard·if_return(chk);
        #endif
        *flag = ( (uintptr_t)p & alignment ) == 0;
        return Core·Status·on_track;
      }

      Local Core·Status Core·round_down(AU *p ,extent·AU alignment ,AU **result){
        #ifdef Core·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,p ,"given NULL p to round");
          Core·Guard·fg.check(&chk ,1 ,result ,"result is NULL, so nowhere to write result");
          Core·Guard·if_return(chk);
        #endif
        *result = (AU *)( (uintptr_t)p & ~(uintptr_t)alignment );
        return Core·Status·on_track;
      }

      Local Core·Status Core·round_up(AU *p ,extent·AU alignment ,AU **result){
        #ifdef Core·DEBUG
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
          ,.is_aligned = Core·is_aligned
          ,.round_down = Core·round_down  // Add `Core`
          ,.round_up = Core·round_up      // Add `Core`
      };

  #endif // LOCAL

#endif // IMPLEMENTATION
