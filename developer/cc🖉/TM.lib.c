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
    void               (*step_right)      ( ·(TM,_TM·CVT_) tm );
    void               (*step_left)       ( ·(TM,_TM·CVT_) tm );
    void               (*rewind)          ( ·(TM,_TM·CVT_) tm );

    _TM·CVT_           (*read)            ( ·(TM,_TM·CVT_) tm );
    void               (*write)           ( ·(TM,_TM·CVT_) tm ,_TM·CVT_ *remote_pt );

  } ·(TM,_TM·CVT_,FG);

  //----------------------------------------
  // ZeroLength tape interface

    typedef struct{
      bool hd;
    } ·(TM,_TM·CVT_,ZeroLength,Tableau);

    ·(TM,_TM·CVT_) ·(TM,_TM·CVT_,ZeroLength,init)(
       ·(TM,_TM·CVT_,ZeroLength,Tableau) *t
    );

  //----------------------------------------
  // SingletonSegment tape interface

    typedef struct{
      bool     hd;
      _TM·CVT_ value;
    } ·(TM,_TM·CVT_,SingletonSegment,Tableau);

    ·(TM,_TM·CVT_) ·(TM,_TM·CVT_,SingletonSegment,init)(
       ·(TM,_TM·CVT_,SingletonSegment,Tableau) *t
      ,_TM·CVT_ initial_value
    );

  //----------------------------------------
  // SingletonCircle tape interface

    typedef struct{
      bool     hd;
      _TM·CVT_ value;
    } ·(TM,_TM·CVT_,SingletonCircle,Tableau);

    ·(TM,_TM·CVT_) ·(TM,_TM·CVT_,SingletonCircle,init)(
       ·(TM,_TM·CVT_,SingletonCircle,Tableau) *t
      ,_TM·CVT_ initial_value
    );

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
      ZeroLength tape implementation
    */

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
        ·(TM,_TM·CVT_,ZeroLength,Tableau) *t = (·(TM,_TM·CVT_,ZeroLength,Tableau) *) tm.tableau;
        if( !t->hd ) return TM·Status·dismounted;
        return TM·Status·out_of_area;
      }

      Local bool ·(TM,_TM·CVT_,ZeroLength,dismounted)( ·(TM,_TM·CVT_) tm ){
        ·(TM,_TM·CVT_,ZeroLength,Tableau) *t = (·(TM,_TM·CVT_,ZeroLength,Tableau) *) tm.tableau;
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
        ·(TM,_TM·CVT_,ZeroLength,Tableau) *t = (·(TM,_TM·CVT_,ZeroLength,Tableau) *) tm.tableau;
        if( !t->hd ) t->hd = true;
      }

      Local void ·(TM,_TM·CVT_,ZeroLength,dismount)( ·(TM,_TM·CVT_) tm ){
        ·(TM,_TM·CVT_,ZeroLength,Tableau) *t = (·(TM,_TM·CVT_,ZeroLength,Tableau) *) tm.tableau;
        t->hd = false;
      }

      Local void ·(TM,_TM·CVT_,ZeroLength,step)( ·(TM,_TM·CVT_) tm ){
        assert(0);
      }

      Local void ·(TM,_TM·CVT_,ZeroLength,step_left)( ·(TM,_TM·CVT_) tm ){
        assert(0);
      }

      // rewind does nothing if the tape is dismounted
      Local void ·(TM,_TM·CVT_,ZeroLength,rewind)( ·(TM,_TM·CVT_) tm ){
        ·(TM,_TM·CVT_,ZeroLength,Tableau) *t = (·(TM,_TM·CVT_,ZeroLength,Tableau) *) tm.tableau;
        if( ·(TM,_TM·CVT_,ZeroLength,dismounted)( tm ) ) return;
        assert(0);
      }

      Local _TM·CVT_ ·(TM,_TM·CVT_,ZeroLength,read)( ·(TM,_TM·CVT_) tm ){
        assert(0);
      }

      Local void ·(TM,_TM·CVT_,ZeroLength,write)( ·(TM,_TM·CVT_) tm ,_TM·CVT_ *remote_pt ){
        assert(0);
      }

      Local ·(TM,_TM·CVT_,FG) ·(TM,_TM·CVT_,ZeroLength,fg) = {

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

        ,.step          = ·(TM,_TM·CVT_,ZeroLength,step)
        ,.step_right    = ·(TM,_TM·CVT_,ZeroLength,step)
        ,.step_left     = ·(TM,_TM·CVT_,ZeroLength,step_left)
        ,.rewind        = ·(TM,_TM·CVT_,ZeroLength,rewind)

        ,.read          = ·(TM,_TM·CVT_,ZeroLength,read)
        ,.write         = ·(TM,_TM·CVT_,ZeroLength,write)

      };

      ·(TM,_TM·CVT_) ·(TM,_TM·CVT_,ZeroLength,init)(
         ·(TM,_TM·CVT_,ZeroLength,Tableau) *t
      ){
        t->hd       = true;
        ·(TM,_TM·CVT_) tm = {
           .tableau = (·(TM,_TM·CVT_,Tableau) *)t
          ,.fg      = &·(TM,_TM·CVT_,ZeroLength,fg)
        };
        return tm;
      }

    /*------------------------------------------------------------------------
      SingletonSegment tape
    */

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

    // these could be pulled from the tm.fg table, and defined once
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

    Local void ·(TM,_TM·CVT_,SingletonSegment,step)( ·(TM,_TM·CVT_) tm ){
      assert(0);
    }
    Local void ·(TM,_TM·CVT_,SingletonSegment,step_left)( ·(TM,_TM·CVT_) tm ){
      assert(0);
    }
    Local void ·(TM,_TM·CVT_,SingletonSegment,rewind)( ·(TM,_TM·CVT_) tm ){
      return;
    }

    Local _TM·CVT_ ·(TM,_TM·CVT_,SingletonSegment,read)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonSegment,Tableau) *t = (·(TM,_TM·CVT_,SingletonSegment,Tableau) *) tm.tableau;
      return t->value;
    }
    Local void ·(TM,_TM·CVT_,SingletonSegment,write)( ·(TM,_TM·CVT_) tm ,_TM·CVT_ *remote_pt ){
      ·(TM,_TM·CVT_,SingletonSegment,Tableau) *t = (·(TM,_TM·CVT_,SingletonSegment,Tableau) *) tm.tableau;
      t->value = *remote_pt;
    }

    Local ·(TM,_TM·CVT_,FG) ·(TM,_TM·CVT_,SingletonSegment,fg) = {

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

      ,.step          = ·(TM,_TM·CVT_,SingletonSegment,step)
      ,.step_right    = ·(TM,_TM·CVT_,SingletonSegment,step)
      ,.step_left     = ·(TM,_TM·CVT_,SingletonSegment,step_left)
      ,.rewind        = ·(TM,_TM·CVT_,SingletonSegment,rewind)

      ,.read          = ·(TM,_TM·CVT_,SingletonSegment,read)
      ,.write         = ·(TM,_TM·CVT_,SingletonSegment,write)

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
        ,.fg      = &·(TM,_TM·CVT_,SingletonSegment,fg)
      };

      return tm;
    }


    /*------------------------------------------------------------------------
      SingletonCircle tape
    */

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
      ·(TM,_TM·CVT_,SingletonCircle,Tableau) *t = (·(TM,_TM·CVT_,SingletonCircle,Tableau) *) tm.tableau;
      if( !t->hd ) return TM·Status·dismounted;
      return TM·Status·interim;
    }
    Local bool ·(TM,_TM·CVT_,SingletonCircle,dismounted)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonCircle,Tableau) *t = (·(TM,_TM·CVT_,SingletonCircle,Tableau) *) tm.tableau;
      return !t->hd;
    }

    // these could be pulled from the tm.fg table, and defined once
    Local bool ·(TM,_TM·CVT_,SingletonCircle,on_tape)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonCircle,Tableau) *t = (·(TM,_TM·CVT_,SingletonCircle,Tableau) *) tm.tableau;
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
      ·(TM,_TM·CVT_,SingletonCircle,Tableau) *t = (·(TM,_TM·CVT_,SingletonCircle,Tableau) *) tm.tableau;
      t->hd = true;
    }
    Local void ·(TM,_TM·CVT_,SingletonCircle,dismount)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonCircle,Tableau) *t = (·(TM,_TM·CVT_,SingletonCircle,Tableau) *) tm.tableau;
      t->hd = false;
    }

    Local void ·(TM,_TM·CVT_,SingletonCircle,step)( ·(TM,_TM·CVT_) tm ){
      return;
    }
    Local void ·(TM,_TM·CVT_,SingletonCircle,step_left)( ·(TM,_TM·CVT_) tm ){
      return;
    }
    Local void ·(TM,_TM·CVT_,SingletonCircle,rewind)( ·(TM,_TM·CVT_) tm ){
      return;
    }

    Local _TM·CVT_ ·(TM,_TM·CVT_,SingletonCircle,read)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,SingletonCircle,Tableau) *t = (·(TM,_TM·CVT_,SingletonCircle,Tableau) *) tm.tableau;
      return t->value;
    }
    Local void ·(TM,_TM·CVT_,SingletonCircle,write)( ·(TM,_TM·CVT_) tm ,_TM·CVT_ *remote_pt ){
      ·(TM,_TM·CVT_,SingletonCircle,Tableau) *t = (·(TM,_TM·CVT_,SingletonCircle,Tableau) *) tm.tableau;
      t->value = *remote_pt;
    }

    Local ·(TM,_TM·CVT_,FG) ·(TM,_TM·CVT_,SingletonCircle,fg) = {

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

      ,.step          = ·(TM,_TM·CVT_,SingletonCircle,step)
      ,.step_right    = ·(TM,_TM·CVT_,SingletonCircle,step)
      ,.step_left     = ·(TM,_TM·CVT_,SingletonCircle,step_left)
      ,.rewind        = ·(TM,_TM·CVT_,SingletonCircle,rewind)

      ,.read          = ·(TM,_TM·CVT_,SingletonCircle,read)
      ,.write         = ·(TM,_TM·CVT_,SingletonCircle,write)

    };

    /*
     tm is up casted from being array specific, to being generic. Later it is downcasted within the array code before being used. This can be seen at the top of each of the array specific function. This is the only loss of static type safety, and it is embedded in the library code.
    */
    ·(TM,_TM·CVT_) ·(TM,_TM·CVT_,SingletonCircle,init)(
        ·(TM,_TM·CVT_,SingletonCircle,Tableau) *t
       ,_TM·CVT_ initial_value
    ){
      t->hd    = true;
      t->value = initial_value;

      ·(TM,_TM·CVT_) tm = {
         .tableau = (·(TM,_TM·CVT_,Tableau) *)t
        ,.fg      = &·(TM,_TM·CVT_,SingletonCircle,fg)
      };

      return tm;
    }


    /*------------------------------------------------------------------------
      Array implementation with a segment tape
    */

    Local TM·Topo ·(TM,_TM·CVT_,Array,topo)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      if( t->extent == 0 ) return TM·Topo·singleton;
      return TM·Topo·segment;
    }

    Local bool ·(TM,_TM·CVT_,Array,bounded)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      return ·(TM,_TM·CVT_,Array,topo)( tm ) & TM·Topo·bounded;
    }

    Local ·(extent_t,_TM·CVT_) ·(TM,_TM·CVT_,Array,extent)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      return t->extent;
    }


    Local TM·Status ·(TM,_TM·CVT_,Array,status)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      if( !t->hd ) return TM·Status·dismounted;
      if( t->hd == t->position ) return TM·Status·leftmost;

      _TM·CVT_ *rightmost_pt = t->position + t->extent;
      if( t->hd == rightmost_pt ) return TM·Status·rightmost;
      if( t->hd < t->position || t->hd > rightmost_pt )
        return TM·Status·out_of_area;

      return TM·Status·interim;
    }

    Local bool ·(TM,_TM·CVT_,Array,dismounted)( ·(TM,_TM·CVT_) tm ){
      return ·(TM,_TM·CVT_,Array,status)( tm ) & TM·Status·dismounted;
    }

    Local bool ·(TM,_TM·CVT_,Array,on_tape)( ·(TM,_TM·CVT_) tm ){
      return ·(TM,_TM·CVT_,Array,status)( tm ) & TM·Status·on_tape;
    }

    Local bool ·(TM,_TM·CVT_,Array,on_leftmost)( ·(TM,_TM·CVT_) tm ){
      return ·(TM,_TM·CVT_,Array,status)( tm ) & TM·Status·leftmost;
    }

    Local bool ·(TM,_TM·CVT_,Array,on_rightmost)( ·(TM,_TM·CVT_) tm ){
      return ·(TM,_TM·CVT_,Array,status)( tm ) & TM·Status·rightmost;
    }

    // does nothing if tape is already mounted
    Local void ·(TM,_TM·CVT_,Array,mount)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      if( !t->hd ) t->hd = t->position;
    }

    Local void ·(TM,_TM·CVT_,Array,dismount)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      t->hd = NULL;
    }

    Local void ·(TM,_TM·CVT_,Array,step)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      t->hd++;
    }

    Local void ·(TM,_TM·CVT_,Array,step_left)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      t->hd--;
    }

    // rewind does nothing if the tape is dismounted
    Local void ·(TM,_TM·CVT_,Array,rewind)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      if( ·(TM,_TM·CVT_,Array,dismounted)( tm ) ) return;
      t->hd = t->position;
    }

    Local _TM·CVT_ ·(TM,_TM·CVT_,Array,read)( ·(TM,_TM·CVT_) tm ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      return *t->hd;
    }

    Local void ·(TM,_TM·CVT_,Array,write)( ·(TM,_TM·CVT_) tm ,_TM·CVT_ *remote_pt ){
      ·(TM,_TM·CVT_,Array,Tableau) *t = (·(TM,_TM·CVT_,Array,Tableau) *) tm.tableau;
      *t->hd = *remote_pt;
    }

    Local ·(TM,_TM·CVT_,FG) ·(TM,_TM·CVT_,Array,fg) = {

      .topo           = ·(TM,_TM·CVT_,Array,topo)
      ,.bounded       = ·(TM,_TM·CVT_,Array,bounded)
      ,.extent        = ·(TM,_TM·CVT_,Array,extent)

      ,.status        = ·(TM,_TM·CVT_,Array,status)
      ,.dismounted    = ·(TM,_TM·CVT_,Array,dismounted)
      ,.on_tape       = ·(TM,_TM·CVT_,Array,on_tape)
      ,.on_leftmost   = ·(TM,_TM·CVT_,Array,on_leftmost)
      ,.on_rightmost  = ·(TM,_TM·CVT_,Array,on_rightmost)

      ,.mount         = ·(TM,_TM·CVT_,Array,mount)
      ,.dismount      = ·(TM,_TM·CVT_,Array,dismount)

      ,.step          = ·(TM,_TM·CVT_,Array,step)
      ,.step_right    = ·(TM,_TM·CVT_,Array,step)
      ,.step_left     = ·(TM,_TM·CVT_,Array,step_left)
      ,.rewind        = ·(TM,_TM·CVT_,Array,rewind)

      ,.read          = ·(TM,_TM·CVT_,Array,read)
      ,.write         = ·(TM,_TM·CVT_,Array,write)

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
