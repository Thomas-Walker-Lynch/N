/*
  Namespace: TM·ZeroLength

  Template parameters:

    `_TM·CVT_` Cell Value Type`.

*/
#include "TM.lib.c"

/*--------------------------------------------------------------------------------
 Interface 
--------------------------------------------------------------------------------*/

// once per translation unit
#ifndef TM·ZeroLength·FACE
#define TM·ZeroLength·FACE

  #define TM·ZeroLength·DEBUG
  #ifdef TM·ZeroLength·DEBUG
    #include <stdio.h>
  #endif

#endif //#ifndef TM·FACE

// once per _TM·CVT_ value
// Caller must #define SET_Binding__TM·ZeroLength·<CVT>, after inclusion, to prevent re-inclusion
#ifdef _TM·CVT_
#if BOOLEAN( NOT_IN(Binding ,·(TM·ZeroLength,_TM·CVT_)) )
#ifdef TM·ZeroLength·DEBUG
  #pragma message( "Creating TM·ZeroLength type with a CVT of:" STR_VAL(_TM·CVT_) )
#endif

  typedef struct{
    bool hd;
  } ·(TM,_TM·CVT_,ZeroLength,StateParameter);

  ·(TM,_TM·CVT_) ·(TM,_TM·CVT_,ZeroLength,init)(
     ·(TM,_TM·CVT_,ZeroLength,StateParameter) *t
  );

#endif
#endif

/*--------------------------------------------------------------------------------
  Local 
--------------------------------------------------------------------------------*/
#ifdef LOCAL

  // once per translation unit
  #ifndef TM·ZeroLength·LOCAL
  #define TM·ZeroLength·LOCAL

  #endif

  // once per _TM·CVT_ value
  // Caller must #define SET_TM·ZeroLength·LOCAL__TM·<CVT>, after inclusion, to prevent re-inclusion
  #ifdef _TM·CVT_
  #if BOOLEAN(NOT_IN(TM·LOCAL ,·(TM·ZeroLength,_TM·CVT_)))
  #ifdef TM·ZeroLength·DEBUG
    #pragma message( "Including TM·ZeroLength LOCAL code for:" STR_VAL(_TM·CVT_) )
  #endif

      Local TM·Topo ·(TM,_TM·CVT_,ZeroLength,topo)( ·(TM,_TM·CVT_) tm ){
        return TM·Topo·zero_length;
      }

      Local bool ·(TM,_TM·CVT_,ZeroLength,bounded)( ·(TM,_TM·CVT_) tm ){
        return true;
      }

      Local ·(extent_t,_TM·CVT_) ·(TM,_TM·CVT_,ZeroLength,extent)( ·(TM,_TM·CVT_) tm ){
        assert(0);
      }

      Local TM·Status ·(TM,_TM·CVT_,ZeroLength,status)( ·(TM,_TM·CVT_) tm ){
        ·(TM,_TM·CVT_,ZeroLength,StateParameter) *t = (·(TM,_TM·CVT_,ZeroLength,StateParameter) *) tm.state_parameter;
        if( !t->hd ) return TM·Status·dismounted;
        return TM·Status·out_of_area;
      }

      Local bool ·(TM,_TM·CVT_,ZeroLength,dismounted)( ·(TM,_TM·CVT_) tm ){
        ·(TM,_TM·CVT_,ZeroLength,StateParameter) *t = (·(TM,_TM·CVT_,ZeroLength,StateParameter) *) tm.state_parameter;
        return !t->hd;
      }

      Local bool ·(TM,_TM·CVT_,ZeroLength,on_tape)( ·(TM,_TM·CVT_) tm ){
        return false;
      }

      Local bool ·(TM,_TM·CVT_,ZeroLength,on_leftmost)( ·(TM,_TM·CVT_) tm ){
        return false;
      }

      Local bool ·(TM,_TM·CVT_,ZeroLength,on_rightmost)( ·(TM,_TM·CVT_) tm ){
        return false;
      }

      // does nothing if tape is already mounted
      Local void ·(TM,_TM·CVT_,ZeroLength,mount)( ·(TM,_TM·CVT_) tm ){
        ·(TM,_TM·CVT_,ZeroLength,StateParameter) *t = (·(TM,_TM·CVT_,ZeroLength,StateParameter) *) tm.state_parameter;
        if( !t->hd ) t->hd = true;
      }

      Local void ·(TM,_TM·CVT_,ZeroLength,dismount)( ·(TM,_TM·CVT_) tm ){
        ·(TM,_TM·CVT_,ZeroLength,StateParameter) *t = (·(TM,_TM·CVT_,ZeroLength,StateParameter) *) tm.state_parameter;
        t->hd = false;
      }

      Local void ·(TM,_TM·CVT_,ZeroLength,s)( ·(TM,_TM·CVT_) tm ){
        assert(0);
      }

      Local void ·(TM,_TM·CVT_,ZeroLength,step_left)( ·(TM,_TM·CVT_) tm ){
        assert(0);
      }

      // rewind does nothing if the tape is dismounted
      Local void ·(TM,_TM·CVT_,ZeroLength,rewind)( ·(TM,_TM·CVT_) tm ){
        ·(TM,_TM·CVT_,ZeroLength,StateParameter) *t = (·(TM,_TM·CVT_,ZeroLength,StateParameter) *) tm.state_parameter;
        if( ·(TM,_TM·CVT_,ZeroLength,dismounted)( tm ) ) return;
        assert(0);
      }

      Local _TM·CVT_ ·(TM,_TM·CVT_,ZeroLength,r)( ·(TM,_TM·CVT_) tm ){
        assert(0);
      }

      Local void ·(TM,_TM·CVT_,ZeroLength,w)( ·(TM,_TM·CVT_) tm ,_TM·CVT_ *remote_pt ){
        assert(0);
      }

      Local ·(TM,_TM·CVT_,PFT) ·(TM,_TM·CVT_,ZeroLength,pft) = {

        .topo           = ·(TM,_TM·CVT_,ZeroLength,topo)
        ,.bounded       = ·(TM,_TM·CVT_,ZeroLength,bounded)
        ,.extent        = ·(TM,_TM·CVT_,ZeroLength,extent)

        ,.status        = ·(TM,_TM·CVT_,ZeroLength,status)
        ,.dismounted    = ·(TM,_TM·CVT_,ZeroLength,dismounted)
        ,.on_tape       = ·(TM,_TM·CVT_,ZeroLength,on_tape)
        ,.on_leftmost   = ·(TM,_TM·CVT_,ZeroLength,on_leftmost)
        ,.on_rightmost  = ·(TM,_TM·CVT_,ZeroLength,on_rightmost)

        ,.mount         = ·(TM,_TM·CVT_,ZeroLength,mount)
        ,.dismount      = ·(TM,_TM·CVT_,ZeroLength,dismount)

        ,.step          = ·(TM,_TM·CVT_,ZeroLength,s)
        ,.step_left     = ·(TM,_TM·CVT_,ZeroLength,step_left)
        ,.rewind        = ·(TM,_TM·CVT_,ZeroLength,rewind)

        ,.r          = ·(TM,_TM·CVT_,ZeroLength,r)
        ,.w         = ·(TM,_TM·CVT_,ZeroLength,w)

      };

      ·(TM,_TM·CVT_) ·(TM,_TM·CVT_,ZeroLength,init)(
         ·(TM,_TM·CVT_,ZeroLength,StateParameter) *t
      ){
        t->hd       = true;
        ·(TM,_TM·CVT_) tm = {
           .state_parameter = (·(TM,_TM·CVT_,StateParameter) *)t
          ,.pft      = &·(TM,_TM·CVT_,ZeroLength,pft)
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

