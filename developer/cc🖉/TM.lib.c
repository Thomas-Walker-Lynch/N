/*
  Namespace: TM
  Template parameters:

    `_TM·CVT_` Cell Value Type`.

  TM - Tape Machine Model

  Caller must declare TM set members: 
     #define SET_Binding__TM·<CVT> // replacing <CVT> with the actual value of CVT

*/

/*--------------------------------------------------------------------------------
 Interface 
--------------------------------------------------------------------------------*/

// once per translation unit
#ifndef TM·FACE
#define TM·FACE

  #include <stdint.h>
  #include <stddef.h>
  #include "cpp_ext.c"

  #include "Core.lib.c"
  #include "Binding.lib.c"

  #define TM·DEBUG
  #ifdef TM·DEBUG
    #include <stdio.h>
  #endif

  typedef enum{
     TM·Topo·mu = 0 
    ,TM·Topo·zero_length      = 1
    ,TM·Topo·singleton   = 1 << 1
    ,TM·Topo·segment     = 1 << 2
    ,TM·Topo·circle      = 1 << 3
    ,TM·Topo·tail_cyclic = 1 << 4
    ,TM·Topo·infinite    = 1 << 5
  }TM·Topo;
  const TM·Topo TM·Topo·bounded = 
    TM·Topo·singleton 
    | TM·Topo·segment
    ;

  typedef enum{
    TM·Status·mu = 0
    ,TM·Status·dismounted  = 1
    ,TM·Status·out_of_area = 1 << 1
    ,TM·Status·leftmost    = 1 << 2
    ,TM·Status·interim     = 1 << 3
    ,TM·Status·rightmost   = 1 << 4
  } TM·Status;

  const TM·Status TM·Status·on_tape = 
    TM·Status·leftmost
    | TM·Status·interim
    | TM·Status·rightmost
    ;

#endif //#ifndef TM·FACE

// once per _TM·CVT_ value
// Caller must #define SET_Binding__TM·<CVT>, after inclusion, to prevent re-inclusion
#ifdef _TM·CVT_
#if BOOLEAN( NOT_IN(TM ,_TM·CVT_) )
#ifdef TM·DEBUG
  #pragma message( "Creating TM type with a CVT of:" STR_VAL(_TM·CVT_) )
#endif

  typedef size_t ·(extent_t,_TM·CVT_);

  ParameterConnector·DECLARE( ·(TM,_TM·CVT_) )

  typedef struct ·(TM,_TM·CVT_,PFT){

    TM·Topo              (*topo)     ( ·(TM,_TM·CVT_) tm );
    bool                 (*bounded)  ( ·(TM,_TM·CVT_) tm );
    ·(extent_t,_TM·CVT_) (*extent)   ( ·(TM,_TM·CVT_) tm );

    TM·Status     (*status)     ( ·(TM,_TM·CVT_) tm );
    bool               (*dismounted) ( ·(TM,_TM·CVT_) tm );
    bool               (*on_tape)    ( ·(TM,_TM·CVT_) tm );
    bool               (*on_leftmost)( ·(TM,_TM·CVT_) tm );
    bool               (*on_rightmost)( ·(TM,_TM·CVT_) tm );

    // tape machine functions
    void               (*mount)           ( ·(TM,_TM·CVT_) tm );
    void               (*dismount)        ( ·(TM,_TM·CVT_) tm );

    void               (*step)            ( ·(TM,_TM·CVT_) tm );
    void               (*step_left)       ( ·(TM,_TM·CVT_) tm );
    void               (*rewind)          ( ·(TM,_TM·CVT_) tm );

    _TM·CVT_           (*read)            ( ·(TM,_TM·CVT_) tm );
    void               (*write)           ( ·(TM,_TM·CVT_) tm ,_TM·CVT_ *remote_pt );

  } ·(TM,_TM·CVT_,PFT);


#endif
#endif

/*--------------------------------------------------------------------------------
  Local 
--------------------------------------------------------------------------------*/
#ifdef LOCAL

  // once per translation unit
  #ifndef TM·LOCAL
  #define TM·LOCAL

    #include "Core.lib.c"
    #include "Binding.lib.c"

    const char *TM·Msg·tm     = "given NULL tm";
    const char *TM·Msg·flag   = "given NULL flag pointer";
    const char *TM·Msg·result = "given NULL result pointer";
    const char *TM·Msg·status = "bad head status";

  #endif

  // once per _TM·CVT_ value
  // Caller must #define SET_TM·LOCAL__TM·<CVT>, after inclusion, to prevent re-inclusion
  #ifdef _TM·CVT_
  #if BOOLEAN(NOT_IN(TM·LOCAL ,·(TM,_TM·CVT_)))
  #ifdef TM·DEBUG
    #pragma message( "Including TM LOCAL code for:" STR_VAL(_TM·CVT_) )
  #endif

  #endif // #if BOOLEAN(NOT_IN(TM·LOCAL ,TM))
  #endif // #ifdef _TM·CVT_

#endif // LOCAL

/*--------------------------------------------------------------------------------
 Library - compiled into a lib.a file by the current make
   Core currently has no library components
--------------------------------------------------------------------------------*/
#ifdef LIBRARY
#endif 
