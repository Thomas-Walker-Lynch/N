/*
  Namespace: TM
  Template parameters:

    `_TM·CVT_` Cell Value Type`.

  TM - Tape Machine Model

  Caller must declare TM set members: 
     #define SET__Binding__TM·<CVT> // replacing <CVT> with the actual value of CVT

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
     TM·Tape·Topo·mu = 0 
    ,TM·Tape·Topo·empty       = 1
    ,TM·Tape·Topo·singleton   = 1 << 1
    ,TM·Tape·Topo·segment     = 1 << 2
    ,TM·Tape·Topo·circle      = 1 << 3
    ,TM·Tape·Topo·tail_cyclic = 1 << 4
    ,TM·Tape·Topo·infinite    = 1 << 5
  }TM·Tape·Topo;
  const TM·Tape·Topo TM·Tape·Topo·bounded = 
    TM·Tape·Topo·singleton 
    | TM·Tape·Topo·segment
    ;

  typedef enum{
    TM·Head·Status·mu = 0
    ,TM·Head·Status·dismounted  = 1
    ,TM·Head·Status·out_of_area = 1 << 1
    ,TM·Head·Status·leftmost    = 1 << 2
    ,TM·Head·Status·interim     = 1 << 3
    ,TM·Head·Status·rightmost   = 1 << 4
  } TM·Head·Status;

  const TM·Head·Status TM·Head·Status·on_tape = 
    TM·Head·Status·leftmost
    | TM·Head·Status·interim
    | TM·Head·Status·rightmost
    ;

#endif //#ifndef TM·FACE

// once per _TM·CVT_ value
// Caller must #define SET__Binding__TM·<CVT>, after inclusion, to prevent re-inclusion
#ifdef _TM·CVT_
#if BOOLEAN( NOT_IN(Binding ,·(TM,_TM·CVT_)) )
#ifdef TM·DEBUG
  #pragma message( "Creating TM type with a CVT of:" STR_VAL(_TM·CVT_) )
#endif

  #define _BINDING_ ·(TM,_TM·CVT_)
  #include "Binding.lib.c"

  // debug: TM·AU tm_binding;

  typedef size_t ·(extent_t,_TM·CVT_);

  typedef struct ·(TM,_TM·CVT_,FG){

    TM·Tape·Topo         (*Tape·topo)     ( ·(TM,_TM·CVT_) tm );
    bool                 (*Tape·bounded)  ( ·(TM,_TM·CVT_) tm );
    ·(extent_t,_TM·CVT_) (*Tape·extent)   ( ·(TM,_TM·CVT_) tm );

    TM·Head·Status     (*Head·status)     ( ·(TM,_TM·CVT_) tm );
    bool               (*Head·dismounted) ( ·(TM,_TM·CVT_) tm );
    bool               (*Head·on_tape)    ( ·(TM,_TM·CVT_) tm );
    bool               (*Head·on_leftmost)( ·(TM,_TM·CVT_) tm );
    bool               (*Head·on_rightmost)( ·(TM,_TM·CVT_) tm );

    // tape machine functions
    void               (*mount)           ( ·(TM,_TM·CVT_) tm );
    void               (*dismount)        ( ·(TM,_TM·CVT_) tm );

    void               (*step)            ( ·(TM,_TM·CVT_) tm );
    void               (*step_right)      ( ·(TM,_TM·CVT_) tm );
    void               (*step_left)       ( ·(TM,_TM·CVT_) tm );
    void               (*rewind)          ( ·(TM,_TM·CVT_) tm );

    _TM·CVT_           (*read)            ( ·(TM,_TM·CVT_) tm );
    void               (*write)           ( ·(TM,_TM·CVT_) tm ,_TM·CVT_ *remote_pt );

  } ·(TM,_TM·CVT_,FG);

  //----------------------------------------
  // Array interface

  // Not exposing the implementation would be better, but the user needs to allocate these before calling init.
  typedef struct{
    _TM·CVT_  *hd;
    _TM·CVT_  *position;
    ·(extent_t,_TM·CVT_) extent;
  } ·(TM,_TM·CVT_,Array,Tableau);

  /*
    We assume that the binding produced by init is valid. Thus: it does not have null pointers to the tableau or the fg table; it points to an initialized tableau; it points to a valid array fg table; and, that the fg table and tableau go together.

    Calling init is the only time the user/programmer will need to know the implementation name.

    The resulting binding object returned is what the user will call an instance of the
    type.
  */
  ·(TM,_TM·CVT_) ·(TM,_TM·CVT_,Array,init_pe)(
     ·(TM,_TM·CVT_,Array,Tableau) *t
    ,_TM·CVT_ position[]
    ,·(extent_t,_TM·CVT_) extent
  );

  ·(TM,_TM·CVT_) ·(TM,_TM·CVT_,Array,init_pp)(
     ·(TM,_TM·CVT_,Array,Tableau) *t
    ,_TM·CVT_ *position_left
    ,_TM·CVT_ *position_right
  );

#endif // #if BOOLEAN( NOT_IN(Binding ,TM) )
#endif // #ifdef _TM·CVT_

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
  // Caller must #define SET__TM·LOCAL__TM·<CVT>, after inclusion, to prevent re-inclusion
  #ifdef _TM·CVT_
  #if BOOLEAN(NOT_IN(TM·LOCAL ,·(TM,_TM·CVT_)))
  #ifdef TM·DEBUG
    #pragma message( "Including LOCAL code for:" STR_VAL(_TM·CVT_) )
  #endif


    /*------------------------------------------------------------------------
      Array implementation with a segment tape
    */

    Local TM·Tape·Topo ·(TM,_TM·CVT_,Array,Tape·topo)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      if( t->extent == 0 ) return TM·Tape·Topo·singleton;
      return TM·Tape·Topo·segment;
    }

    Local bool ·(TM,_TM·CVT_,Array,Tape·bounded)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      return ·(TM,_TM·CVT_,Array,Tape·topo)( tm ) & TM·Tape·Topo·bounded;
    }

    Local ·(extent_t,_TM·CVT_) ·(TM,_TM·CVT_,Array,Tape·extent)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      return t->extent;
    }


    Local TM·Head·Status ·(TM,_TM·CVT_,Array,Head·status)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      if( !t->hd ) return TM·Head·Status·dismounted;
      if( t->hd == t->position ) return TM·Head·Status·leftmost;

      _TM·CVT_ *rightmost_pt = t->position + t->extent;
      if( t->hd == rightmost_pt ) return TM·Head·Status·rightmost;
      if( t->hd < t->position || t->hd > rightmost_pt )
        return TM·Head·Status·out_of_area;

      return TM·Head·Status·interim;
    }

    Local bool ·(TM,_TM·CVT_,Array,Head·dismounted)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      return ·(TM,_TM·CVT_,Array,Head·status)( tm ) & TM·Head·Status·dismounted;
    }

    Local bool ·(TM,_TM·CVT_,Array,Head·on_tape)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      return ·(TM,_TM·CVT_,Array,Head·status)( tm ) & TM·Head·Status·on_tape;
    }

    Local bool ·(TM,_TM·CVT_,Array,Head·on_leftmost)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      return ·(TM,_TM·CVT_,Array,Head·status)( tm ) & TM·Head·Status·leftmost;
    }

    Local bool ·(TM,_TM·CVT_,Array,Head·on_rightmost)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      return ·(TM,_TM·CVT_,Array,Head·status)( tm ) & TM·Head·Status·rightmost;
    }

    // does nothing if the hd is already mounted
    Local void ·(TM,_TM·CVT_,Array,mount)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      if( !t->hd ) t->hd = t->position;
    }

    Local void ·(TM,_TM·CVT_,Array,dismount)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      t->hd = NULL;
    }

    // does nothing if the hd is not mounted
    Local void ·(TM,_TM·CVT_,Array,step)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      t->hd++;
    }

    Local void ·(TM,_TM·CVT_,Array,step_left)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      t->hd--;
    }

    Local void ·(TM,_TM·CVT_,Array,rewind)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      if( ·(TM,_TM·CVT_,Array,Head·dismounted)( tm ) ) return;
      t->hd = t->position;
    }

    Local _TM·CVT_ ·(TM,_TM·CVT_,Array,read)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      return *t->hd;
    }

    Local void ·(TM,_TM·CVT_,Array,write)( ·(TM,_TM·CVT_) tm ,_TM·CVT_ *remote_pt ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      *remote_pt = *t->hd;
    }

    Local ·(TM,_TM·CVT_,FG) ·(TM,_TM·CVT_,Array,fg) = {

      .Tape·topo         = ·(TM,_TM·CVT_,Array,Tape·topo)
      ,.Tape·bounded     = ·(TM,_TM·CVT_,Array,Tape·bounded)
      ,.Tape·extent      = ·(TM,_TM·CVT_,Array,Tape·extent)

      ,.Head·status      = ·(TM,_TM·CVT_,Array,Head·status)
      ,.Head·dismounted  = ·(TM,_TM·CVT_,Array,Head·dismounted)
      ,.Head·on_tape     = ·(TM,_TM·CVT_,Array,Head·on_tape)
      ,.Head·on_leftmost = ·(TM,_TM·CVT_,Array,Head·on_leftmost)
      ,.Head·on_rightmost= ·(TM,_TM·CVT_,Array,Head·on_rightmost)

      ,.mount            = ·(TM,_TM·CVT_,Array,mount)
      ,.dismount         = ·(TM,_TM·CVT_,Array,dismount)

      ,.step             = ·(TM,_TM·CVT_,Array,step)
      ,.step_right       = ·(TM,_TM·CVT_,Array,step)
      ,.step_left        = ·(TM,_TM·CVT_,Array,step_left)
      ,.rewind           = ·(TM,_TM·CVT_,Array,rewind)

      ,.read             = ·(TM,_TM·CVT_,Array,read)
      ,.write            = ·(TM,_TM·CVT_,Array,write)

    };

    /*
     tm is up casted from being array specific, to being generic. Later it is downcasted within the array code before being used. This can be seen at the top of each of the array specific function. This is the only loss of static type safety, and it is embedded in the library code.
    */
    ·(TM,_TM·CVT_) ·(TM,_TM·CVT_,Array,init_pe)(
       ·(TM,_TM·CVT_,Array,Tableau) *t
      ,_TM·CVT_ *position
      ,·(extent_t,_TM·CVT_) extent
    ){
      t->hd       = position;
      t->position = position;
      t->extent   = extent;

      ·(TM,_TM·CVT_) tm = {
         .tableau = (·(TM,_TM·CVT_,Tableau) *)t
        ,.fg      = &·(TM,_TM·CVT_,Array,fg)
      };

      return tm;
    }

    ·(TM,_TM·CVT_) ·(TM,_TM·CVT_,Array,init_pp)(
       ·(TM,_TM·CVT_,Array,Tableau) *t
      ,_TM·CVT_ *position_left
      ,_TM·CVT_ *position_right
    ){
      return ·(TM,_TM·CVT_,Array,init_pe)(
        t ,position_left ,position_right - position_left
      );
    }


  #endif // #if BOOLEAN(NOT_IN(TM·LOCAL ,TM))
  #endif // #ifdef _TM·CVT_

#endif // LOCAL

/*--------------------------------------------------------------------------------
 Library - compiled into a lib.a file by the current make
   Core currently has no library components
--------------------------------------------------------------------------------*/
#ifdef LIBRARY
#endif 

//--------------------------------------------------------------------------------
// undef the template parameters
//--------------------------------------------------------------------------------
#undef _TM·CVT_
