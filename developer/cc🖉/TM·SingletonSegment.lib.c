/*
  Namespace: TM·SingletonSegment

  Template parameters:

    `_TM·CVT_` Cell Value Type`.

*/
#include "TM.lib.c"

/*--------------------------------------------------------------------------------
 Interface 
--------------------------------------------------------------------------------*/

// once per translation unit
#ifndef TM·FACE
#define TM·FACE

  #define TM·SingletonSegment·DEBUG
  #ifdef TM·SingletonSegment·DEBUG
    #include <stdio.h>
  #endif

#endif //#ifndef TM·FACE

// once per _TM·CVT_ value
// Caller must #define SET_Binding__TM·SingletonSegment·<CVT>, after inclusion, to prevent re-inclusion
#ifdef _TM·CVT_
#if BOOLEAN( NOT_IN(Binding ,·(TM·SingletonSegment,_TM·CVT_)) )
#ifdef TM·SingletonSegment·DEBUG
  #pragma message( "Creating TM·SingletonSegment type with a CVT of:" STR_VAL(_TM·CVT_) )
#endif

  typedef struct{
    bool     hd;
    _TM·CVT_ value;
  } ·(TM,_TM·CVT_,SingletonSegment,Tableau);

  ·(TM,_TM·CVT_) ·(TM,_TM·CVT_,SingletonSegment,init)(
     ·(TM,_TM·CVT_,SingletonSegment,Tableau) *t
    ,_TM·CVT_ initial_value
  );

#endif 
#endif 

/*--------------------------------------------------------------------------------
  Local 
--------------------------------------------------------------------------------*/
#ifdef LOCAL

  // once per translation unit
  #ifndef TM·SingletonSegment·LOCAL
  #define TM·SingletonSegment·LOCAL

  #endif

  // once per _TM·CVT_ value
  // Caller must #define SET_TM·LOCAL__TM·<CVT>, after inclusion, to prevent re-inclusion
  #ifdef _TM·CVT_
  #if BOOLEAN(NOT_IN(TM·LOCAL ,·(TM·SingletonSegment,_TM·CVT_)))
  #ifdef TM·SingletonSegment·DEBUG
    #pragma message( "Including TM·SingletonSegment LOCAL code for:" STR_VAL(_TM·CVT_) )
  #endif

    Local TM·Topo ·(TM,_TM·CVT_,SingletonSegment,topo)( ·(TM,_TM·CVT_) tm ){
      return TM·Topo·singleton;
    }
    Local bool ·(TM,_TM·CVT_,SingletonSegment,bounded)( ·(TM,_TM·CVT_) tm ){
      return true;
    }
    Local ·(extent_t,_TM·CVT_) ·(TM,_TM·CVT_,SingletonSegment,extent)( ·(TM,_TM·CVT_) tm ){
      return 0;
    }
    Local TM·Status ·(TM,_TM·CVT_,SingletonSegment,status)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonSegment,Tableau) *t = (·(TM,_TM·CVT_,SingletonSegment,Tableau) *) tm.tableau;
      if( !t->hd ) return TM·Status·dismounted;
      return TM·Status·leftmost | TM·Status·rightmost;
    }
    Local bool ·(TM,_TM·CVT_,SingletonSegment,dismounted)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonSegment,Tableau) *t = (·(TM,_TM·CVT_,SingletonSegment,Tableau) *) tm.tableau;
      return !t->hd;
    }

    // these could be pulled from the tm.pft table, and defined once
    Local bool ·(TM,_TM·CVT_,SingletonSegment,on_tape)( ·(TM,_TM·CVT_) tm ){
      return ·(TM,_TM·CVT_,SingletonSegment,status)( tm ) & TM·Status·on_tape;
    }
    Local bool ·(TM,_TM·CVT_,SingletonSegment,on_leftmost)( ·(TM,_TM·CVT_) tm ){
      return ·(TM,_TM·CVT_,SingletonSegment,status)( tm ) & TM·Status·leftmost;
    }
    Local bool ·(TM,_TM·CVT_,SingletonSegment,on_rightmost)( ·(TM,_TM·CVT_) tm ){
      return ·(TM,_TM·CVT_,SingletonSegment,status)( tm ) & TM·Status·rightmost;
    }

    // does nothing if tape is already mounted
    Local void ·(TM,_TM·CVT_,SingletonSegment,mount)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonSegment,Tableau) *t = (·(TM,_TM·CVT_,SingletonSegment,Tableau) *) tm.tableau;
      t->hd = true;
    }
    Local void ·(TM,_TM·CVT_,SingletonSegment,dismount)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonSegment,Tableau) *t = (·(TM,_TM·CVT_,SingletonSegment,Tableau) *) tm.tableau;
      t->hd = false;
    }

    Local void ·(TM,_TM·CVT_,SingletonSegment,s)( ·(TM,_TM·CVT_) tm ){
      assert(0);
    }
    Local void ·(TM,_TM·CVT_,SingletonSegment,step_left)( ·(TM,_TM·CVT_) tm ){
      assert(0);
    }
    Local void ·(TM,_TM·CVT_,SingletonSegment,rewind)( ·(TM,_TM·CVT_) tm ){
      return;
    }

    Local _TM·CVT_ ·(TM,_TM·CVT_,SingletonSegment,r)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonSegment,Tableau) *t = (·(TM,_TM·CVT_,SingletonSegment,Tableau) *) tm.tableau;
      return t->value;
    }
    Local void ·(TM,_TM·CVT_,SingletonSegment,w)( ·(TM,_TM·CVT_) tm ,_TM·CVT_ *remote_pt ){
      ·(TM,_TM·CVT_,SingletonSegment,Tableau) *t = (·(TM,_TM·CVT_,SingletonSegment,Tableau) *) tm.tableau;
      t->value = *remote_pt;
    }

    Local ·(TM,_TM·CVT_,PFT) ·(TM,_TM·CVT_,SingletonSegment,pft) = {

      .topo           = ·(TM,_TM·CVT_,SingletonSegment,topo)
      ,.bounded       = ·(TM,_TM·CVT_,SingletonSegment,bounded)
      ,.extent        = ·(TM,_TM·CVT_,SingletonSegment,extent)

      ,.status        = ·(TM,_TM·CVT_,SingletonSegment,status)
      ,.dismounted    = ·(TM,_TM·CVT_,SingletonSegment,dismounted)
      ,.on_tape       = ·(TM,_TM·CVT_,SingletonSegment,on_tape)
      ,.on_leftmost   = ·(TM,_TM·CVT_,SingletonSegment,on_leftmost)
      ,.on_rightmost  = ·(TM,_TM·CVT_,SingletonSegment,on_rightmost)

      ,.mount         = ·(TM,_TM·CVT_,SingletonSegment,mount)
      ,.dismount      = ·(TM,_TM·CVT_,SingletonSegment,dismount)

      ,.step          = ·(TM,_TM·CVT_,SingletonSegment,s)
      ,.step_left     = ·(TM,_TM·CVT_,SingletonSegment,step_left)
      ,.rewind        = ·(TM,_TM·CVT_,SingletonSegment,rewind)

      ,.r          = ·(TM,_TM·CVT_,SingletonSegment,r)
      ,.w         = ·(TM,_TM·CVT_,SingletonSegment,w)

    };

    /*
     tm is up casted from being array specific, to being generic. Later it is downcasted within the array code before being used. This can be seen at the top of each of the array specific function. This is the only loss of static type safety, and it is embedded in the library code.
    */
    ·(TM,_TM·CVT_) ·(TM,_TM·CVT_,SingletonSegment,init)(
        ·(TM,_TM·CVT_,SingletonSegment,Tableau) *t
       ,_TM·CVT_ initial_value
    ){
      t->hd    = true;
      t->value = initial_value;

      ·(TM,_TM·CVT_) tm = {
         .tableau = (·(TM,_TM·CVT_,Tableau) *)t
        ,.pft      = &·(TM,_TM·CVT_,SingletonSegment,pft)
      };

      return tm;
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

