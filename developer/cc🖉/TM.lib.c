/*
  TM - Tape Machine

  This file has the template parameter: `CVT`

  `CVT` Cell Value Type`. It is the type of the datum placed in a cell. By default (when the   macro CVT has no definition) the cell value type is taken as AUM. This file must
  be included with CVT undefined ,before inclusions with it defined.
 
  'Tape' is operated on by the Tape Machine.

  'Area' is subset of an address space that is used as a virtual Tape by a machine.

   An Area with zero elements has 'length == 0' or is 'empty'. In contrast ,and
   area located (position specified) with a null pointer is said not to exist.

   ----

   CVT is a template variable that affects type names that show on the interface.
   They affect all implementations in the same way.

   There are multiple implementations, such as Array. Each has its own FG
   table instance. For example Array·fg. Also each has its own Tableau type, for
   example, `typedef struct { ... } Ξ(TM·Array ,CVT)` is the Tableau type for
   the array implementation.

   This implementation uses pointer pairs to stand for 'TM type' that are
   passed around. 

*/

#define TM·DEBUG
#ifdef TM·DEBUG
  #include <stdio.h>
#endif

#define TM·ALL  ( defined(CVT) ) // ... && defined( ...
#define TM·NONE ( !defined(CVT) )
#if !( TM·ALL || TM·NONE )
  #error "TM template inconsistency: must define all or none of: CVT"
#endif

#ifndef FACE
#define TM·IMPLEMENTATION
#define FACE
#endif 

//--------------------------------------------------------------------------------
// Interface - always included

#if TM·ALL
  #define TM·ALL_VAL = CVT 
  #define TM_t Ξ(TM ,TM·ALL_VAL)
#endif

#ifndef Ξ(TM ,FACE)
#define Ξ(TM ,FACE)

 
  #include <stdint.h>
  #include <stddef.h>

  #include "Core.lib.c"
  #include "FG.lib.c"

  //----------------------------------------
  // Tape Machine interface
  //----------------------------------------

  #if TM·NONE
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

    // If tape machine does not support step left ,then Status·leftmost 
    // will be reported as Status·interim.
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

  #endif // #ifndef CVT

  #if TM·All

    // TM·<CVT>·Tableau
    typedef struct Ξ(TM ,Tableau);
    
    // create an FG·Binding so that FG·call will work
    #define FG·Type TM 
    #include "FG.lib.c"
    #undef FG·Type

    // extent is an index, hence its effect is a function of CVT
    typedef Ξ(extent_t ,CVT) size_t;
                             
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

      TM·FG TM·fg; // points to TM·FG instance
      Ξ(extent_t ,CVT) (*extent)(TM tm);
      CVT  (*read) (TM tm);
      void (*write)(TM tm ,CVT *remote_pt);

    } Ξ(TM ,FG);

    //----------------------------------------
    // Array interface
    //----------------------------------------

    typedef struct Ξ(TM ,Array)·Tableau;
                                  
    TM Ξ(TM ,Array)·init_pe( 
       Ξ(TM ,Array)·Tableau *t
      ,CVT position[] 
      ,Ξ(extent_t ,CVT) extent 
    );

    TM Ξ(TM ,Array)·init_pp( 
       Ξ(TM ,Array)·Tableau *t
      ,CVT *position_left 
      ,CVT *position_right 
    );

  #endif // #ifdef CVT

#endif // FACE

//--------------------------------------------------------------------------------
// Implementation

#ifdef TM·IMPLEMENTATION

  // implementation to go into the lib.a file
  //
    #ifndef LOCAL
    #endif 

  #ifdef LOCAL

    //----------------------------------------
    // Dispatch wrapper
    //----------------------------------------

    #ifndef CVT

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

      Local TM·FG TM·fg = {
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

    #endif // ifndef CVT

    //-----------------------------------
    // CVT dependent functions

    #ifdef CVT

      Local Ξ(extent_t ,CVT) Ξ(TM ,CVT)·extent(TM *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,TM·Tape·bounded(tm) ,"Tape is not bounded.");
          Core·Guard·assert(chk);
        #endif
        return tm->fg.extent(tm);
      }

      Local CVT TM·read(TM *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check( &chk ,1 ,TM·head_on_tape(tm) ,TM·Msg·head);
          Core·Guard·assert(chk);
        #endif
        return tm->fg.read(tm);
      }

      Local void TM·write(TM *tm ,CVT *write_pt){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk); 
          Core·Guard·fg.check( &chk ,1 ,TM·head_on_tape(tm) ,TM·Msg·head); 
          Core·Guard·fg.check( &chk ,1 ,write_pt ,"Given NULL write_pt"); 
          Core·Guard·assert(chk);
        #endif
        return tm->fg.write(tm ,write_pt);
      }

      Local Ξ(TM ,CVT)·FG Ξ(TM ,CVT)·fg = {
        .parent = TM·fg
        ,.extent = Ξ(TM ,CVT)·extent
        ,.read = Ξ(TM ,CVT)·read
        ,.write = Ξ(TM ,CVT)·write
      };

    #endif // ifdef CVT


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

    #ifndef CVT

      //-----------------------------------
      // common error messages

      const char *TM·Array·Msg·tm="given NULL tm";
      const char *TM·Array·Msg·flag="given NULL flag pointer";
      const char *TM·Array·Msg·result="given NULL result pointer";
      const char *TM·Array·Msg·status="bad head status";
        

    #endif // #ifndef CVT

    #ifdef CVT

      typedef struct Ξ(TM ,CVT)·Tableau;

      typedef struct{
        CVT *hd;
        CVT *position;
        Ξ(extent_t ,CVT) extent;
      }Ξ(TM·Array ,CVT)·Tableau;

      //-----------------------------------

      // TM·Array.tape implementation

      //----------------------------------------
      // TM Array implementation, not CVT differentiated

      Local TM·Tape·Topo TM·Array·Tape·topo( Ξ(TM ,CVT)·Tableau *tableau ){
        Ξ(TM·Array ,CVT)·Tableau *t = (Ξ(TM·Array ,CVT)·Tableau *)tableau;
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
       TM·Tape·Topo  Ξ(TM·Array ,CVT)·Tape·topo(Ξ(TM·Array ,CVT) *tm){
         if(!tm || !tm->position) return TM·Tape·Topo·mu;
         if(tm->extent == 0) TM·Tape·Topo·singleton; 
         return TM·Tape·Topo·segment;
      }

      // check the Tape·topo to make sure tape has extent before calling this
      // `extent·CVT` returns the index to the rightmost cell in the array.
      Local  Ξ(extent_t ,CVT) Ξ(TM·Array ,CVT)·extent(Ξ(TM·Array ,CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Tape·Topo Tape·topo = Core·Tape·Topo·mu;
          Core·Status status = Ξ(TM·Array ,CVT)·Tape·topo(tm ,&Tape·topo);
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

      Local Core·Status Ξ(TM·Array ,CVT)·mount_pe(
        Ξ(TM·Array ,CVT) *tm ,CVT *position ,Ξ(extent_t ,CVT) extent
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

      // If size of CVT is not a power of two this can perform a divide
      Local Core·Status Ξ(TM·Array ,CVT)·mount_pp(
        Ξ(TM·Array ,CVT) *tm ,CVT *pos_leftmost ,CVT *pos_rightmost
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

        Ξ(extent_t ,CVT) extent = pos_rightmost - pos_leftmost);
        return Ξ(TM·Array ,CVT)·mount_pe(tm ,pos_leftmost ,extent);
      }

      //-----------------------------------
      // base Tape Machine operations

      Local Core·Status Ξ(TM·Array ,CVT)·mount(Ξ(TM·Array ,CVT) *tm){
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

      Local Core·Status Ξ(TM·Array ,CVT)·dismount(Ξ(TM·Array ,CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,tm ,TM·Array·Msg·tm);
          Core·Guard·if_return(chk);
        #endif

        tm->hd = NULL;
        return Core·Status·on_track;
      }

      Local TM·Head·Status Ξ(TM·Array ,CVT)·head_status(TM *tm){
        if(!tm || !tm->position) return TM·Head·Status·mu;
        if(!tm->hd) return TM·Head·Status·dismounted;
        if(tm->hd == tm->position) return TM·Head·Status·leftmost;

        CVT *rightmost_pt = tm->position + tm->extent;
        if(tm->hd == rightmost_pt) TM·Head·Status·rightmost;
        if(tm->hd < tm->position || tm->hd > rightmost_pt)
          return TM·Head·Status·out_of_area;

         return TM·Head·Status·interim;
      }

      bool Ξ(TM·Array ,CVT)·can_read(Ξ(TM·Array ,CVT) *tm){
        return tm && tm->position && tm->hd;
      }

      // can_read was true
      bool Ξ(TM·Array ,CVT)·on_origin(Ξ(TM·Array ,CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          bool flag = true ,s;
          s = Ξ(TM·Array ,CVT)·head_on_format(tm ,flag) == Core·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"head off format");
          Core·Guard·assert(chk);
        #endif
        return tm->hd == tm->position;
      }

      // can_read was true
      bool Ξ(TM·Array ,CVT)·on_rightmost(Ξ(TM·Array ,CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          bool flag = true ,s;
          s = Ξ(TM·Array ,CVT)·head_on_format(tm ,flag) == Core·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"head off format");
          Core·Guard·assert(chk);
        #endif
        return tm->hd == tm->position;
      }

      void Ξ(TM·Array ,CVT)·step(Ξ(TM·Array ,CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          bool flag = true ,s;
          s = Ξ(TM·Array ,CVT)·head_on_format(tm ,flag) == Core·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"head off format");
          Core·Guard·assert(chk);
        #endif
        tm->hd++;
      }

      void Ξ(TM·Array ,CVT)·step_left(Ξ(TM·Array ,CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          bool flag = true ,s;
          s = Ξ(TM·Array ,CVT)·head_on_format(tm ,flag) == Core·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"head off format");
          Core·Guard·assert(chk);
        #endif
        tm->hd--;
      }

      void Ξ(TM·Array ,CVT)·rewind(Ξ(TM·Array ,CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          bool flag = true ,s;
          s = Ξ(TM·Array ,CVT)·head_on_format(tm ,flag) == Core·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"head off format");
          Core·Guard·assert(chk);
        #endif
        tm->hd = tm->position;
      }

      // tm_can_read must be true for both machines.
      void Ξ(TM·Array ,CVT)·copy_datum(Ξ(TM·Array ,CVT) *tm_read ,Ξ(TM·Array ,CVT) *tm_write){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          bool flag = true ,s;
          s = Ξ(TM·Array ,CVT)·head_on_format(tm_read ,flag) == Control·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"tm_read head off track");
          s = Ξ(TM·Array ,CVT)·head_on_format(tm_write ,flag) == Control·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"tm_write head off track");
          Core·Guard·assert(chk);
        #endif

        *(tm_write->hd) = *(tm_read->hd);
        return Core·Status·on_track;
      }

      void Ξ(TM·Array ,CVT)·read(Ξ(TM·Array ,CVT) *tm ,CVT *read_pt){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          bool flag = true ,s;
          s = Ξ(TM·Array ,CVT)·head_on_format(tm ,flag) == Core·Status·on_track;
          Core·Guard·fg.check(&chk ,1 ,s && flag ,"head off format");
          Core·Guard·assert(chk);
        #endif

        *read_pt = *(tm->hd);
      }


      // step_right is a synonym for step


      //----------------------------------------
      // Initialization for Ξ(TM·Array ,CVT)·fg

      Local Ξ(TM·Array ,CVT)·FG Ξ(TM·Array ,CVT)·fg = {
        .tape = {
           .Tape·topo   = Ξ(TM·Array ,CVT)·Tape·topo
           .extent = Ξ(TM·Array ,CVT)·extent
        }

        ,.area = {
           .mount_pe = Ξ(TM·Array ,CVT)·mount_pe
          ,.mount_pp = Ξ(TM·Array ,CVT)·mount_pp
        }

        ,.mount    = Ξ(TM·Array ,CVT)·mount
        ,.dismount = Ξ(TM·Array ,CVT)·dismount

        ,.status         = Ξ(TM·Array ,CVT)·status
        ,.head_on_format = Ξ(TM·Array ,CVT)·head_on_format

        ,.can_read     = Ξ(TM·Array ,CVT)·can_read
        ,.on_origin    = Ξ(TM·Array ,CVT)·on_origin
        ,.on_rightmost = Ξ(TM·Array ,CVT)·on_rightmost

        ,.step = Ξ(TM·Array ,CVT)·step
        ,.step_left = Ξ(TM·Array ,CVT)·step_left
        ,.step_right = Ξ(TM·Array ,CVT)·step_right // Synonym for step
        ,.rewind = Ξ(TM·Array ,CVT)·rewind

        ,.copy_datum = Ξ(TM·Array ,CVT)·copy_datum
        ,.read = Ξ(TM·Array ,CVT)·read
        ,.write = Ξ(TM·Array ,CVT)·write

      };

    #endif // ifdef CVT

  #endif // LOCAL

#endif // IMPLEMENTATION


void Ξ(TM·Array ,CVT)·write(Ξ(TM·Array ,CVT) *tm ,CVT *write_pt){
  #ifdef TM·DEBUG
    Core·Guard·init_count(chk);
    bool flag = true ,s;
    s = Ξ(TM·Array ,CVT)·head_on_format(tm ,flag) == Core·Status·on_track;
    Core·Guard·fg.check(&chk ,1 ,s && flag ,"head off format");
    Core·Guard·assert(chk);
  #endif

  *(tm->hd) = *write_pt;
}

      //-----------------------------------
      // TM struct initializers

      Local Core·Status TM·mount_pe(
        TM *tm ,CVT *position ,Ξ(extent_t ,CVT) extent
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

      // If size of CVT is not a power of two this can perform a divide
      Local Core·Status TM·mount_pp(
        TM *tm ,CVT *pos_leftmost ,CVT *pos_rightmost
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

        Ξ(extent_t ,CVT) extent = pos_rightmost - pos_leftmost);
        return TM·mount_pe(tm ,pos_leftmost ,extent);
      }

