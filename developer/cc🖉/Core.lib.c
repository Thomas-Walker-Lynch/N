/*
  Core - core memory operations.

  Abbreviations used in comments:

    'ATP' `At This Point' in the code. Acronym used in comments usually before pointing
    out certain values variables must have.

    'AToW' - At Time of Writing ,also used in comments.

  Abbreviations used in identifiers:
 
  `AU` `Addressable Unit for the machine`. The C standard leaves this open to definition by the architecture and calls it `char`. On most all machines today it is uint8_t;

  We use structs to group functions into a namespace. When all the functions that have a specific type of given argument are group together ,we call the table a 'Functions Given Type X table' ,or 'FG table' for short. A specific instance of an FG table is an `fg` table.

*/

//--------------------------------------------------------------------------------
// Interface 
//--------------------------------------------------------------------------------
#ifndef Core·FACE
#define Core·FACE

  #define Core·DEBUG
  #ifdef FG·DEBUG
    #include <stdio.h>
  #endif

  #include <stdint.h>
  #include <stdbool.h>
  #include <stddef.h>
  #include <assert.h>
  #include "cpp_ext.c"

  //----------------------------------------
  // memory interface

    // extent is the maximum index in an address space ,tape or area ,the doted
    // unit is the cell type.
    typedef size_t extent_t·AU;
    #define extent_of·AU(x) (sizeof(x) - 1)
    // Funny ,we seldom check for this ,perhaps that matters on some tiny machine.
    #define extent_of·AU_address_space ~(uintptr)0;

    // addressable unit for the machine
    // C language standard left this undefined. AToW industry uses uint8_t.
    typedef  uint8_t AU ;
    typedef uint16_t AU2;
    typedef uint32_t AU4;
    typedef uint64_t AU8;

    const AU  AU_MAX  = (~(AU)0);
    const AU2 AU2_MAX = (~(AU2)0);
    const AU4 AU4_MAX = (~(AU4)0);
    const AU8 AU8_MAX = (~(AU8)0);

    // ask the compiler what this is
    // when using enums we get this whether we want it or not
    typedef unsigned int WU;
    const WU WU_MAX = (~(WU)0);

  //----------------------------------------
  // flag facility ,argument guard facility

    typedef enum{
       Core·Status·mu = 0
      ,Core·Status·on_track
      ,Core·Status·derailed
    }Core·Status;

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
      const char *name;
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
  
    // no state ,this is merely a namespace

    typedef struct{
      Core·Status (*on_track)();
      Core·Status (*derailed)();
      Core·Status (*is_aligned)(AU *p ,extent_t·AU alignment ,bool *flag);
      Core·Status (*round_down)(AU *p ,extent_t·AU alignment ,AU **result);
      Core·Status (*round_up)(AU *p ,extent_t·AU alignment ,AU **result);
    } Core·F;
    Local Core·F Core·f;

#endif // #if CORE·FACE


//--------------------------------------------------------------------------------
// Local - at bottom of translation unit, to keep some functions private
//--------------------------------------------------------------------------------
#ifdef LOCAL

// once per translation unit
#ifndef Core·LOCAL
#define Core·LOCAL

  //----------------------------------------
  // argument guard implementation

    Local void Core·Guard·init(Core·Guard *chk ,const char *name ,Core·Flag·Fn af){
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
      ,char *message
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

    Core·Status Core·on_track(){ return Core·Status·on_track; }
    Core·Status Core·derailed(){ return Core·Status·derailed; }

    Local Core·Status Core·is_aligned(AU *p ,extent_t·AU alignment ,bool *flag){
      #ifdef Core·DEBUG
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,p ,"given NULL p");
        Core·Guard·fg.check(&chk ,1 ,flag ,"flag is NULL, so nowhere to write result");
        Core·Guard·if_return(chk);
      #endif
      *flag = ( (uintptr_t)p & alignment ) == 0;
      return Core·Status·on_track;
    }

    Local Core·Status Core·round_down(AU *p ,extent_t·AU alignment ,AU **result){
      #ifdef Core·DEBUG
        Core·Guard·init_count(chk);
        Core·Guard·fg.check(&chk ,1 ,p ,"given NULL p to round");
        Core·Guard·fg.check(&chk ,1 ,result ,"result is NULL, so nowhere to write result");
        Core·Guard·if_return(chk);
      #endif
      *result = (AU *)( (uintptr_t)p & ~(uintptr_t)alignment );
      return Core·Status·on_track;
    }

    Local Core·Status Core·round_up(AU *p ,extent_t·AU alignment ,AU **result){
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

#endif // Core·LOCAL
#endif // LOCAL

//--------------------------------------------------------------------------------
// Library - compiled into a lib.a file by the current make
//   Core currently has no library components
//--------------------------------------------------------------------------------
#ifdef LIBRARY
#endif 

//--------------------------------------------------------------------------------
// undef the template parameters
//   Core currently has not template parameters
//--------------------------------------------------------------------------------
