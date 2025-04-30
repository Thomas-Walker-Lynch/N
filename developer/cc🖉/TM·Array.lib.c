/*
  Namespace: TM·Array

  Template parameters:

    `_TM·CVT_` Cell Value Type`.

*/
#include "TM.lib.c"

/*--------------------------------------------------------------------------------
 Interface 
--------------------------------------------------------------------------------*/

// once per translation unit
#ifndef TM·Array·FACE
#define TM·Array·FACE

  #define TM·Array·DEBUG
  #ifdef TM·Array·DEBUG
    #include <stdio.h>
  #endif

#endif

// once per _TM·CVT_ value
// Caller must #define SET_TM·Array·<CVT>, after inclusion, to prevent re-inclusion
#ifdef _TM·CVT_
#if BOOLEAN( NOT_IN(TM·Array ,_TM·CVT_) )

  #include "TM.Lib.c"

  #pragma push(T)
  #pragma push(TA)
  #pragma push(TE)

  #define T  ·(TM,_TM·CVT_)
  #define TA ·(TM,Array,_TM·CVT_)
  #define TE ·(extent_t,_TM·CVT_)

  #ifdef TM·Array·DEBUG
    #pragma message( "Declaring:" STR_VAL(TA) )
  #endif

     typedef struct{
       _TM·CVT_  *hd;
       _TM·CVT_  *position;
       TE extent;
     } ·(TA,Tableau);

     //  call signatures for the initialization functions
     //
     T ·(TA,init_pe)(
       ·(TA,Tableau) *t ,_TM·CVT_ position[] ,TE extent
     );

     T ·(TA,init_pp)(
        ·(TA,Tableau) *t ,_TM·CVT_ *position_left ,_TM·CVT_ *position_right
     );

  #pragma pop(T)
  #pragma pop(TA)
  #pragma pop(TE)

#endif
#endif

/*--------------------------------------------------------------------------------
  Local 
--------------------------------------------------------------------------------*/
#ifdef LOCAL

  // once per translation unit
  #ifndef TM·Array·LOCAL
  #define TM·Array·LOCAL

  #endif

  // once per _TM·CVT_ value
  #ifdef _TM·CVT_
  #if BOOLEAN( NOT_IN(TM·Array·LOCAL ,_TM·CVT_) )

    #include "TM.Lib.c"

    #pragma push(T)
    #pragma push(TA)
    #pragma push(TE)

    #define T  ·(TM,_TM·CVT_)
    #define TA ·(TM,Array,_TM·CVT_)
    #define TE ·(extent_t,_TM·CVT_)

    #ifdef TM·Array·DEBUG
      #pragma message( "LOCAL:" STR_VAL(TA) )
    #endif

      Local TM·Topo ·(TA,topo)(T tm){
        ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
        if( t->extent == 0 ) return TM·Topo·singleton;
        return TM·Topo·segment;
      }
      Local bool ·(TA,bounded)(T tm){
        ·(TA,Tableau) *t = (·(TA,Tableau) *) tm.tableau;
        return ·(TA,topo)(tm) & TM·Topo·bounded;
      }
      Local TE ·(TA,extent)(T tm){
        ·(TA,Tableau) *t = (·(TA,Tableau) *) tm.tableau;
        return t->extent;
      }

      Local TM·Status ·(TA,status)(T tm){
        ·(TA,Tableau) *t = (·(TA,Tableau) *) tm.tableau;
        if( !t->hd ) return TM·Status·dismounted;
        if( t->hd == t->position ) return TM·Status·leftmost;

        _TM·CVT_ *rightmost_pt = t->position + t->extent;
        if( t->hd == rightmost_pt ) return TM·Status·rightmost;
        if( t->hd < t->position || t->hd > rightmost_pt )
          return TM·Status·out_of_area;

        return TM·Status·interim;
      }

      Local bool ·(TA,dismounted)(T tm){
        return ·(TA,status)( tm ) & TM·Status·dismounted;
      }

      Local bool ·(TA,on_tape)(T tm){
        return ·(TA,status)( tm ) & TM·Status·on_tape;
      }

      Local bool ·(TA,on_leftmost)(T tm){
        return ·(TA,status)( tm ) & TM·Status·leftmost;
      }

      Local bool ·(TA,on_rightmost)(T tm){
        return ·(TA,status)( tm ) & TM·Status·rightmost;
      }

      // does nothing if tape is already mounted
      Local void ·(TA,mount)(T tm){
        ·(TA,Tableau) *t = (·(TA,Tableau) *) tm.tableau;
        if( !t->hd ) t->hd = t->position;
      }

      Local void ·(TA,dismount)(T tm){
        ·(TA,Tableau) *t = (·(TA,Tableau) *) tm.tableau;
        t->hd = NULL;
      }

      Local void ·(TA,step)(T tm){
        ·(TA,Tableau) *t = (·(TA,Tableau) *) tm.tableau;
        t->hd++;
      }

      Local void ·(TA,step_left)(T tm){
        ·(TA,Tableau) *t = (·(TA,Tableau) *) tm.tableau;
        t->hd--;
      }

      // rewind does nothing if the tape is dismounted
      Local void ·(TA,rewind)(T tm){
        ·(TA,Tableau) *t = (·(TA,Tableau) *) tm.tableau;
        if( ·(TA,dismounted)( tm ) ) return;
        t->hd = t->position;
      }

      Local _TM·CVT_ ·(TA,read)(T tm){
        ·(TA,Tableau) *t = (·(TA,Tableau) *) tm.tableau;
        return *t->hd;
      }

      Local void ·(TA,write)(T tm ,_TM·CVT_ *remote_pt){
        ·(TA,Tableau) *t = (·(TA,Tableau) *) tm.tableau;
        *t->hd = *remote_pt;
      }

      Local ·(TM,_TM·CVT_,PFT) ·(TA,pft) = {

        .topo           = ·(TA,topo)
        ,.bounded       = ·(TA,bounded)
        ,.extent        = ·(TA,extent)

        ,.status        = ·(TA,status)
        ,.dismounted    = ·(TA,dismounted)
        ,.on_tape       = ·(TA,on_tape)
        ,.on_leftmost   = ·(TA,on_leftmost)
        ,.on_rightmost  = ·(TA,on_rightmost)

        ,.mount         = ·(TA,mount)
        ,.dismount      = ·(TA,dismount)

        ,.step          = ·(TA,step)
        ,.step_left     = ·(TA,step_left)
        ,.rewind        = ·(TA,rewind)

        ,.read          = ·(TA,read)
        ,.write         = ·(TA,write)

      };

      /*
       tm is up casted from being array specific, to being generic. Later it is downcasted within the array code before being used. This can be seen at the top of each of the array specific function. This is the only loss of static type safety, and it is embedded in the library code.
      */
      ·(TM,_TM·CVT_) ·(TA,init_pe)(
         ·(TA,Tableau) *t
        ,_TM·CVT_ *position
        ,TE extent
      ){
        t->hd       = position;
        t->position = position;
        t->extent   = extent;

        T tm = {
           .tableau = (·(TM,_TM·CVT_,Tableau) *)t
          ,.pft      = &·(TA,pft)
        };

        return tm;
      }

      ·(TM,_TM·CVT_) ·(TA,init_pp)(
         ·(TA,Tableau) *t
        ,_TM·CVT_ *position_left
        ,_TM·CVT_ *position_right
      ){
        return ·(TA,init_pe)(
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
