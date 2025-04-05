/*
  Namespace: TM
  Template parameters:

    `TM·CVT` Cell Value Type`.

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

// once per TM·CVT value
// Caller must #define SET__Binding__TM·<CVT>, after inclusion, to prevent re-inclusion
#ifdef TM·CVT
#if BOOLEAN( NOT_IN(Binding ,Ξ(TM,TM·CVT)) )
#ifdef TM·DEBUG
  #pragma message( "Creating TM type with a CVT of:" STR_VAL(TM·CVT) )
#endif

  #define Binding Ξ(TM,TM·CVT)
  #include "Binding.lib.c"

  typedef size_t Ξ(extent_t,TM·CVT);

  typedef struct Ξ(TM,TM·CVT,FG){

    TM·Tape·Topo       (*Tape·topo)       ( Ξ(TM,TM·CVT) tm );
    bool               (*Tape·bounded)    ( Ξ(TM,TM·CVT) tm );
    Ξ(extent_t,TM·CVT) (*Tape·extent)     ( Ξ(TM,TM·CVT) tm );

    TM·Head·Status     (*Head·status)     ( Ξ(TM,TM·CVT) tm );
    bool               (*Head·dismounted) ( Ξ(TM,TM·CVT) tm );
    bool               (*Head·on_tape)    ( Ξ(TM,TM·CVT) tm );
    bool               (*Head·on_leftmost)( Ξ(TM,TM·CVT) tm );
    bool               (*Head·on_rightmost)( Ξ(TM,TM·CVT) tm );

    // tape machine functions
    void               (*mount)           ( Ξ(TM,TM·CVT) tm );
    void               (*dismount)        ( Ξ(TM,TM·CVT) tm );

    void               (*step)            ( Ξ(TM,TM·CVT) tm );
    void               (*step_right)      ( Ξ(TM,TM·CVT) tm );
    void               (*step_left)       ( Ξ(TM,TM·CVT) tm );
    void               (*rewind)          ( Ξ(TM,TM·CVT) tm );

    TM·CVT             (*read)            ( Ξ(TM,TM·CVT) tm );
    void               (*write)           ( Ξ(TM,TM·CVT) tm ,TM·CVT *remote_pt );

  } Ξ(TM,TM·CVT,FG);

  //----------------------------------------
  // Array interface

  #undef  TM·ARRAY
  #define TM·ARRAY Ξ(TM,TM·CVT,ARRAY)

  struct Ξ(TM,TM·CVT,ARRAY,Tableau);
  typedef struct Ξ(TM,TM·CVT,ARRAY,Tableau) Ξ(TM,TM·CVT,ARRAY,Tableau);

  /*
    We assume that the binding produced by init is valid. Thus it does not have null pointers to the tableau or the fg table; it points to an initialized tableau; it points to a valid array fg table; and, that the fg table and tableau go together.

    Calling init is the only time the user/programmer will need to know the implementation name.

    The resulting binding object returned is what the user will call an instance of the
    type.
  */
  TM Ξ(TM,TM·CVT,ARRAY,init_pe)(
     Ξ(TM,TM·CVT,ARRAY,Tableau) *t
    ,TM·CVT position[]
    ,Ξ(extent_t,TM·CVT) extent
  );

  TM Ξ(TM,TM·CVT,ARRAY,init_pp)(
     Ξ(TM,TM·CVT,ARRAY,Tableau) *t
    ,TM·CVT *position_left
    ,TM·CVT *position_right
  );

#endif // #if BOOLEAN( NOT_IN(Binding ,TM) )
#endif // #ifdef TM·CVT

/*--------------------------------------------------------------------------------
  Local 
--------------------------------------------------------------------------------*/
#ifdef LOCAL

  // once per translation unit
  #ifndef TM·LOCAL
  #define TM·LOCAL

    const char *TM·Msg·tm     = "given NULL tm";
    const char *TM·Msg·flag   = "given NULL flag pointer";
    const char *TM·Msg·result = "given NULL result pointer";
    const char *TM·Msg·status = "bad head status";

  #endif

  // once per TM·CVT value
  // Caller must #define SET__TM·LOCAL__TM·<CVT>, after inclusion, to prevent re-inclusion
  #ifdef TM·CVT
  #if BOOLEAN(NOT_IN(TM·LOCAL ,Ξ(TM,TM·CVT)))
  #ifdef TM·DEBUG
    #pragma message( "Including LOCAL code for:" STR_VAL(TM·CVT) )
  #endif


    /*------------------------------------------------------------------------
      Array implementation with a segment tape
    */

    typedef struct Ξ(TM,TM·CVT,ARRAY,Tableau){
      TM·CVT               *hd;
      TM·CVT                position[];
      Ξ(extent_t,TM·CVT)    extent;
    } Ξ(TM,TM·CVT,ARRAY,Tableau);

    Local TM·Tape·Topo Ξ(TM,TM·CVT,ARRAY,Tape·topo)( Ξ(TM,TM·CVT) tm ){
      Ξ(TM,TM·CVT,ARRAY,Tableau) *t = (Ξ(TM,TM·CVT,ARRAY,Tableau) *) tm.tableau;
      if( t->extent == 0 ) return TM·Tape·Topo·singleton;
      return TM·Tape·Topo·segment;
    }

    Local bool Ξ(TM,TM·CVT,ARRAY,Tape·bounded)( Ξ(TM,TM·CVT) tm ){
      Ξ(TM,TM·CVT,ARRAY,Tableau) *t = (Ξ(TM,TM·CVT,ARRAY,Tableau) *) tm.tableau;
      return Ξ(TM,TM·CVT,ARRAY,Tape·topo)( tm ) & TM·Tape·Topo·bounded;
    }

    Local Ξ(extent_t,TM·CVT) Ξ(TM,TM·CVT,ARRAY,Tape·extent)( Ξ(TM,TM·CVT) tm ){
      Ξ(TM,TM·CVT,ARRAY,Tableau) *t = (Ξ(TM,TM·CVT,ARRAY,Tableau) *) tm.tableau;
      return t->extent;
    }


    Local TM·Head·Status Ξ(TM,TM·CVT,ARRAY,Head·status)( Ξ(TM,TM·CVT) tm ){
      Ξ(TM,TM·CVT,ARRAY,Tableau) *t = (Ξ(TM,TM·CVT,ARRAY,Tableau) *) tm.tableau;
      if( !t->hd ) return TM·Head·Status·dismounted;
      if( t->hd == tm->position ) return TM·Head·Status·leftmost;

      TM·CVT *rightmost_pt = t->position + t->extent;
      if( t->hd == rightmost_pt ) return TM·Head·Status·rightmost;
      if( t->hd < tm->position || t->hd > rightmost_pt )
        return TM·Head·Status·out_of_area;

      return TM·Head·Status·interim;
    }

    Local bool Ξ(TM,TM·CVT,ARRAY,Head·dismounted)( Ξ(TM,TM·CVT) tm ){
      Ξ(TM,TM·CVT,ARRAY,Tableau) *t = (Ξ(TM,TM·CVT,ARRAY,Tableau) *) tm.tableau;
      return Ξ(TM,TM·CVT,ARRAY,Head·status)( tm ) & TM·Head·Status·dismounted;
    }

    Local bool Ξ(TM,TM·CVT,ARRAY,Head·on_tape)( Ξ(TM,TM·CVT) tm ){
      Ξ(TM,TM·CVT,ARRAY,Tableau) *t = (Ξ(TM,TM·CVT,ARRAY,Tableau) *) tm.tableau;
      return Ξ(TM,TM·CVT,ARRAY,Head·status)( tm ) & TM·Head·Status·on_tape;
    }

    Local bool Ξ(TM,TM·CVT,ARRAY,Head·on_leftmost)( Ξ(TM,TM·CVT) tm ){
      Ξ(TM,TM·CVT,ARRAY,Tableau) *t = (Ξ(TM,TM·CVT,ARRAY,Tableau) *) tm.tableau;
      return Ξ(TM,TM·CVT,ARRAY,Head·status)( tm ) & TM·Head·Status·leftmost;
    }

    Local bool Ξ(TM,TM·CVT,ARRAY,Head·on_rightmost)( Ξ(TM,TM·CVT) tm ){
      Ξ(TM,TM·CVT,ARRAY,Tableau) *t = (Ξ(TM,TM·CVT,ARRAY,Tableau) *) tm.tableau;
      return Ξ(TM,TM·CVT,ARRAY,Head·status)( tm ) & TM·Head·Status·rightmost;
    }

    // does nothing if the hd is already mounted
    Local void Ξ(TM,TM·CVT,ARRAY,mount)( Ξ(TM,TM·CVT) tm ){
      Ξ(TM,TM·CVT,ARRAY,Tableau) *t = (Ξ(TM,TM·CVT,ARRAY,Tableau) *) tm.tableau;
      if( !t->hd ) t->hd = t->position;
    }

    Local void Ξ(TM,TM·CVT,ARRAY,dismount)( Ξ(TM,TM·CVT) tm ){
      Ξ(TM,TM·CVT,ARRAY,Tableau) *t = (Ξ(TM,TM·CVT,ARRAY,Tableau) *) tm.tableau;
      t->hd = NULL;
    }

    // does nothing if the hd is not mounted
    Local void Ξ(TM,TM·CVT,ARRAY,step)( Ξ(TM,TM·CVT) tm ){
      Ξ(TM,TM·CVT,ARRAY,Tableau) *t = (Ξ(TM,TM·CVT,ARRAY,Tableau) *) tm.tableau;
      t->hd++;
    }

    Local void Ξ(TM,TM·CVT,ARRAY,step_left)( Ξ(TM,TM·CVT) tm ){
      Ξ(TM,TM·CVT,ARRAY,Tableau) *t = (Ξ(TM,TM·CVT,ARRAY,Tableau) *) tm.tableau;
      t->hd--;
    }

    Local void Ξ(TM,TM·CVT,ARRAY,rewind)( Ξ(TM,TM·CVT) tm ){
      Ξ(TM,TM·CVT,ARRAY,Tableau) *t = (Ξ(TM,TM·CVT,ARRAY,Tableau) *) tm.tableau;
      if( Ξ(TM,TM·CVT,ARRAY,Head·dismounted)( tm ) ) return;
      t->hd = t->position;
    }

    Local TM·CVT Ξ(TM,TM·CVT,ARRAY,read)( Ξ(TM,TM·CVT) tm ){
      Ξ(TM,TM·CVT,ARRAY,Tableau) *t = (Ξ(TM,TM·CVT,ARRAY,Tableau) *) tm.tableau;
      return *t->hd;
    }

    Local void Ξ(TM,TM·CVT,ARRAY,write)( Ξ(TM,TM·CVT) tm ,TM·CVT *remote_pt ){
      Ξ(TM,TM·CVT,ARRAY,Tableau) *t = (Ξ(TM,TM·CVT,ARRAY,Tableau) *) tm.tableau;
      *remote_pt = *t->hd;
    }

    Local TM·Binding Ξ(TM,TM·CVT,FG) Ξ(TM,TM·CVT,ARRAY,fg) = {

      .Tape·topo         = Ξ(TM,TM·CVT,ARRAY,Tape·topo)
      ,.Tape·bounded     = Ξ(TM,TM·CVT,ARRAY,Tape·bounded)
      ,.Tape·extent      = Ξ(TM,TM·CVT,ARRAY,Tape·extent)

      ,.Head·status      = Ξ(TM,TM·CVT,ARRAY,Head·status)
      ,.Head·dismounted  = Ξ(TM,TM·CVT,ARRAY,Head·dismounted)
      ,.Head·on_tape     = Ξ(TM,TM·CVT,ARRAY,Head·on_tape)
      ,.Head·on_leftmost = Ξ(TM,TM·CVT,ARRAY,Head·on_leftmost)
      ,.Head·on_rightmost= Ξ(TM,TM·CVT,ARRAY,Head·on_rightmost)

      ,.mount            = Ξ(TM,TM·CVT,ARRAY,mount)
      ,.dismount         = Ξ(TM,TM·CVT,ARRAY,dismount)

      ,.step             = Ξ(TM,TM·CVT,ARRAY,step)
      ,.step_right       = Ξ(TM,TM·CVT,ARRAY,step)
      ,.step_left        = Ξ(TM,TM·CVT,ARRAY,step_left)
      ,.rewind           = Ξ(TM,TM·CVT,ARRAY,rewind)

      ,.read             = Ξ(TM,TM·CVT,ARRAY,read)
      ,.write            = Ξ(TM,TM·CVT,ARRAY,write)

    };

    /*
     tm is up casted from being array specific, to being generic. Later it is downcasted within the array code before being used. This can be seen at the top of each of the array specific function. This is the only loss of static type safety, and it is embedded in the library code.
    */
    Ξ(TM,TM·CVT) Ξ(TM,TM·CVT,ARRAY,init_pe)(
       Ξ(TM,TM·CVT,ARRAY,Tableau) *t
      ,TM·CVT *position
      ,Ξ(extent_t,TM·CVT) extent
    ){
      t->hd       = position;
      t->position = position;
      t->extent   = extent;

      Ξ(TM,TM·CVT) tm = (Ξ(TM,TM·CVT)) {
         .tableau = t
        ,.fg      = &Ξ(TM,TM·CVT,ARRAY,fg)
      };

      return tm;
    }

    Ξ(TM,TM·CVT) Ξ(TM,TM·CVT,ARRAY,init_pp)(
       Ξ(TM,TM·CVT,ARRAY,Tableau) *t
      ,TM·CVT *position_left
      ,TM·CVT *position_right
    ){
      t->hd = position_left;
      t->position = position_left;
      t->extent = position_right - position_left;
    }



  #endif // #if BOOLEAN(NOT_IN(TM·LOCAL ,TM))
  #endif // #ifdef TM·CVT

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
#undef TM·CVT
