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

  #define TM·SingletonCircle·DEBUG
  #ifdef TM·SingletonCircle·DEBUG
    #include <stdio.h>
  #endif

#endif //#ifndef TM·FACE

// once per _TM·CVT_ value
// Caller must #define SET_Binding__TM·SingletonCircle·<CVT>, after inclusion, to prevent re-inclusion
#ifdef _TM·CVT_
#if BOOLEAN( NOT_IN(Binding ,·(TM·SingletonCircle,_TM·CVT_)) )
#ifdef TM·DEBUG
  #pragma message( "Creating TM·SingletonCircle type with a CVT of:" STR_VAL(_TM·CVT_) )
#endif

  typedef struct{
    bool     hd;
    _TM·CVT_ value;
  } ·(TM,_TM·CVT_,SingletonCircle,StateParameter);

  ·(TM,_TM·CVT_) ·(TM,_TM·CVT_,SingletonCircle,init)(
     ·(TM,_TM·CVT_,SingletonCircle,StateParameter) *t
    ,_TM·CVT_ initial_value
  );

#endif // #if BOOLEAN( NOT_IN(Binding ,TM) )
#endif // #ifdef _TM·CVT_

/*--------------------------------------------------------------------------------
  Local 
--------------------------------------------------------------------------------*/
#ifdef LOCAL

  // once per translation unit
  #ifndef TM·SingletonCircle·LOCAL
  #define TM·SingletonCircle·LOCAL

  #endif

  // once per _TM·CVT_ value
  // Caller must #define SET_TM·LOCAL__TM·<CVT>, after inclusion, to prevent re-inclusion
  #ifdef _TM·CVT_
  #if BOOLEAN(NOT_IN(TM·SingletonCircle·LOCAL ,·(TM,_TM·CVT_)))
  #ifdef TM·SingletonCircle·DEBUG
    #pragma message( "Including TM·SingletonCircle LOCAL code for:" STR_VAL(_TM·CVT_) )
  #endif

    Local TM·Topo ·(TM,_TM·CVT_,SingletonCircle,topo)( ·(TM,_TM·CVT_) tm ){
      return TM·Topo·circle;
    }
    Local bool ·(TM,_TM·CVT_,SingletonCircle,bounded)( ·(TM,_TM·CVT_) tm ){
      return false;
    }
    Local ·(extent_t,_TM·CVT_) ·(TM,_TM·CVT_,SingletonCircle,extent)( ·(TM,_TM·CVT_) tm ){
      return 0;
    }
    Local TM·Status ·(TM,_TM·CVT_,SingletonCircle,status)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonCircle,StateParameter) *t = (·(TM,_TM·CVT_,SingletonCircle,StateParameter) *) tm.state_parameter;
      if( !t->hd ) return TM·Status·dismounted;
      return TM·Status·interim;
    }
    Local bool ·(TM,_TM·CVT_,SingletonCircle,dismounted)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonCircle,StateParameter) *t = (·(TM,_TM·CVT_,SingletonCircle,StateParameter) *) tm.state_parameter;
      return !t->hd;
    }

    // these could be pulled from the tm.pft table, and defined once
    Local bool ·(TM,_TM·CVT_,SingletonCircle,on_tape)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonCircle,StateParameter) *t = (·(TM,_TM·CVT_,SingletonCircle,StateParameter) *) tm.state_parameter;
      return t->hd;
    }
    Local bool ·(TM,_TM·CVT_,SingletonCircle,on_leftmost)( ·(TM,_TM·CVT_) tm ){
      return false;
    }
    Local bool ·(TM,_TM·CVT_,SingletonCircle,on_rightmost)( ·(TM,_TM·CVT_) tm ){
      return false;
    }

    // does nothing if tape is already mounted
    Local void ·(TM,_TM·CVT_,SingletonCircle,mount)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonCircle,StateParameter) *t = (·(TM,_TM·CVT_,SingletonCircle,StateParameter) *) tm.state_parameter;
      t->hd = true;
    }
    Local void ·(TM,_TM·CVT_,SingletonCircle,dismount)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonCircle,StateParameter) *t = (·(TM,_TM·CVT_,SingletonCircle,StateParameter) *) tm.state_parameter;
      t->hd = false;
    }

    Local void ·(TM,_TM·CVT_,SingletonCircle,s)( ·(TM,_TM·CVT_) tm ){
      return;
    }
    Local void ·(TM,_TM·CVT_,SingletonCircle,step_left)( ·(TM,_TM·CVT_) tm ){
      return;
    }
    Local void ·(TM,_TM·CVT_,SingletonCircle,rewind)( ·(TM,_TM·CVT_) tm ){
      return;
    }

    Local _TM·CVT_ ·(TM,_TM·CVT_,SingletonCircle,r)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonCircle,StateParameter) *t = (·(TM,_TM·CVT_,SingletonCircle,StateParameter) *) tm.state_parameter;
      return t->value;
    }
    Local void ·(TM,_TM·CVT_,SingletonCircle,w)( ·(TM,_TM·CVT_) tm ,_TM·CVT_ *remote_pt ){
      ·(TM,_TM·CVT_,SingletonCircle,StateParameter) *t = (·(TM,_TM·CVT_,SingletonCircle,StateParameter) *) tm.state_parameter;
      t->value = *remote_pt;
    }

    Local ·(TM,_TM·CVT_,PFT) ·(TM,_TM·CVT_,SingletonCircle,pft) = {

      .topo           = ·(TM,_TM·CVT_,SingletonCircle,topo)
      ,.bounded       = ·(TM,_TM·CVT_,SingletonCircle,bounded)
      ,.extent        = ·(TM,_TM·CVT_,SingletonCircle,extent)

      ,.status        = ·(TM,_TM·CVT_,SingletonCircle,status)
      ,.dismounted    = ·(TM,_TM·CVT_,SingletonCircle,dismounted)
      ,.on_tape       = ·(TM,_TM·CVT_,SingletonCircle,on_tape)
      ,.on_leftmost   = ·(TM,_TM·CVT_,SingletonCircle,on_leftmost)
      ,.on_rightmost  = ·(TM,_TM·CVT_,SingletonCircle,on_rightmost)

      ,.mount         = ·(TM,_TM·CVT_,SingletonCircle,mount)
      ,.dismount      = ·(TM,_TM·CVT_,SingletonCircle,dismount)

      ,.step          = ·(TM,_TM·CVT_,SingletonCircle,s)
      ,.step_left     = ·(TM,_TM·CVT_,SingletonCircle,step_left)
      ,.rewind        = ·(TM,_TM·CVT_,SingletonCircle,rewind)

      ,.r          = ·(TM,_TM·CVT_,SingletonCircle,r)
      ,.w         = ·(TM,_TM·CVT_,SingletonCircle,w)

    };

    /*
     tm is up casted from being array specific, to being generic. Later it is downcasted within the array code before being used. This can be seen at the top of each of the array specific function. This is the only loss of static type safety, and it is embedded in the library code.
    */
    ·(TM,_TM·CVT_) ·(TM,_TM·CVT_,SingletonCircle,init)(
        ·(TM,_TM·CVT_,SingletonCircle,StateParameter) *t
       ,_TM·CVT_ initial_value
    ){
      t->hd    = true;
      t->value = initial_value;

      ·(TM,_TM·CVT_) tm = {
         .state_parameter = (·(TM,_TM·CVT_,StateParameter) *)t
        ,.pft      = &·(TM,_TM·CVT_,SingletonCircle,pft)
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

