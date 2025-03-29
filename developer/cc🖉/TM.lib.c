/*
  TM - Tape Machine Model

   Template parameters

    `TM·CVT` Cell Value Type`.

*/

//--------------------------------------------------------------------------------
// Interface 

// once per translation unit
#ifndef TM·TYPE_LIST
#define TM·TYPE_LIST

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

// once per TM·CVT value
#ifdef TM·CVT
#if ! FIND_ITEM( TM·CVT ,TM·TYPE_LIST )
#define TM·TYPE_LIST APPEND(TM·TYPE_LIST ,TM·TYPE)

  // declare 'TM' as a type
  #define EQ__TM__oo__TM
  #define Binding·Type TM 
  #include "Binding.lib.c"
  
  // `extent_t` is an index, so element size matters, so TM·CVT matters
  typedef Ξ(extent_t ,TM·CVT) size_t;
                           
  typedef struct{

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

    TM·Binding TM·fg; // points to TM·Binding instance
    Ξ(extent_t ,TM·CVT) (*extent)(TM tm);
    TM·CVT  (*read) (TM tm);
    void (*write)(TM tm ,TM·CVT *remote_pt);

  } Ξ(TM ,FG);

  //----------------------------------------
  // Array interface
  //----------------------------------------

  typedef struct Ξ(TM ,Array)·Tableau;
                                
  TM Ξ(TM ,Array)·init_pe( 
     Ξ(TM ,Array)·Tableau *t
    ,TM·CVT position[] 
    ,Ξ(extent_t ,TM·CVT) extent 
  );

  TM Ξ(TM ,Array)·init_pp( 
     Ξ(TM ,Array)·Tableau *t
    ,TM·CVT *position_left 
    ,TM·CVT *position_right 
  );

#endif 
#endif 

//--------------------------------------------------------------------------------
// Implementation

#ifdef IMPLEMENTATION

  // implementation to go into the lib.a file
  //
    #ifndef LOCAL
    #endif 

  #ifdef LOCAL

    //----------------------------------------
    // included first without defining TM·CVT
    //----------------------------------------

    #ifndef TM·CVT

      //-----------------------------------
      // common error messages

      const char *TM·Msg·tm="given NULL tm";
      const char *TM·Msg·position=
        "Null position.This is only possible when the tape machine has not been initialized.";
      const char *TM·Msg·flag="given NULL flag pointer";
      const char *TM·Msg·result="given NULL result pointer";
      const char *TM·Msg·head="head not on tape";

      //-----------------------------------
      // generic call wrappers

      Local TM·Tape·Topo TM·Tape·topo(TM *tm){
        return tm->fg.Tape·topo(tm);
      }
      Local TM·Tape·Topo TM·Tape·bounded(TM *tm){
        return TM·tape_top(tm) & TM·Tape·Topo·bounded);
      }

      Local TM·Head·Status TM·head_status(TM *tm){
        return tm->fg.status(tm ,result);
      }
      Local TM·Head·Status TM·head_on_tape(TM *tm){
        return TM·status(tm) & TM·Head·Status·on_tape;
      }

      // does nothing if the head is already mounted
      Local Core·Status TM·mount(TM *tm){
        #ifdef TM·DEBUG 
          Core·Guard·init_count(chk); 
          Core·Guard·fg.check(&chk, 1, tm, TM·Msg·tm); 
          Core·Guard·if_return(chk); 
        #endif 
        if( !TM·head_on_tape(tm) ) return Core·Status·on_track;
        return tm->fg.mount(tm); 
      }

      // does nothing if the head is already dismounted
      Local Core·Status TM·dismount(TM *tm){ 
        #ifdef TM·DEBUG 
          Core·Guard·init_count(chk); 
          Core·Guard·fg.check(&chk, 1, tm, TM·Msg·tm); 
          Core·Guard·if_return(chk); 
        #endif
        if( TM·head_status(TM *tm) & TM·Head·Status·dismounted) ) return Core·Status·on_track;
        return tm->fg.dismount(tm); 
      }
   
      #define TM·macro·bool_tm(name)
      Local bool name(TM *tm){ \
        #ifdef TM·DEBUG \
          Core·Guard·init_count(chk); \
          Core·Guard·fg.check( &chk ,1 ,TM·head_on_tape(tm) ,TM·Msg·head); \
          Core·Guard·assert(chk); \
        #endif \
        return tm->fg.name(tm); \
      } 

      Local bool TM·macro·bool_tm(on_leftmost);
      Local bool TM·macro·bool_tm(on_rightmost);
      Local bool TM·macro·bool_tm(step);
      Local bool TM·macro·bool_tm(step_left);
      Local bool TM·macro·bool_tm(rewind);

      //----------------------------------------
      // Initialization for TM·fg

      Local TM·Binding TM·fg = {
        .Tape·topo = TM·Tape·topo
        ,.Tape·bounded = TM·Tape·bounded
        ,.Head·status = TM·Head·status
        ,.Head·on_tape = TM·Head·on_tape

        ,.Head·on_leftmost = TM·Head·on_leftmost
        ,.Head·on_rightmost = TM·Head·on_rightmost

        ,.mount    = TM·mount
        ,.dismount = TM·dismount

        ,.step = TM·step
        ,.step_left = TM·step_left
        ,.step_right = TM·step // Synonym
        ,.rewind = TM·rewind

      };

    #endif // ifndef TM·CVT

    //-----------------------------------
    // TM·CVT dependent functions

    #ifdef TM·CVT

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


    //----------------------------------------
    // TM·Array implementation
    //----------------------------------------

    /*
      TM·Array is bound to a tape with head on the tape via mount_pe or mount_pp.  

      Once a tape is bound, it remains so, though the head can be dismounted
      and remounted.

      C lacks an allocation-bound initialization feature, so the library user is responsible
      to make sure a TM·Array has been initialized before use.

      The TM·Array has no locking facility, so it is not possible to know if it is in use.

      Re-initializing a TM·Array while in use can lead to unspecified badness. Use `rewind` instead.
    */

    //----------------------------------------
    // TM·Array implementation
    //----------------------------------------

    #ifndef TM·CVT

      //-----------------------------------
      // common error messages

      const char *TM·Array·Msg·tm="given NULL tm";
      const char *TM·Array·Msg·flag="given NULL flag pointer";
      const char *TM·Array·Msg·result="given NULL result pointer";
      const char *TM·Array·Msg·status="bad head status";
        

    #endif // #ifndef TM·CVT

    #ifdef TM·CVT

      typedef struct Ξ(TM ,TM·CVT)·Tableau;

      typedef struct{
        TM·CVT *hd;
        TM·CVT *position;
        Ξ(extent_t ,TM·CVT) extent;
      }Ξ(TM·Array ,TM·CVT)·Tableau;

      //-----------------------------------

      // TM·Array.tape implementation

      //----------------------------------------
      // TM Array implementation, not TM·CVT differentiated

      Local TM·Tape·Topo TM·Array·Tape·topo( Ξ(TM ,TM·CVT)·Tableau *tableau ){
        Ξ(TM·Array ,TM·CVT)·Tableau *t = (Ξ(TM·Array ,TM·CVT)·Tableau *)tableau;
        if(!t || !t->position) return T·Tape·Topo·mu;
        if(t->extent == 0) TM·Tape·Topo·singleton; 
        return TM·Tape·Topo·segment;
      }
      Local TM·Tape·Topo TM·Tape·bounded(TM·Tableau *t){
        return TM·tape_top(t) & TM·Tape·Topo·bounded;
      }

      TM·Tape·Topo  TM·Array·Tape·mount(TM·Tableau *t){
        if(!t || !t->position) return T·Tape·Topo·mu;
        if(t->extent == 0) TM·Tape·Topo·singleton; 
        return TM·Tape·Topo·segment;
      }

       // For an Array Tape Machine ,a bound tape will be singleton or segment.
       TM·Tape·Topo  Ξ(TM·Array ,TM·CVT)·Tape·topo(Ξ(TM·Array ,TM·CVT) *tm){
         if(!tm || !tm->position) return TM·Tape·Topo·mu;
         if(tm->extent == 0) TM·Tape·Topo·singleton; 
         return TM·Tape·Topo·segment;
      }

      // check the Tape·topo to make sure tape has extent before calling this
      // `extent·TM·CVT` returns the index to the rightmost cell in the array.
      Local  Ξ(extent_t ,TM·CVT) Ξ(TM·Array ,TM·CVT)·extent(Ξ(TM·Array ,TM·CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Tape·Topo Tape·topo = Core·Tape·Topo·mu;
          Core·Status status = Ξ(TM·Array ,TM·CVT)·Tape·topo(tm ,&Tape·topo);
          bool good_Tape·topo = 
            (status == Core·Status·on_track) && (Tape·topo & Core·Tape·Topo·finite_nz)
            ;
          Core·Guard·fg.check(&chk ,1 ,good_Tape·topo ,"Tape does not have an extent.");
          Core·Guard·assert(chk);
        #endif

        *result = tm->extent;
        return Core·Status·on_track;
      }

      //-----------------------------------
      // TM·Array.area implementation 

      Local Core·Status Ξ(TM·Array ,TM·CVT)·mount_pe(
        Ξ(TM·Array ,TM·CVT) *tm ,TM·CVT *position ,Ξ(extent_t ,TM·CVT) extent
      ){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,tm ,TM·Array·Msg·tm);
          Core·Guard·fg.check(&chk ,1 ,position ,"Given NULL position.");
          Core·Guard·if_return(chk);
        #endif
        tm->position = position;
        tm->extent = extent;
        tm->hd = position; // mount the head on the origin cell
        return Core·Status·on_track;
      }

      // If size of TM·CVT is not a power of two this can perform a divide
      Local Core·Status Ξ(TM·Array ,TM·CVT)·mount_pp(
        Ξ(TM·Array ,TM·CVT) *tm ,TM·CVT *pos_leftmost ,TM·CVT *pos_rightmost
      ){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,pos_leftmost  ,"Null pos_leftmost.");
          Core·Guard·fg.check(&chk ,1 ,pos_rightmost ,"Null pos_rightmost.");
          if(pos_leftmost && pos_rightmost){
            Core·Guard·fg.check(&chk ,1 ,pos_rightmost >= pos_leftmost 
              ,"pos_rightmost < pos_leftmost"
            );
          }
          Core·Guard·if_return(chk);
        #endif

        Ξ(extent_t ,TM·CVT) extent = pos_rightmost - pos_leftmost);
        return Ξ(TM·Array ,TM·CVT)·mount_pe(tm ,pos_leftmost ,extent);
      }

      //-----------------------------------
      // base Tape Machine operations

      Local Core·Status Ξ(TM·Array ,TM·CVT)·mount(Ξ(TM·Array ,TM·CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,tm ,TM·Array·Msg·tm);
          if(tm) Core·Guard·fg.check(&chk ,1 ,tm->position ,TM·Array·Msg·position);
          Core·Guard·if_return(chk);
        #endif

        // mounting an already mounted head does nothing ,perhaps you meant `rewind`?
        if(!tm->hd) tm->hd = tm->position;
        return Core·Status·on_track;
      }

      Local Core·Status Ξ(TM·Array ,TM·CVT)·dismount(Ξ(TM·Array ,TM·CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,tm ,TM·Array·Msg·tm);
          Core·Guard·if_return(chk);
        #endif

        tm->hd = NULL;
        return Core·Status·on_track;
      }

      Local TM·Head·Status Ξ(TM·Array ,TM·CVT)·head_status(TM *tm){
        if(!tm || !tm->position) return TM·Head·Status·mu;
        if(!tm->hd) return TM·Head·Status·dismounted;
        if(tm->hd == tm->position) return TM·Head·Status·leftmost;

        TM·CVT *rightmost_pt = tm->position + tm->extent;
        if(tm->hd == rightmost_pt) TM·Head·Status·rightmost;
        if(tm->hd < tm->position || tm->hd > rightmost_pt)
          return TM·Head·Status·out_of_area;

         return TM·Head·Status·interim;
      }

      bool Ξ(TM·Array ,TM·CVT)·can_read(Ξ(TM·Array ,TM·CVT) *tm){
        return tm && tm->position && tm->hd;
      }

      // can_read was true
      bool Ξ(TM·Array ,TM·CVT)·on_origin(Ξ(TM·Array ,TM·CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          bool flag = true ,s;
          s = Ξ(TM·Array ,TM·CVT)·head_on_format(tm ,flag) == Core·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"head off format");
          Core·Guard·assert(chk);
        #endif
        return tm->hd == tm->position;
      }

      // can_read was true
      bool Ξ(TM·Array ,TM·CVT)·on_rightmost(Ξ(TM·Array ,TM·CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          bool flag = true ,s;
          s = Ξ(TM·Array ,TM·CVT)·head_on_format(tm ,flag) == Core·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"head off format");
          Core·Guard·assert(chk);
        #endif
        return tm->hd == tm->position;
      }

      void Ξ(TM·Array ,TM·CVT)·step(Ξ(TM·Array ,TM·CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          bool flag = true ,s;
          s = Ξ(TM·Array ,TM·CVT)·head_on_format(tm ,flag) == Core·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"head off format");
          Core·Guard·assert(chk);
        #endif
        tm->hd++;
      }

      void Ξ(TM·Array ,TM·CVT)·step_left(Ξ(TM·Array ,TM·CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          bool flag = true ,s;
          s = Ξ(TM·Array ,TM·CVT)·head_on_format(tm ,flag) == Core·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"head off format");
          Core·Guard·assert(chk);
        #endif
        tm->hd--;
      }

      void Ξ(TM·Array ,TM·CVT)·rewind(Ξ(TM·Array ,TM·CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          bool flag = true ,s;
          s = Ξ(TM·Array ,TM·CVT)·head_on_format(tm ,flag) == Core·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"head off format");
          Core·Guard·assert(chk);
        #endif
        tm->hd = tm->position;
      }

      // tm_can_read must be true for both machines.
      void Ξ(TM·Array ,TM·CVT)·copy_datum(Ξ(TM·Array ,TM·CVT) *tm_read ,Ξ(TM·Array ,TM·CVT) *tm_write){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          bool flag = true ,s;
          s = Ξ(TM·Array ,TM·CVT)·head_on_format(tm_read ,flag) == Control·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"tm_read head off track");
          s = Ξ(TM·Array ,TM·CVT)·head_on_format(tm_write ,flag) == Control·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"tm_write head off track");
          Core·Guard·assert(chk);
        #endif

        *(tm_write->hd) = *(tm_read->hd);
        return Core·Status·on_track;
      }

      void Ξ(TM·Array ,TM·CVT)·read(Ξ(TM·Array ,TM·CVT) *tm ,TM·CVT *read_pt){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          bool flag = true ,s;
          s = Ξ(TM·Array ,TM·CVT)·head_on_format(tm ,flag) == Core·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"head off format");
          Core·Guard·assert(chk);
        #endif

        *read_pt = *(tm->hd);
      }


      // step_right is a synonym for step


      //----------------------------------------
      // Initialization for Ξ(TM·Array ,TM·CVT)·fg

      Local Ξ(TM·Array ,TM·CVT)·Binding Ξ(TM·Array ,TM·CVT)·fg = {
        .tape = {
           .Tape·topo   = Ξ(TM·Array ,TM·CVT)·Tape·topo
           .extent = Ξ(TM·Array ,TM·CVT)·extent
        }

        ,.area = {
           .mount_pe = Ξ(TM·Array ,TM·CVT)·mount_pe
          ,.mount_pp = Ξ(TM·Array ,TM·CVT)·mount_pp
        }

        ,.mount    = Ξ(TM·Array ,TM·CVT)·mount
        ,.dismount = Ξ(TM·Array ,TM·CVT)·dismount

        ,.status         = Ξ(TM·Array ,TM·CVT)·status
        ,.head_on_format = Ξ(TM·Array ,TM·CVT)·head_on_format

        ,.can_read     = Ξ(TM·Array ,TM·CVT)·can_read
        ,.on_origin    = Ξ(TM·Array ,TM·CVT)·on_origin
        ,.on_rightmost = Ξ(TM·Array ,TM·CVT)·on_rightmost

        ,.step = Ξ(TM·Array ,TM·CVT)·step
        ,.step_left = Ξ(TM·Array ,TM·CVT)·step_left
        ,.step_right = Ξ(TM·Array ,TM·CVT)·step_right // Synonym for step
        ,.rewind = Ξ(TM·Array ,TM·CVT)·rewind

        ,.copy_datum = Ξ(TM·Array ,TM·CVT)·copy_datum
        ,.read = Ξ(TM·Array ,TM·CVT)·read
        ,.write = Ξ(TM·Array ,TM·CVT)·write

      };

    #endif // ifdef TM·CVT

  #endif // LOCAL

#endif // IMPLEMENTATION


void Ξ(TM·Array ,TM·CVT)·write(Ξ(TM·Array ,TM·CVT) *tm ,TM·CVT *write_pt){
  #ifdef TM·DEBUG
    Core·Guard·init_count(chk);
    bool flag = true ,s;
    s = Ξ(TM·Array ,TM·CVT)·head_on_format(tm ,flag) == Core·Status·on_track;
    Core·Guard·fg.check(&chk ,1 ,s && flag ,"head off format");
    Core·Guard·assert(chk);
  #endif

  *(tm->hd) = *write_pt;
}

      //-----------------------------------
      // TM struct initializers

      Local Core·Status TM·mount_pe(
        TM *tm ,TM·CVT *position ,Ξ(extent_t ,TM·CVT) extent
      ){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,tm ,TM·Msg·tm);
          Core·Guard·fg.check(&chk ,1 ,position ,"Given NULL position.");
          Core·Guard·if_return(chk);
        #endif
        tm->position = position;
        tm->extent = extent;
        tm->hd = position; // mount the head on the origin cell
        return Core·Status·on_track;
      }

      // If size of TM·CVT is not a power of two this can perform a divide
      Local Core·Status TM·mount_pp(
        TM *tm ,TM·CVT *pos_leftmost ,TM·CVT *pos_rightmost
      ){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,pos_leftmost  ,"Null pos_leftmost.");
          Core·Guard·fg.check(&chk ,1 ,pos_rightmost ,"Null pos_rightmost.");
          if(pos_leftmost && pos_rightmost){
            Core·Guard·fg.check(&chk ,1 ,pos_rightmost >= pos_leftmost 
              ,"pos_rightmost < pos_leftmost"
            );
          }
          Core·Guard·if_return(chk);
        #endif

        Ξ(extent_t ,TM·CVT) extent = pos_rightmost - pos_leftmost);
        return TM·mount_pe(tm ,pos_leftmost ,extent);
      }

