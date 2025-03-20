/*
  TM - Tape Machine

  This file has the template parameter: `CVT`

  `CVT` Cell Value Type`. It is the type of the datum placed in a cell. By default (when the   macro CVT has no definition) the cell value type is taken as AUM. This file must
  be included with CVT undefined ,before inclusions with it defined.
 
  'Tape' is operated on by the Tape Machine.

  'Area' is subset of an address space that is used as a virtual Tape by a machine.

   An Area with zero elements has 'length == 0' or is 'empty'. In contrast ,and
   area located (position specified) with a null pointer is said not to exist.

   Template variable: CVT

*/

#define TM·DEBUG

#ifndef FACE
#define TM·IMPLEMENTATION
#define FACE
#endif 

#if defined(Array) && !defined(CVT)
  #error "Array implementation requires CVT to be defined"
#endif


//--------------------------------------------------------------------------------
// Interface

#ifndef Ξ(TM·FACE ,CVT)
#define Ξ(TM·FACE ,CVT)

  #include <stdint.h>
  #include <stddef.h>

  #include "Core.lib.c"

  //----------------------------------------
  // Tape Machine interface
  //----------------------------------------

  #ifndef CVT
    typedef enum{
       TM·Tape·Topo·mu = 0 
      ,TM·Tape·Topo·nonexistent = 1 
      ,TM·Tape·Topo·empty       = 1 << 1
      ,TM·Tape·Topo·singleton   = 1 << 2
      ,TM·Tape·Topo·segment     = 1 << 3
      ,TM·Tape·Topo·circle      = 1 << 4
      ,TM·Tape·Topo·tail_cyclic = 1 << 5
      ,TM·Tape·Topo·infinite    = 1 << 6
    }TM·Tape·Topo;

    const TM·Tape·Topo TM·Tape·Topo·finite_nz = 
      TM·Tape·Topo·singleton | TM·Tape·Topo·segment
      ;

    // If tape machine does not support step left ,then Status·leftmost 
    // will be reported as Status·interim.
    typedef enum{
       TM·Head·Status·mu = 0
      ,TM·Head·Status·not_on_tape = 1
      ,TM·Head·Status·origin    = 1 << 1
      ,TM·Head·Status·interim   = 1 << 2
      ,TM·Head·Status·rightmost = 1 << 3
    } TM·Head·Status;

    const TM·Head·Status TM·Head·Status·on_tape = 
      TM·Head·Status·origin
      | TM·Head·Status·interim
      | TM·Head·Status·rightmost
      ;

    typedef struct TM;

    // tape and area are included with Tape Machine to facilitate abstract interfaces.
    typedef struct{

      struct {
        Core·Status      (*topo)  ( TM *tm ,TM·Tape·Topo *result );
        Ξ(extent_t ,CVT) (*extent)( TM *tm );
      } tape;

      struct {
        // Initialize tm
        Core·Status (*mount_pe)(TM *tm ,CVT position[] ,Ξ(extent_t ,CVT) extent);
        Core·Status (*mount_pp)(TM *tm ,CVT *position_left ,CVT *position_right);
      } area;

      // tape machine functions
      Core·Status (*mount)   (TM *tm);
      Core·Status (*dismount)(TM *tm);

      TM·Head·Status (*status)        (TM *tm ,TM·Head·Status *status);
      Core·Status    (*head_on_format)(TM *tm ,bool *flag);

      bool (*can_read)    (TM *tm);
      bool (*on_leftmost) (TM *tm);
      bool (*on_rightmost)(TM *tm);

      void (*step)      (TM *tm);
      void (*step_left) (TM *tm);
      void (*step_right)(TM *tm); // Synonym for step
      void (*rewind)    (TM *tm);

      void (*copy_datum)( TM *tm_read ,TM *tm_write );
      void (*apply)( TM *tm_read , TM·FN);

    } TM·FG;


  #endif // #ifndef CVT

  #ifdef CVT


    typedef struct{

      void (*read)      ( TM *tm ,CVT *remote_pt );
      void (*write)     ( TM *tm ,CVT *remote_pt );

    } Ξ(TM ,CVT);

    // default table for TM implemented by an Array
    #ifdef Array
      Ξ(TM ,CVT)·FG Ξ(TM·Array ,CVT)·fg;
    #endif

  #endif // #ifdef CVT

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef TM·IMPLEMENTATION
  // declarations available to all of the IMPLEMENTATION go here
  //
    #ifdef TM·DEBUG
      #include <stdio.h>
    #endif

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

      const char *TM·Array·Msg·tm="given NULL tm";
      const char *TM·Array·Msg·flag="given NULL flag pointer";
      const char *TM·Array·Msg·result="given NULL result pointer";
      const char *TM·Array·Msg·position=
        "Null position.This is only possible when the tape machine has not been initialized.";

    #endif // #ifndef CVT

    #ifdef CVT

      struct{
        Ξ(TM ,CVT)·FG *fg
      }Ξ(TM ,CVT);

      //-----------------------------------
      // TM·Array.tape implementation

      /*
        For an Array Tape Machine ,a bound tape will be singleton or segment.
        An initialized Array Tape Machine always has a bound tape.
      */
      Core·Status Ξ(TM ,CVT)·topo(Ξ(TM·Array ,CVT) *tm ,TM·Tape·Topo *result){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,tm ,TM·Array·Msg·tm);
          Core·Guard·fg.check(&chk ,1 ,result ,TM·Array·Msg·result);
          Core·Guard·if_return(chk);
        #endif
        if(tm->extent == 0){
          *result = TM·Tape·Topo·singleton; 
        }else{
          *result = TM·Tape·Topo·segment;
        }
        return Core·Status·on_track;
      }

      // check the topo to make sure tape has extent before calling this
      // `extent·CVT` returns the index to the rightmost cell in the array.
      Local  Ξ(extent_t ,CVT) Ξ(TM·Array ,CVT)·extent(Ξ(TM·Array ,CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Tape·Topo topo = Core·Tape·Topo·mu;
          Core·Status status = Ξ(TM·Array ,CVT)·topo(tm ,&topo);
          bool good_topo = 
            (status == Core·Status·on_track) && (topo & Core·Tape·Topo·finite_nz)
            ;
          Core·Guard·fg.check(&chk ,1 ,good_topo ,"Tape does not have an extent.");
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

      Local TM·Head·Status Ξ(TM·Array ,CVT)·status(
        Ξ(TM·Array ,CVT) *tm ,TM·Head·Status *status
      ){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,tm ,TM·Array·Msg·tm);
          Core·Guard·fg.check(&chk ,1 ,status ,"given NULL status pointer");
          Core·Guard·if_return(chk);
        #endif

        if(tm->hd == NULL){
          *status = TM·Head·Status·not_on_tape;
        }else if(tm->hd == tm->position){
          *status = TM·Head·Status·origin;
        }else if(tm->hd == tm->position + tm->extent){
          *status = TM·Head·Status·rightmost;
        }else{
          *status = TM·Head·Status·interim;
        }

        return Core·Status·on_track;
      }

     // Stronger than `can_read`. Used mostly for debugging.
     // as it checks for a legal head position.
      Local Core·Status Ξ(TM·Array ,CVT)·head_on_format(
        Ξ(TM·Array ,CVT) *tm ,bool *flag
      ){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,tm ,TM·Array·Msg·tm);
          if(tm) Core·Guard·fg.check(&chk ,1 ,tm->position ,TM·Array·Msg·position);
          Core·Guard·fg.check(&chk ,1 ,flag ,TM·Array·Msg·flag);
          Core·Guard·if_return(chk);
        #endif

        *flag = 
             tm->hd
          && tm->hd >= tm->position
          && tm->hd - tm->position <= tm->extent
          && ( (AU *)tm->hd - (AU *)tm->position ) % sizeof(CVT) == 0 // '%' expensive
          ;
        return Core·Status·on_track;
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

      // step_right is a synonym for step


      //----------------------------------------
      // Initialization for Ξ(TM·Array ,CVT)·fg

      Local Ξ(TM·Array ,CVT)·FG Ξ(TM·Array ,CVT)·fg = {
        .tape = {
           .topo   = Ξ(TM·Array ,CVT)·topo
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
      const char *TM·Array·Msg·position=
        "Null position.This is only possible when the tape machine has not been initialized.";

    #endif // #ifndef CVT

    #ifdef CVT

      struct{
        CVT *hd;
        CVT *position;
        Ξ(extent_t ,CVT) extent;
      }Ξ(TM·Array ,CVT);

      //-----------------------------------
      // TM·Array.tape implementation

      /*
        For an Array Tape Machine ,a bound tape will be singleton or segment.
        An initialized Array Tape Machine always has a bound tape.
      */
      Core·Status Ξ(TM·Array ,CVT)·topo(Ξ(TM·Array ,CVT) *tm ,TM·Tape·Topo *result){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,tm ,TM·Array·Msg·tm);
          if(tm) Core·Guard·fg.check(&chk ,1 ,tm->position ,TM·Array·Msg·position);
          Core·Guard·fg.check(&chk ,1 ,result ,TM·Array·Msg·result);
          Core·Guard·if_return(chk);
        #endif
        if(tm->extent == 0){
          *result = TM·Tape·Topo·singleton; 
        }else{
          *result = TM·Tape·Topo·segment;
        }
        return Core·Status·on_track;
      }

      // check the topo to make sure tape has extent before calling this
      // `extent·CVT` returns the index to the rightmost cell in the array.
      Local  Ξ(extent_t ,CVT) Ξ(TM·Array ,CVT)·extent(Ξ(TM·Array ,CVT) *tm){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Tape·Topo topo = Core·Tape·Topo·mu;
          Core·Status status = Ξ(TM·Array ,CVT)·topo(tm ,&topo);
          bool good_topo = 
            (status == Core·Status·on_track) && (topo & Core·Tape·Topo·finite_nz)
            ;
          Core·Guard·fg.check(&chk ,1 ,good_topo ,"Tape does not have an extent.");
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

      Local TM·Head·Status Ξ(TM·Array ,CVT)·status(
        Ξ(TM·Array ,CVT) *tm ,TM·Head·Status *status
      ){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,tm ,TM·Array·Msg·tm);
          Core·Guard·fg.check(&chk ,1 ,status ,"given NULL status pointer");
          Core·Guard·if_return(chk);
        #endif

        if(tm->hd == NULL){
          *status = TM·Head·Status·not_on_tape;
        }else if(tm->hd == tm->position){
          *status = TM·Head·Status·origin;
        }else if(tm->hd == tm->position + tm->extent){
          *status = TM·Head·Status·rightmost;
        }else{
          *status = TM·Head·Status·interim;
        }

        return Core·Status·on_track;
      }

     // Stronger than `can_read`. Used mostly for debugging.
     // as it checks for a legal head position.
      Local Core·Status Ξ(TM·Array ,CVT)·head_on_format(
        Ξ(TM·Array ,CVT) *tm ,bool *flag
      ){
        #ifdef TM·DEBUG
          Core·Guard·init_count(chk);
          Core·Guard·fg.check(&chk ,1 ,tm ,TM·Array·Msg·tm);
          if(tm) Core·Guard·fg.check(&chk ,1 ,tm->position ,TM·Array·Msg·position);
          Core·Guard·fg.check(&chk ,1 ,flag ,TM·Array·Msg·flag);
          Core·Guard·if_return(chk);
        #endif

        *flag = 
             tm->hd
          && tm->hd >= tm->position
          && tm->hd - tm->position <= tm->extent
          && ( (AU *)tm->hd - (AU *)tm->position ) % sizeof(CVT) == 0 // '%' expensive
          ;
        return Core·Status·on_track;
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

      // step_right is a synonym for step


      //----------------------------------------
      // Initialization for Ξ(TM·Array ,CVT)·fg

      Local Ξ(TM·Array ,CVT)·FG Ξ(TM·Array ,CVT)·fg = {
        .tape = {
           .topo   = Ξ(TM·Array ,CVT)·topo
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
