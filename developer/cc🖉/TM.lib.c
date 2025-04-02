/*
  TM - Tape Machine Model

  User must define the equivalence template:

      EQ_TM·<TM·CVT>__oo__TM·<TM·CVT>

    While filling in the CVT values of course. For pointers, struct, arrays, etc. use a typedef to reduce the type to an identifier.

  Template parameters

    `TM·CVT` Cell Value Type`.

*/

//--------------------------------------------------------------------------------
// Interface 
//--------------------------------------------------------------------------------

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

#endif

#ifdef TM·CVT
#if BOOLEAN(NOT_IN(TM·SET ,TM·CVT))
#ifdef TM·CVT·DEBUG
  #pragma message( "adding binding for:" STR_VAL(Binding) )
#endif

  // some synonyms to make this section easier to read
  #undef TM
  #define TM Ξ(TM ,TM·CVT)
  #undef EXTENT_T
  #define EXTENT_T Ξ(extent_t ,TM·CVT)

  // declare 'TM·CVT' as a type 
  // (or more specifically, make a struct for binding the FG table to the tableau)
  #undef Binding·Type
  #define Binding·Type TM
  #include "Binding.lib.c"

  // `extent_t` is an index, so element size matters, so I associate it with TM·CVT
  typedef Ξ(extent_t ,TM·CVT) size_t;

  typedef struct Ξ(TM ,FG){

    TM·Tape·Topo (*Tape·topo)(TM tm);
    bool (*Tape·bounded)(TM tm);

    TM·Head·Status (*Head·status)(TM tm);
    bool (*Head·on_tape)(TM tm);
    bool (*Head·on_leftmost) (TM tm);
    bool (*Head·on_rightmost)(TM tm);

    // tape machine functions
    Core·Status (*mount)   (TM tm);
    Core·Status (*dismount)(TM tm);

    void (*step)      (TM tm);
    void (*step_left) (TM tm);
    void (*rewind)    (TM tm);

    EXTENT_T (*extent)(TM tm);
    TM·CVT   (*read)  (TM tm);
    void     (*write) (TM tm ,TM·CVT *remote_pt);

  } Ξ(TM ,FG);

  //----------------------------------------
  // Array interface

  #undef  TM·ARRAY 
  #define TM·ARRAY Ξ(TM ,ARRAY)

  struct Ξ(TM·ARRAY ,Tableau);
    
  /*
    We assume that the binding produced by init is valid. Thus it does not have null pointers to the tableau or the fg table; it points to an initialized tableau; it points to a valid array fg table; and, that the fg table and tableau go together.

    Calling init is the only time the user/programmer will need to know the implementation name.

    The resulting binding object returned is what the user will call an instance of the
    type.
  */
    TM Ξ(TM·ARRAY ,init_pe)( 
       struct Ξ(TM·ARRAY ,Tableau) *t
       ,TM·CVT position[] 
       ,EXTENT_T extent 
     );

     TM Ξ(TM·ARRAY ,init_pp)( 
       Ξ(TM·ARRAY ,Tableau) *t
       ,TM·CVT *position_left 
       ,TM·CVT *position_right 
     );

#endif 


//--------------------------------------------------------------------------------
// Local - at bottom of translation unit, to keep some functions private
//--------------------------------------------------------------------------------
#ifdef LOCAL

// once per translation unit
#ifndef TM·LOCAL·TYPE_LIST
#define TM·LOCAL·TYPE_LIST

  const char *TM·Msg·tm="given NULL tm";
  const char *TM·Msg·flag="given NULL flag pointer";
  const char *TM·Msg·result="given NULL result pointer";
  const char *TM·Msg·status="bad head status";

#endif

// once per TM·CVT value
#ifdef TM·CVT
#if NOT_CONTAINS( TM·CVT ,TM·LOCAL·TYPE_LIST )
#ifdef Binding·DEBUG
  #pragma message( STR_VAL(TM·LOCAL·TYPE_LIST) )
#endif

  //this is what it takes to append to a list in cpp ...
  #undef TEMP
  #define TEMP TM·LOCAL·TYPE_LIST ,TM·CVT
  #undef TM·LOCAL·TYPE_LIST
  #define TM·LOCAL·TYPE_LIST TEMP

  // some aliases to make things a little easier to read
  #undef TM
  #define TM Ξ(TM ,TM·CVT)
  #undef EXTENT_T
  #define EXTENT_T Ξ(extent_t ,TM·CVT)

  //----------------------------------------
  // TM Array implementation, not TM·CVT differentiated

  // some aliases
  #undef  TM·ARRAY 
  #define TM·ARRAY Ξ(TM ,ARRAY)

  typedef struct Ξ(TM·ARRAY ,Tableau){
    TM·CVT *hd;
    TM·CVT position[];
    EXTENT_T extent;
  } Ξ(TM·ARRAY ,Tableau);


  typedef struct Ξ(TM·ARRAY ,Tableau){
    TM·CVT *hd;
    TM·CVT *position;
    Ξ(extent_t ,TM·CVT) extent;
  } Ξ(TM·Array ,TM·CVT)·Tableau;

  // with a direct interface, an array can implement three among the possible tape topologies
  Local TM·Tape·Topo Ξ(TM·ARRAY ,Tape·topo)(TM tm){
    Ξ(TM·ARRAY ,Tableau) *t = (Ξ(TM·ARRAY ,Tableau) *) tm.tableau;
    if(t->extent == 0) return TM·Tape·Topo·singleton; 
    return TM·Tape·Topo·segment;
  }
  Local bool Ξ(TM·ARRAY ,bounded)(TM tm){
    Ξ(TM·ARRAY ,Tableau) *t = (Ξ(TM·ARRAY ,Tableau) *) tm.tableau;
    return Ξ(TM·ARRAY ,Tape·topo)(tm) & TM·Tape·Topo·bounded;
  }

  Local TM·Head·Status Ξ(TM·ARRAY ,Head·status)(TM tm){
    Ξ(TM·ARRAY ,Tableau) *t = (Ξ(TM·ARRAY ,Tableau) *) tm.tableau;
    if(!t->hd) return TM·Head·Status·dismounted;
    if(t->hd == tm->position) return TM·Head·Status·leftmost;

    TM·CVT *rightmost_pt = t->position + t->extent;
    if(t->hd == rightmost_pt) TM·Head·Status·rightmost;
    if(t->hd < tm->position || tm->hd > rightmost_pt)
      return TM·Head·Status·out_of_area;

    return TM·Head·Status·interim;
  }
  Local bool Ξ(TM·ARRAY ,Head·on_tape)(TM tm){
    Ξ(TM·ARRAY ,Tableau) *t = (Ξ(TM·ARRAY ,Tableau) *) tm.tableau;
    return Ξ(TM·ARRAY ,Head·status)(tm) & TM·Head·Status·on_tape;
  }


  Ξ(TM ,FG) TM·ARRAY·fg = {

  }


    TM Ξ(TM·ARRAY ,init_pe)( 
       struct Ξ(TM·ARRAY ,Tableau) *t
       ,TM·CVT position[] 
       ,EXTENT_T extent 
     );

     TM Ξ(TM·ARRAY ,init_pp)( 
       Ξ(TM·ARRAY ,Tableau) *t
       ,TM·CVT *position_left 
       ,TM·CVT *position_right 
     );

--
  TM·Tape·Topo  TM·Array·Tape·mount(TM·Tableau *t){
    if(!t || !t->position) return T·Tape·Topo·mu;
    if(t->extent == 0) TM·Tape·Topo·singleton; 
    return TM·Tape·Topo·segment;
  }

  Local Ξ(extent_t ,TM·CVT) Ξ(TM ,TM·CVT)·extent(TM *tm){
    #ifdef TM·DEBUG
      Core·Guard·init_count(chk);
      Core·Guard·fg.check(&chk ,1 ,TM·Tape·bounded(tm) ,"Tape is not bounded.");
      Core·Guard·assert(chk);
    #endif
    return tm->fg.extent(tm);
  }

  Local TM·CVT TM·read(TM *tm){
    #ifdef TM·DEBUG
      Core·Guard·init_count(chk);
      Core·Guard·fg.check( &chk ,1 ,TM·head_on_tape(tm) ,TM·Msg·head);
      Core·Guard·assert(chk);
    #endif
    return tm->fg.read(tm);
  }

  Local void TM·write(TM *tm ,TM·CVT *write_pt){
    #ifdef TM·DEBUG
      Core·Guard·init_count(chk); 
      Core·Guard·fg.check( &chk ,1 ,TM·head_on_tape(tm) ,TM·Msg·head); 
      Core·Guard·fg.check( &chk ,1 ,write_pt ,"Given NULL write_pt"); 
      Core·Guard·assert(chk);
    #endif
    return tm->fg.write(tm ,write_pt);
  }

  Local Ξ(TM ,TM·CVT)·Binding Ξ(TM ,TM·CVT)·fg = {
    .parent = TM·fg
    ,.extent = Ξ(TM ,TM·CVT)·extent
    ,.read = Ξ(TM ,TM·CVT)·read
    ,.write = Ξ(TM ,TM·CVT)·write
  };

#endif // ifdef TM·CVT
#undef TM·CVT


//--------------------------------------------------------------------------------
// Library - compiled into a lib.a file by the current make
//   Core currently has no library components
//--------------------------------------------------------------------------------
#ifdef LIBRARY
#endif 

//--------------------------------------------------------------------------------
// undef the template parameters
//--------------------------------------------------------------------------------
#undef TM·CVT
