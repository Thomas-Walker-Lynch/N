 /*
  Core - core memory operations.

  'ATP'  Used in comments. 'At This Point' in the code. 

  'Tape' refers to an tape (in contrast to a point) in the address space.

   A non-exist array is said to be non-existent.
   An array with zero elements has 'length == 0' or is 'empty'.

   In contrast, an allocation does not exist if it has zero bytes.

   It is better to separate the tableau than have a union, and let the
   optimizer figure out the memory life times, and what can be reused.

   Nodes include neighbor links, that makes traversal more efficient.

   FG FunctionsGiven<type>, e.g. FunctionsGiven<Core·Area>, abbreviate as FG, or fg.
   Nice abbreviation as f and g are common one letter abreviaions for a functions.

   U - short for Utility
  
*/

#define Core·DEBUG

#ifndef FACE
#define Core·IMPLEMENTATION
#define FAC
#endif 

//--------------------------------------------------------------------------------
// Interface

#ifndef Core·FACE
#define Core·FACE

  #include <stdint.h>
  #include <stddef.h>

  //----------------------------------------
  // memory

    #define extentof(x)(sizeof(x) - 1)
    #define extent_t size_t

    // AU == Addressable Unit
    // given an AU is an 8 bit byte, 4AU is 32 bits, and 8 AU is 64 bits.
    #define AU uint8_t

  //----------------------------------------
  // model

    // no state, this is merely a namespace

    typedef enum{
       Core·Status·mu = 0
      ,Core·Status·on_track
      ,Core·Status·derailed
    }Core·Status;

    typedef struct{
      Core·Status (*on_track)();
      Core·Status (*derailed)();

      void *(*offset)(void *p ,size_t Δ);
      void *(*offset_8AU)(void *p ,size_t Δ);

      bool (*is_aligned_on_8AU)(void *p);
      void *(*floor_within_aligned_8AU)(void *p);
      void *(*ceiling_within_aligned_8AU)(void *p);

    } Core·U;

    Local Core·U Core·u;


  //----------------------------------------
  // Tape Machine

    typedef struct Core·TM_NX;
    typedef struct Core·TM_NX·Address;

    typedef enum{
       Core·Tape·Topo·mu
      ,Core·Tape·Topo·nonexistent // pointer to tape is NULL
      ,Core·Tape·Topo·empty      // tape has no cells
      ,Core·Tape·Topo·singleton  // extent is zero
      ,Core·Tape·Topo·segment    // finite non-singleton tape
      ,Core·Tape·Topo·circle     // initial location recurs
      ,Core·Tape·Topo·tail_cyclic  // other than initial location recurs
      ,Core·Tape·Topo·infinite   // exists, not empty, no cycle, no rightmost
    }Core·Tape·Topo;

    // If tape machine does not support step left, then Status·leftmost 
    // will be reported as Status·interim.
    typedef enum{
       Core·TM·Head·Status·mu = 0
      ,Core·TM·Head·Status·not_on_tape = 1
      ,Core·TM·Head·Status·origin
      ,Core·TM·Head·Status·interim 
      ,Core·TM·Head·Status·rightmost
    } Core·TM·Head·Status;

    typedef struct{

      Core·Status (*mount)(Core·TM_NX *tm);
      Core·Status (*dismount)(Core·TM_NX *tm);
      Core·Status (*rewind)(Core·TM_NX *tm);

      Core·Status (*can_step)(Core·TM_NX *tm ,bool *flag);
      Core·Status (*on_origin)(Core·TM_NX *tm ,bool *flag);
      Core·Status (*on_rightmost)(Core·TM_NX *tm ,bool *flag);

      Core·Status (*extent)(Core·TM_NX *tm ,extent_t *extent_pt);
      Core·Status (*can_read)(Core·TM_NX *tm ,bool *flag);

      Core·Status (*step)(Core·TM_NX *tm);
      Core·Status (*step_left)(Core·TM_NX *tm);
      Core·Status (*step_right)(Core·TM_NX *tm); // Synonym for step
      Core·TM·Head·Status (*status)(Core·TM_NX *tm);

      struct {
        Core·Tape·Topo (*topo)(Core·TM_NX *tm);
        Core·Status (*extent)(Core·TM_NX *tm ,extent_t *write_pt);
        Core·Status (*is_origin)(Core·TM_NX *tm ,Core·TM_NX·Address *a ,bool *result);
        Core·Status (*is_rightmost)(Core·TM_NX *tm ,Core·TM_NX·Address *a ,bool *result);
        Core·Status (*read)(Core·TM_NX *tm ,Core·TM_NX·Address *a ,Core·Tape·Remote *remote);
        Core·Status (*write)(Core·TM_NX *tm ,Core·TM_NX·Address *a ,Core·Tape·Remote *remote);

        // Initialize TM area
        Core·Status (*init_pe)(Core·TM_NX *tm ,void *position ,extent_t extent);
        Core·Status (*init_pp)(Core·TM_NX *tm ,void *position_left ,void *position_right);
        Core·Status (*set_position)(Core·TM_NX *tm ,Core·TM_NX·Address *new_position);
        Core·Status (*set_position_left)(Core·TM_NX *tm ,Core·TM_NX·Address *new_position); // synonym
        Core·Status (*set_position_right)(Core·TM_NX *tm ,Core·TM_NX·Address *new_position_right);
        Core·Status (*set_extent)(Core·TM_NX *tm ,extent_t extent); 

        // Read TM properties
        Core·TM_NX·Address *(*position)(Core·TM_NX *tm);
        Core·TM_NX·Address *(*position_left)(Core·TM_NX *tm); // synonym
        Core·TM_NX·Address *(*position_right)(Core·TM_NX *tm);

        Core·TM_NX·Address *(*complement)(Core·TM_NX *tm ,Core·TM_NX·Address *r);

        // TM relationships
        bool (*encloses_pt)(Core·TM_NX *tm ,Core·TM_NX·Address *pt);
        bool (*encloses_pt_strictly)(Core·TM_NX *tm ,Core·TM_NX·Address *pt);
        bool (*encloses_tm)(Core·TM_NX *outer ,Core·TM_NX *inner);
        bool (*encloses_tm_strictly)(Core·TM_NX *outer ,Core·TM_NX *inner);
        bool (*overlap)(Core·TM_NX *a ,Core·TM_NX *b);
        void (*largest_aligned_64)(Core·TM_NX *outer ,Core·TM_NX *inner_64);
      } area;

    } Core·TM_NX·FG;

  //----------------------------------------
  // Tape Machine

    typedef struct Core·TM_NX;
    typedef struct Core·TM_NX·Address;

    typedef enum{
       Core·Tape·Topo·mu
      ,Core·Tape·Topo·nonexistent // pointer to tape is NULL
      ,Core·Tape·Topo·empty      // tape has no cells
      ,Core·Tape·Topo·singleton  // extent is zero
      ,Core·Tape·Topo·segment    // finite non-singleton tape
      ,Core·Tape·Topo·circle     // initial location recurs
      ,Core·Tape·Topo·tail_cyclic  // other than initial location recurs
      ,Core·Tape·Topo·infinite   // exists, not empty, no cycle, no rightmost
    }Core·Tape·Topo;

    // If tape machine does not support step left, then Status·leftmost 
    // will be reported as Status·interim.
    typedef enum{
       Core·TM·Head·Status·mu = 0
      ,Core·TM·Head·Status·not_on_tape = 1
      ,Core·TM·Head·Status·origin
      ,Core·TM·Head·Status·interim 
      ,Core·TM·Head·Status·rightmost
    } Core·TM·Head·Status;

    typedef struct{

      Core·Status mount(Core·TM_NX *tm);
      Core·Status dismount(Core·TM_NX *tm);
      Core·Status rewind(Core·TM_NX *tm);

      Core·Tape·Extent·Status (*can_step)(Core·TM_NX *tm ,bool *flag);
      Core·Tape·Extent·Status (*on_origin)(Core·TM_NX *tm ,bool *flag);
      Core·Tape·Extent·Status (*on_rightmost)(Core·TM_NX *tm ,bool *flag);

      Core·Tape·Extent·Status (*extent)(Core·TM_NX *tm ,extent_t *extent_pt);
      Core·Status can_read(bool *flag);
      Core·Status can_step(bool *flag);

      Core·Status step(Core·TM_NX *tm);
      Core·Status step_left(Core·TM_NX *tm);
      Core·Status step_right(Core·TM_NX *tm); // Synonym for step
      Core·TM·Head·Status status(Core·TM_NX *tm);

      struct{
        Core·Tape·Topo (*topo)(Core·TM_NX *tm);
        Core·Tape·Extent·Status (*extent)(Core·TM_NX *tm ,extent_t *write_pt);
        Core·Status (*is_leftmot)(Core·Tape *tape ,Core·Tape·Address *a ,bool *result);
        Core·Status (*is_rightmost)(Core·Tape *tape ,Core·Tape·Address *a ,bool *result);
        Core·Status (*read)(Core·TM_NX *tm ,Core·Tape·Address *a ,Core·Tape·Remote *remote);
        Core·Status (*write)(Core·TM_NX *tm ,Core·Tape·Address *a ,Core·Tape·Remote *remote);

        // initialize tm area
        Core·Status (*init_pe)(Core·TM_NX *tm ,void *position ,extent_t extent);
        Core·Status (*init_pp)(Core·TM_NX *tm ,void *position_left ,void *position_right);
        Core·Status (*set_position)(Core·TM_NX *tm ,Core·TM_NX·Address *new_position);
        Core·Status (*set_position_left)(Core·TM_NX *tm ,Core·TM_NX·Address *new_position); // synonym
        Core·Status (*set_position_right)(Core·TM_NX *tm ,Core·TM_NX·Address *new_position_right);
        Core·Status (*set_extent)(Core·TM_NX *tm ,extent_t extent); 

        // read tm properties
        Core·TM_NX·Address *(*position)(Core·TM_NX *tm);
        Core·TM_NX·Address *(*position_left)(Core·TM_NX *tm); // synonym
        Core·TM_NX·Address *(*position_right)(Core·TM_NX *tm);

        Core·TM_NX·Address *(*complement)(Core·TM_NX *tm ,Core·TM_NX·Address *r);

        // tm relationships
        bool (*encloses_pt)(Core·TM_NX *tm ,Core·TM_NX·Address *pt);
        bool (*encloses_pt_strictly)(Core·TM_NX *tm ,Core·TM_NX·Address *pt);
        bool (*encloses_tm)(Core·TM_NX *outer ,Core·TM_NX *inner);
        bool (*encloses_tm_strictly)(Core·TM_NX *outer ,Core·TM_NX *inner);
        bool (*overlap)(Core·TM_NX *a ,Core·TM_NX *b);
        void (*largest_aligned_64)(Core·TM_NX *outer ,Core·TM_NX *inner_64);

      }area;

    } Core·TM_NX·FG;

    // Default functions given a Tape Machine
    Local Core·TM_NX·FG Core·TM_NX·fg;

  //----------------------------------------
  // Map

    typedef enum{
       Core·Map·Status·mu = 0
      ,Core·Map·Status·no_tape
      ,Core·Map·Status·not_computable
      ,Core·Map·Status·complete
    } Core·Map·Status;

    typedef enum{
       Core·Map·Completion·mu = 0
      ,Core·Map·Completion·no_tape                
      ,Core·Map·Completion·not_computable         
      ,Core·Map·Completion·failed                 
      ,Core·Map·Completion·perfect_fit            
      ,Core·Map·Completion·read_surplus           
      ,Core·Map·Completion·read_surplus_write_gap 
      ,Core·Map·Completion·write_available        
      ,Core·Map·Completion·write_gap              
    } Core·Map·Completion;

    const uint Core·Map·Completion·derailed =
        Core·Map·Completion·no_tape                
      | Core·Map·Completion·not_computable         
      | Core·Map·Completion·failed
      ;

    const uint Core·Map·Completion·on_track =
        Core·Map·Completion·perfect_fit            
      | Core·Map·Completion·read_surplus           
      | Core·Map·Completion·read_surplus_write_gap 
      | Core·Map·Completion·write_available        
      | Core·Map·Completion·write_gap              
      ;

    typedef Core·Map·Fn (*Core·Map·Fn)();

    // Link for Map
    typedef struct{
        Core·Link *domain;
        Core·Link *range;
        Core·Map·Fn *fn;
        Core·Map·Status status;
    } Core·Map·Tableau;

    void Core·map(Core·Map·Tableau t);

    // true if function enters the map loop, otherwise false.
    void Core·map(Core·Map·Tableau *t){
      #ifdef Core·DEBUG
      /*
        if(!t){
          fprintf(stderr, "Core·Map·Tableau:: given NULL t");
          return;
        }
        uint error = 0;
        if( (t->status & Core·Map·Completion·derailed) != 0 ){
          fprintf(stderr, "Core·Map:: prior map completion status is derailed.");
        }
        call(status ,t->domain);
        if( (t->domain->tableau->status & Core·TM·Head·Status·on_track) == 0 ){ 
          fprintf(stderr, "Core·Map:: domain is not on_track.");
          error++;
        }
        call(status ,t->range);
        if( (t->range->tableau->status & Core·TM·Head·Status·on_track) == 0 ){ 
          fprintf(stderr, "Core·Map:: range is not on_track.");
          error++;
        }
        if(error > 0) return;
      */
      #endif
      

    }


#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef Core·IMPLEMENTATION
  // declarations available to all of the IMPLEMENTATION go here
  //
    #ifdef Core·DEBUG
      #include <stdio.h>
    #endif

  // implementation to go into the lib.a file
  //
    #ifndef LOCAL
    #endif 

  #ifdef LOCAL

  //----------------------------------------
  // Utilities

    Core·Status Core·on_track(){ return Core·Status·on_track; }
    Core·Status Core·derailed(){ return Core·Status·derailed; }

    Local void *Core·offset(void *p ,size_t Δ){
      #ifdef Core·Debug
      if( !p ){
        fprintf(stderr ,"Core·offset:: given NULL `p'");
        return NULL;
      }
      #endif
      return (void *)( (AU *)p ) + Δ;
    }

    Local void *Core·offset_8AU(void *p ,size_t Δ){
      #ifdef Core·Debug
      if( !p ){
        fprintf(stderr ,"Core·offset_8AU:: given NULL `p'");
        return NULL;
      }
      #endif
      return (void *)( (uint64_t *)p ) + Δ;
    }

    Local bool Core·is_aligned_on_8AU(void *p){
      #ifdef Core·Debug
      if( !p ){
        fprintf(stderr ,"Core·is_aligned_on_8AU:: given NULL `p'");
        return false;
      }
      #endif
      return ( (uintptr_t)p & 0x7 ) == 0;
    }

    // find the lowest address in an 8 byte aligned window
    // returns the byte pointer to the least address byte in the window
    Local void *Core·floor_within_aligned_8AU(void *p){
      #ifdef Core·Debug
      if( !p ){
        fprintf(stderr ,"Core·floor_8AU:: given NULL `p'" );
        return NULL;
      }
      #endif
      return (void *)( (uintptr_t)p & ~(uintptr_t)0x7 );
    }

    // find the largest address in an 8 byte aligned window
    // returns the byte pointer to the greatest address byte in the window
    Local void *Core·ceiling_within_aligned_8AU(void *p){
      #ifdef Core·Debug
      if( !p ){
        fprintf(stderr ,"Core·ceiling_64:: given NULL `p'" );
        return NULL;
      }
      #endif
      return (void *)( (uintptr_t)p | 0x7 );
    }

    // Struct instance initialization
    Core·U = Core·u = {
       .on_track = Core·on_track
      ,.derailed = Core·derailed
      ,.offset = Core·offset
      ,.offset_8AU = Core·offset_8AU
      ,.is_aligned_on_8AU = Core·is_aligned_on_8AU
      ,.floor_within_aligned_8AU = Core·floor_within_aligned_8AU
      ,.ceiling_within_aligned_8AU = Core·ceiling_within_aligned_8AU
    };

  //----------------------------------------
  // Tape model, default implementation based on an array

    typedef struct{
      void *address;
    }Core·Tape·Address;
    Local Core·Tape·Address Core·Tape·address = {
      .address = NULL
    }

    typedef struct{
      void *address;
    }Core·Tape·Remote;
    Local Core·Tape·Remote Core·Tape·remote = {
      .address = NULL
    }

    typedef struct{
      AU *position;
      extent_t extent;
    }Core·Tape;

    Local Core·Tape·Topo Core·Tape·topo(Core·Tape *tape){
      if(!tape) return Core·Area·Topo·nonexistent;
      if(!tape->position) return Core·Area·Topo·empty;
      if(!tape->extent) return Core·Area·Topo·singleton;
      return Core·Area·Topo·segment;
    }

    Local Core·Status Core·Tape·extent(Core·Tape *tape ,extent_t *write_pt){
      #ifdef Core·Debug
        uint error = 0;
        if(!tape){
          fprintf(stderr,"Core·Tape·extent:: given NULL tape");
          error++;
        }else if(!tape->position){
          fprintf(stderr,"Core·Tape·extent:: extent requested for a marked as empty tape");
          error++;
        }
        if(!write_pt){
          fprintf(stderr,"Core·Tape·extent:: given NULL write_pt");
          error++;
        }
        if(error) return Core·Status·derailed;
      #endif
      return tape->extent;
    }    

    Local bool Core·Area·action.encloses_pt(Core·Area *area ,AU *pt);

    // a must be on tape
    Local Core·Status Core·Tape·is_leftmost(
      Core·Tape *tape ,Core·Tape·Address *a ,bool *result
    ){
      #ifdef Core·Debug
        uint error = 0;
        if(!tape){
          fprintf(stderr,"Core·Tape·is_leftmost:: given NULL tape");
          error++;
        }else if(!tape->position){
          fprintf(stderr,"Core·Tape·is_leftmost:: leftmost requested of empty tape");
          error++;
        }
        if(!result){
          fprintf(stderr,"Core·Tape·is_leftmost:: NULL result pointer");
          error++;
        }
        // making this a warning, but by spec it must be true
        if(!a){
          fprintf(stderr,"Core·Tape·is_leftmost:: given NULL address");
        }else if( !Core·Area·action.encloses_pt((Core·Area *)tape ,a->address) ){
          fprintf(stderr,"Core·Tape·is_leftmost:: address is outside tape bounds");
        }
        if(error) return Core·Status·derailed;
      #endif
        result = tape->position == a;
        return Core·Status·on_track;
    }    

    Local Core·Status Core·Tape·is_rightmost(
      Core·Tape *tape ,Core·Tape·Address *a ,bool *result
    ){
      #ifdef Core·Debug
        uint error = 0;
        if(!tape){
          fprintf(stderr,"Core·Tape·is_rightmost:: given NULL tape");
          error++;
        }else if(!tape->position){
          fprintf(stderr,"Core·Tape·is_rightmost:: rightmost requested of empty tape");
          error++;
        }
        if(!result){
          fprintf(stderr,"Core·Tape·is_rightmost:: NULL result pointer");
          error++;
        }
        // making this a warning, but by spec it must be true
        if(!a){
          fprintf(stderr,"Core·Tape·is_rightmost:: given NULL address");
        }else if( !Core·Area·action.encloses_pt((Core·Area *)tape ,a->address) ){
          fprintf(stderr,"Core·Tape·is_rightmost:: address is outside tape bounds");
        }
        if(error) return Core·Status·derailed;
      #endif
        result = tape->position == tape->position + tape->extent;
        return Core·Status·on_track;
    }    

    Local Core·Status Core·Tape·read(
      Core·Tape *tape ,Core·Tape·Address *a ,Core·Tape·Remote *remote
    ){
      #ifdef Core·Debug
        uint error = 0;
        if(!tape){
          fprintf(stderr,"Core·Tape·read:: given NULL tape");
          error++;
        }else if(!tape->position){
          fprintf(stderr,"Core·Tape·read:: read requested for a marked as empty tape");
          error++;
        }
        if(!a){
          fprintf(stderr,"Core·Tape·read:: given NULL address");
          error++;
        }else if( !Core·Area·action.encloses_pt((Core·Area *)tape ,a->address) ){
          fprintf(stderr,"Core·Tape·read:: address is outside tape bounds");
          error++;
        }
        if(!remote){
          fprintf(stderr,"Core·Tape·read:: given NULL remote");
          error++;
        }
        if(error) return Core·Status·derailed;
      #endif
      *(AU *)remote->address = *(AU *)a->address;
      return Core·Status·on_track;
    }

    Local Core·Status Core·Tape·write(
      Core·Tape *tape ,Core·Tape·Address *a ,Core·Tape·Remote *r
    ){
      #ifdef Core·Debug
        uint error = 0;
        if(!tape){
          fprintf(stderr,"Core·Tape·write:: given NULL tape");
          error++;
        }else if(!tape->position){
          fprintf(stderr,
            "Core·Tape·write:: write requested for a marked as empty tape"
          );
          error++;
        }
        if(!a){
          fprintf(stderr,"Core·Tape·write:: given NULL address");
          error++;
        }else if( !Core·Area·action.encloses_pt((Core·Area *)tape ,a->address) ){
          fprintf(stderr,"Core·Tape·write:: address is outside tape bounds");
          error++;
        }
        if(!r){
          fprintf(stderr,"Core·Tape·write:: given NULL remote");
          error++;
        }
        if(error) return Core·Status·derailed;
      #endif
      *(AU *)a->address = *(AU *)r->address;
      return Core·Status·on_track;
    }

    Local Core·Tape·FG Core·Tape·fg = {
       .topo = Core·Tape·topo
      ,.extent = Core·Tape·extent
      ,.is_leftmost = is_leftmost
      ,.is_rightmost = is_rightmost
      ,.read = Core·Tape·read
      ,.write = Core·Tape·write
    };

    //----------------------------------------
    // Area Model Implementation

    // Area to Tape Wrappers

    Local Core·Tape·Topo Core·Area·topo(Core·Area *area){
      return Core·Tape·fg.topo((Core·Tape *)area);
    }

    Local Core·Status Core·Area·extent(Core·Area *area, extent_t *write_pt){
      return Core·Tape·fg.extent((Core·Tape *)area, write_pt);
    }

    Local void Core·Area·read(Core·Area *area, Core·Tape·Address *a, Core·Tape·Remote *r){
      Core·Tape·fg.read((Core·Tape *)area, a, r);
    }

    Local void Core·Area·write(Core·Area *area, Core·Tape·Address *a, Core·Tape·Remote *r){
      Core·Tape·fg.write((Core·Tape *)area, a, r);
    }

    // initialization

    Local Core·Status Core·Area·set_position(Core·Area *area ,AU *new_position){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr, "Core·Area·set_position:: given NULL area");
        return Core·Status·derailed;
      }
      #endif
      area->position = new_position;
      return Core·Status·on_track;
    }

    Local Core·Status Core·Area·set_extent(Core·Area *area ,extent_t extent){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr, "Core·Area·set_extent:: given NULL area");
        return Core·Status·derailed;
      }
      #endif
      area->extent = extent;
      return Core·Status·on_track;
    }

    Local Core·Status Core·Area·set_position_right(Core·Area *area ,AU *new_position_right){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr, "Core·Area·set_position_right:: given NULL area");
        return Core·Status·derailed;
      }
      if(!area->position){
        fprintf(stderr, "Core·Area·set_position_right:: NULL position_left");
        return Core·Status·derailed;
      }
      #endif
      return Core·Area·set_extent(area, new_position_right - area->position);
    }

    Local Core·Status Core·Area·init_pe(Core·Area *area ,AU *position ,extent_t extent){
      Core·Status s1 = Core·Area·set_position(area, position);
      Core·Status s2 = Core·Area·set_extent(area, extent);
      return (s1 == Core·Status·derailed || s2 == Core·Status·derailed) ? Core·Status·derailed : Core·Status·on_track;
    }

    Local Core·Status Core·Area·init_pp(Core·Area *area ,AU *position_left ,AU *position_right){
      Core·Status s1 = Core·Area·set_position(area, position_left);
      Core·Status s2 = Core·Area·set_position_right(area, position_right);
      return (s1 == Core·Status·derailed || s2 == Core·Status·derailed) ? Core·Status·derailed : Core·Status·on_track;
    }

    // Read Properties

    Local bool Core·Area·empty(Core·Area *area){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·empty:: given NULL area");
        return true;
      }
      #endif
      return area->position == NULL;
    }

    Local Core·Status Core·Area·position(Core·Area *area ,AU **out_position){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·position:: given NULL area");
        *out_position = NULL;
        return Core·Status·derailed;
      }
      #endif
      *out_position = area->position;
      return Core·Status·on_track;
    }

    Local Core·Status Core·Area·position_right(Core·Area *area ,AU **out_position_right){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·position_right:: given NULL area");
        *out_position_right = NULL;
        return Core·Status·derailed;
      }
      if(!area->position){
        fprintf(stderr,"Core·Area·position_right:: request for right position when left is NULL");
        *out_position_right = NULL;
        return Core·Status·derailed;
      }
      #endif
      *out_position_right = area->position + area->extent;
      return Core·Status·on_track;
    }

    Local Core·Status Core·Area·extent(Core·Area *area ,extent_t *out_extent){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·extent:: given NULL area");
        *out_extent = 0;
        return Core·Status·derailed;
      }
      #endif
      *out_extent = area->extent;
      return Core·Status·on_track;
    }

    Local Core·Status Core·Area·complement(Core·Area *area ,AU *r ,AU **out_complement){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·complement:: given NULL area");
        *out_complement = NULL;
        return Core·Status·derailed;
      }
      if(!r){
        fprintf(stderr,"Core·Area·complement:: given NULL reference");
        *out_complement = NULL;
        return Core·Status·derailed;
      }
      #endif
      *out_complement = area->position + (area->position + area->extent - r);
      return Core·Status·on_track;
    }

    // Relationships

    Local bool Core·Area·encloses_pt(Core·Area *area ,AU *pt){
      return 
        (pt >= Core·Area·position(area)) 
        && (pt <= Core·Area·position_right(area));
    }

    Local bool Core·Area·encloses_pt_strictly(Core·Area *area ,AU *pt){
      return 
        (pt > Core·Area·position(area)) 
        && (pt < Core·Area·position_right(area));
    }

    Local bool Core·Area·encloses_area(Core·Area *outer ,Core·Area *inner){
      return 
        (Core·Area·position(inner) >= Core·Area·position(outer)) 
        && (Core·Area·position_right(inner) <= Core·Area·position_right(outer));
    }

    Local bool Core·Area·encloses_area_strictly(Core·Area *outer ,Core·Area *inner){
      return 
        (Core·Area·position(inner) > Core·Area·position(outer)) 
        && (Core·Area·position_right(inner) < Core·Area·position_right(outer));
    }

    Local bool Core·Area·overlap(Core·Area *area0 ,Core·Area *area1){
      return 
        Core·Area·position_right(area0) >= Core·Area·position(area1)
        && Core·Area·position(area0) <= Core·Area·position_right(area1);
    }

    // Aligned Containment

    Local Core·Status Core·Area·largest_aligned_64(Core·Area *outer ,Core·Area *inner_64){
      #ifdef Core·Debug
      if(!outer || !inner_64){
        fprintf(stderr, "Core·Area·largest_aligned_64:: given NULL area(s)");
        return Core·Status·derailed;
      }
      #endif

      uintptr_t p0 = (uintptr_t)Core·Area·position(outer);
      uintptr_t p1 = (uintptr_t)Core·Area·position_right(outer);

      AU *p0_64 = (AU *)( (p0 + 0x7) & ~(uintptr_t)0x7 );
      AU *p1_64 = (AU *)( (p1 - 0x7) & ~(uintptr_t)0x7 );

      if(p1_64 < p0_64){
        return Core·Area·set_position(inner_64 ,NULL);
      }else{
        return Core·Area·init_pp(inner_64 ,p0_64 ,p1_64);
      }
    }

    // Functions given an Area Argument
    Local Core·Area·FG Core·Area·fg = {
      // Given Area functions in parallel to the given Tape functions.
      ,.topo = Core·Area·topo
      ,.extent = Core·Area·extent
      ,.read = Core·Area·read
      ,.write = Core·Area·write

      // Given Area specific methods
      ,.init_pe = Core·Area·init_pe
      ,.init_pp = Core·Area·init_pp
      ,.set_position = Core·Area·set_position
      ,.set_position_left = Core·Area·set_position
      ,.set_position_right = Core·Area·set_position_right
      ,.set_extent = Core·Area·set_extent
      ,.position = Core·Area·position
      ,.position_left = Core·Area·position // Synonym
      ,.position_right = Core·Area·position_right
      ,.complement = Core·Area·complement
      ,.encloses_pt = Core·Area·encloses_pt
      ,.encloses_pt_strictly = Core·Area·encloses_pt_strictly
      ,.encloses_area = Core·Area·encloses_area
      ,.encloses_area_strictly = Core·Area·encloses_area_strictly
      ,.overlap = Core·Area·overlap
      ,.largest_aligned_64 = Core·Area·largest_aligned_64
    };

  //----------------------------------------
  // Tape Machine Implementation

  typedef struct {
    Core·Tape *tape;
    Core·Tape·Address *hd;
  } Core·TM_NX;

  // Returns the current head status.
  Local Core·TM·Head·Status Core·TM_NX·status(Core·TM_NX *tm){
    if(!tm || !tm->tape) return Core·TM·Head·Status·not_on_tape;

    bool is_at_edge;

    if(
      Core·Tape·fg.is_leftmost(tm->tape ,tm->hd ,&is_at_edge) 
      == 
      Core·Status·derailed
    )
      return Core·TM·Head·Status·not_on_tape;
    if(is_at_edge) return Core·TM·Head·Status·leftmost;

    if(
      Core·Tape·fg.is_rightmost(tm->tape ,tm->hd ,&is_at_edge) 
      == 
      Core·Status·derailed
    )
      return Core·TM·Head·Status·not_on_tape;
    if(is_at_edge) return Core·TM·Head·Status·rightmost;

    return Core·TM·Head·Status·interim;
  }

  // Mounts a tape onto the tape machine.
  Local Core·Status Core·TM_NX·mount(Core·TM_NX *tm ,Core·Tape *tape){
    #ifdef Core·DEBUG
      if(!tm){
        fprintf(stderr, "Core·TM_NX·mount:: given NULL tm\n");
        return Core·Status·derailed;
      }
      if(!tape){
        fprintf(stderr, "Core·TM_NX·mount:: given NULL tape\n");
        return Core·Status·derailed;
      }
    #endif
    tm->tape = tape;
    tm->hd = (AU *)Core·Tape·fg.extent(tape ,NULL);
    return Core·Status·on_track;
  }

  // Returns the associated tape.
  Local Core·Status Core·TM_NX·tape(Core·TM_NX *tm ,Core·Tape **out_tape){
    #ifdef Core·DEBUG
      if(!tm){
        fprintf(stderr, "Core·TM_NX·tape:: given NULL tm\n");
        return Core·Status·derailed;
      }
    #endif
    *out_tape = tm->tape;
    return Core·Status·on_track;
  }

  // Resets the tape head to the starting position.
  Local Core·Status Core·TM_NX·rewind(Core·TM_NX *tm){
    #ifdef Core·DEBUG
      if(!tm){
        fprintf(stderr, "Core·TM_NX·rewind:: given NULL tm\n");
        return Core·Status·derailed;
      }
      if(!tm->tape){
        fprintf(stderr, "Core·TM_NX·rewind:: tape is not mounted\n");
        return Core·Status·derailed;
      }
    #endif
    tm->hd = Core·Tape·fg.position(tm->tape ,NULL);
    return Core·Status·on_track;
  }

  // Checks if tape can be read.
  Local Core·Status Core·TM_NX·can_read(bool *result){
    *result = (Core·Tape·Topo·can_read & Core·Tape·fg.topo(tm->tape)) != 0;
    return Core·Status·on_track;
  }

  // Moves the tape head one step to the right.
  Local Core·Status Core·TM_NX·step(Core·TM_NX *tm){
    #ifdef Core·DEBUG
      if(!tm){
        fprintf(stderr, "Core·TM_NX·step:: given NULL tm\n");
        return Core·Status·derailed;
      }
      if(!tm->tape){
        fprintf(stderr, "Core·TM_NX·step:: tape is not mounted\n");
        return Core·Status·derailed;
      }
      if(!Core·TM_NX·can_step(tm)){
        fprintf(stderr, "Core·TM_NX·step:: stepping not possible\n");
        return Core·Status·derailed;
      }
    #endif

    tm->hd++;
    return Core·Status·on_track;
  }

  // Moves the tape head one step to the left.
  Local Core·Status Core·TM_NX·step_left(Core·TM_NX *tm){
    #ifdef Core·DEBUG
      if(!tm){
        fprintf(stderr, "Core·TM_NX·step_left:: given NULL tm\n");
        return Core·Status·derailed;
      }
      if(!tm->tape){
        fprintf(stderr, "Core·TM_NX·step_left:: tape is not mounted\n");
        return Core·Status·derailed;
      }
      if(!Core·TM_NX·can_step_left(tm)){
        fprintf(stderr, "Core·TM_NX·step_left:: stepping left not possible\n");
        return Core·Status·derailed;
      }
    #endif

    tm->hd--;
    return Core·Status·on_track;
  }

  // Determines if stepping right is possible.
  Local bool Core·TM_NX·can_step(Core·TM_NX *tm){
    return tm->hd < (AU *)Core·Tape·fg.extent(tm->tape ,NULL);
  }

  // Determines if stepping left is possible.
  Local bool Core·TM_NX·can_step_left(Core·TM_NX *tm){
    return tm->hd > (AU *)Core·Tape·fg.extent(tm->tape ,NULL);
  }

  // Moves the head to the given position if within bounds.
  Local Core·Status Core·TM_NX·cue(Core·TM_NX *tm ,AU *put){
    #ifdef Core·DEBUG
      if(!tm){
        fprintf(stderr, "Core·TM_NX·cue:: given NULL tm\n");
        return Core·Status·derailed;
      }
      if(!tm->tape){
        fprintf(stderr, "Core·TM_NX·cue:: tape is not mounted\n");
        return Core·Status·derailed;
      }
      if(put < (AU *)Core·Tape·fg.extent(tm->tape ,NULL) 
      || put > (AU *)Core·Tape·fg.extent(tm->tape ,NULL)){
        fprintf(stderr, "Core·TM_NX·cue:: given position is out of bounds\n");
        return Core·Status·derailed;
      }
    #endif

    tm->hd = put;
    return Core·Status·on_track;
  }

Local Core·TapeMachine·FG Core·TapeMachine·fg = {
    .mount = Core·TapeMachine·mount,
    .dismount = Core·TapeMachine·dismount,
    .rewind = Core·TapeMachine·rewind,
    .step = Core·TapeMachine·step,
    .step_left = Core·TapeMachine·step_left,
    .step_right = Core·TapeMachine·step_right,
    .can_step = Core·TapeMachine·can_step,
    .can_step_left = Core·TapeMachine·can_step_left,
    .fast_forward = Core·TapeMachine·fast_forward,
    .cue = Core·TapeMachine·cue,
    .status = Core·TapeMachine·status,

    .area = {
        .topo = Core·TapeMachine·topo,
        .extent = Core·TapeMachine·extent,
        .set_position = Core·TapeMachine·set_position,
        .set_extent = Core·TapeMachine·set_extent,
        .position = Core·TapeMachine·position,
        .position_right = Core·TapeMachine·position_right,
        .complement = Core·TapeMachine·complement,
        .encloses_pt = Core·TapeMachine·encloses_pt,
        .encloses_area = Core·TapeMachine·encloses_area,
        .overlap = Core·TapeMachine·overlap,
        .largest_aligned_64 = Core·TapeMachine·largest_aligned_64
    }
};


  // Initialize the function table
  Local Core·TM_NX·FG Core·TM_NX·fg = {
     .mount = Core·TM_NX·mount
    ,.tape = Core·TM_NX·tape
    ,.rewind = Core·TM_NX·rewind
    ,.can_read = Core·TM_NX·can_read
    ,.step = Core·TM_NX·step
    ,.step_left = Core·TM_NX·step_left
    ,.step_right = Core·TM_NX·step
    ,.status = Core·TM_NX·status
  };

    //----------------------------------------
    // Tape Machine xx

    typedef struct {
      Core·Tape *tape;
      AU *hd;
    } Core·TM_NX;

    // Returns the current head status.
    Local Core·TM·Head·Status Core·TM_NX·status(Core·TM_NX *tm){
      if(!tm) return Core·TM·Head·Status·mu;

      bool q;

      if(
        Core·Tape·is_lefttmost(tm->hd ,&q) == Core·Status·derailed
      )
        return Core·TM·Head·Status·not_on_tape;
      if(q) return Core·TM·Head·Status·leftmost;

      if(
        Core·Tape·is_lefttmost(tm->hd ,&q) == Core·Status·derailed
      )
        return Core·TM·Head·Status·not_on_tape;
      if(q) return Core·TM·Head·Status·rightmost;

      return Core·TM·Head·Status·interim;
    }

    // Mounts a tape onto the tape machine.
    Local void Core·TM_NX·mount(Core·TM_NX *tm ,Core·Tape *tape){
      #ifdef Core·DEBUG
        if(!tm){
          fprintf(stderr, "Core·TM_NX·mount:: given NULL tm\n");
          return;
        }
        if(!tape){
          fprintf(stderr, "Core·TM_NX·mount:: given NULL tape\n");
          return;
        }
      #endif
      tm->tape = tape;
      tm->hd = tape->position; // Initialize head to leftmost position
    }

  // Returns the associated tape.
  Local Core·Tape *Core·TM_NX·tape(Core·TM_NX *tm){
    #ifdef Core·DEBUG
      if(!tm){
        fprintf(stderr, "Core·TM_NX·tape:: given NULL tm\n");
        return NULL;
      }
    #endif
    return tm->tape;
  }

  // Resets the tape head to the starting position.
  Local void Core·TM_NX·rewind(Core·TM_NX *tm){
    #ifdef Core·DEBUG
      if(!tm){
        fprintf(stderr, "Core·TM_NX·rewind:: given NULL tm\n");
        return;
      }
      if(!tm->tape){
        fprintf(stderr, "Core·TM_NX·rewind:: tape is not mounted\n");
        return;
      }
    #endif
    tm->hd = tm->tape->position;
  }

  // Moves the tape head one step to the right.
  Local void Core·TM_NX·step(Core·TM_NX *tm){
    #ifdef Core·DEBUG
      if(!tm){
        fprintf(stderr, "Core·TM_NX·step:: given NULL tm\n");
        return;
      }
      if(!tm->tape){
        fprintf(stderr, "Core·TM_NX·step:: tape is not mounted\n");
        return;
      }
      if(!Core·TM_NX·can_step(tm)){
        fprintf(stderr, "Core·TM_NX·step:: stepping not possible\n");
        return;
      }
    #endif

    tm->hd++;

  }

  // Moves the tape head one step to the left.
  Local void Core·TM_NX·step_left(Core·TM_NX *tm){
    #ifdef Core·DEBUG
      if(!tm){
        fprintf(stderr, "Core·TM_NX·step_left:: given NULL tm\n");
        return;
      }
      if(!tm->tape){
        fprintf(stderr, "Core·TM_NX·step_left:: tape is not mounted\n");
        return;
      }
      if(!Core·TM_NX·can_step_left(tm)){
        fprintf(stderr, "Core·TM_NX·step_left:: stepping left not possible\n");
        return;
      }
    #endif

    tm->hd--;
  }

  // Determines if stepping right is possible.
  Local bool Core·TM_NX·can_step(Core·TM_NX *tm){
    #ifdef Core·DEBUG
      if(!tm){
        fprintf(stderr, "Core·TM_NX··can_step:: given NULL tm\n");
        return;
      }
    #endif
    return tm->hd <= Core·Tape·position_right(tm->tape);
  }

  // Determines if stepping left is possible.
  Local bool Core·TM_NX·can_step_left(Core·TM_NX *tm){
    if(!tm || !tm->tape) return false;
    return tm->hd > tm->tape->position;
  }

  // Fast forward to the rightmost position.
  Local void Core·TM_NX·fast_forward(Core·TM_NX *tm){
    #ifdef Core·DEBUG
      if(!tm){
        fprintf(stderr, "Core·TM_NX·fast_forward:: given NULL tm\n");
        return;
      }
      if(!tm->tape){
        fprintf(stderr, "Core·TM_NX·fast_forward:: tape is not mounted\n");
        return;
      }
    #endif

    tm->hd = tm->tape->position + tm->tape->extent - 1;
  }

  // Moves the head to the given position if within bounds.
  Local void Core·TM_NX·cue(Core·TM_NX *tm ,AU *put){
    #ifdef Core·DEBUG
      if(!tm){
        fprintf(stderr, "Core·TM_NX·cue:: given NULL tm\n");
        return;
      }
      if(!tm->tape){
        fprintf(stderr, "Core·TM_NX·cue:: tape is not mounted\n");
        return;
      }
      if(put < tm->tape->position || put > (tm->tape->position + tm->tape->extent - 1)){
        fprintf(stderr, "Core·TM_NX·cue:: given position is out of bounds\n");
        return;
      }
    #endif

    tm->hd = put;
  }

  // Initialize the function table
  Local Core·TM_NX·FG Core·TM_NX·fg = {
     .mount = Core·TM_NX·mount
    ,.tape = Core·TM_NX·tape
    ,.rewind = Core·TM_NX·rewind
    ,.step = Core·TM_NX·step
    ,.step_left = Core·TM_NX·step
    ,.step_right = Core·TM_NX·step_right
    ,.can_step = Core·TM_NX·can_step
    ,.can_step_left = Core·TM_NX·can_step_left
    ,.can_step_right = Core·TM_NX·can_step
    ,.fast_forward = Core·TM_NX·fast_forward
    ,.cue = Core·TM_NX·cue
    ,.status = Core·TM_NX·status
  };


    //----------------------------------------
    // Map

    // Map function using trampoline execution model
    Local Core·Map·Status Core·map(Core·Map·Fn fn){
      #ifdef Core·Debug
      if(!fn){
        fprintf(stderr,"Core·map:: given null function");
        return Core·Map·argument_guard;
      }
      if(
         true
         && fn != Core·Map·by_8AU
         && fn != Core·Map·AU_by_AU
         && fn != Core·write_hex
         && fn != Core·read_hex
      ){
        fprintf(stderr,"Core·map:: unrecognized copy function\n");
        return Core·Map·argument_guard;
      ) 
      #endif

      while(fn) fn = fn();
      return tf.copy.status;
    }

    //----------------------------------------
    // copy byte_by_byte 

    Core·Map·Fn Core·Map·Map·ByteByByte·perfect_fit;
    Core·Map·Fn Core·Map·Map·ByteByByte·read_surplus;
    Core·Map·Fn Core·Map·Map·ByteByByte·write_available;

    Local Core·Map·Fn Core·Map·AU_by_AU(){
      if(Core·Area·extent(Core·tf.copy.read) == Core·Area·extent(Core·tf.copy.write))
        return Core·Map·ByteByByte·perfect_fit;

      if(Core·Area·extent(Core·tf.copy.read) > Core·Area·extent(Core·tf.copy.write))
        return Core·Map·ByteByByte·read_surplus;

      return Core·Map·ByteByByte·write_available;
    }

    Local Core·Map·Fn Core·Map·ByteByByte·perfect_fit(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,*r);
        if(*r == r1) break;
        (*r)++;
        (*w)++;
      }while(true);

      Core·tf.copy.status = Core·Map·Status·perfect_fit;
      return NULL;
    }

    Local Core·Map·Fn Core·Map·ByteByByte·read_surplus(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;
      AU *w1 = Core·Area·position_right(Core·tf.copy.write);

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,*r);
        if(*w == w1) break;
        (*r)++;
        (*w)++;
      }while(true);

      Core·tf.copy.status = Core·Map·Status·write_available;
      return NULL;
    }

    Local Core·Map·Fn Core·Map·ByteByByte·write_avalable(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,*r);
        if(*r == r1) break;
        (*r)++;
        (*w)++;
      }while(true);

      Core·tf.copy.status = Core·Map·Status·read_surplus;
      return NULL;
    }

    //----------------------------------------
    // copy copy_64

    // 64-bit copy function with updated TableauFace terminology
    Core·Map·Fn Core·Map·by_8AU;
    Core·Map·Fn Core·Map·ByWord64·leadin;
    Core·Map·Fn Core·Map·ByWord64·bulk;
    Core·Map·Fn Core·Map·ByWord64·tail;

    // Initialize the copy_64 process
    Local Core·Map·Fn Core·Map·by_8AU(){
      // Determine the largest 64-bit aligned region within the read area
      Core·Area·largest_aligned_64(Core·tf.copy.read ,&Core·tl.copy_64.area_64);

      // Choose the correct function based on alignment
      if(Core·Area·empty(&Core·tl.copy_64.area_64)) return Core·Map·ByWord64·tail;
      if(Core·is_aligned_on_64(Core·Area·position(Core·tf.copy.read))) return Core·Map·ByWord64·bulk;
      return Core·Map·ByWord64·leadin;
    }

    // Lead-in byte copy (until alignment)
    Local Core·Map·Fn Core·Map·ByWord64·leadin(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r0_64 = Core·Area·position(&Core·tl.copy_64.area_64);
      AU **w = &Core·tf.copy.write_pt;

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,r0_64 ,*r);
        if(*r == r0_64) break;
        (*r)++;
        (*w)++;
      }while(true);

      return Core·Map·ByWord64·bulk;
    }

    // Bulk word copy
    Local Core·Map·Fn Core·Map·ByWord64·bulk(){
      uint64_t **r = (uint64_t **)&Core·tf.copy.read_pt;
      uint64_t **w = (uint64_t **)&Core·tf.copy.write_pt;
      uint64_t *r1_64 = Core·Area·position_right(&Core·tl.copy_64.area_64);

      do{
        **w = Core·tf.copy.read_fn_64(Core·tf.copy.read ,r1_64 ,*r);
        if(*r == r1_64) break;
        (*r)++;
        (*w)++;
      }while(true);

      return Core·Map·ByWord64·tail;
    }

    // Tail byte copy (unaligned trailing bytes)
    Local Core·Map·Fn Core·Map·ByWord64·tail(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(&Core·tl.copy_64.area_64);
      AU **w = &Core·tf.copy.write_pt;

      do{
        **w = Core·tf.copy.read_fn_8(Core·tf.copy.read ,r1 ,*r);
        if(*r == r1) break;
        (*r)++;
        (*w)++;
      }while(true);

      Core·tf.copy.status = Core·Map·Status·perfect_fit;
      return NULL;
    }

    //----------------------------------------
    // copy write hex

    Local uint16_t Core·byte_to_hex(AU byte){
      static const char hex_digits[] = "0123456789ABCDEF";
      return 
          (hex_digits[byte >> 4] << 8) 
        | hex_digits[byte & 0x0F];
    }

    // Forward Declarations
    Core·Map·Fn Core·Map·write_hex;
    Core·Map·Fn Core·Map·WriteHex·perfect_fit;
    Core·Map·Fn Core·Map·WriteHex·read_surplus;
    Core·Map·Fn Core·Map·WriteHex·write_available;

    // Hex Encoding: Initialize Map
    Local Core·Map·Fn Core·Map·write_hex(){
      if(Core·Area·extent(Core·tf.copy.read) == (Core·Area·extent(Core·tf.copy.write) >> 1)){
        return Core·Map·WriteHex·perfect_fit;
      }
      if(Core·Area·extent(Core·tf.copy.read) > (Core·Area·extent(Core·tf.copy.write) >> 1)){
        return Core·Map·WriteHex·read_surplus;
      }
      return Core·Map·WriteHex·write_available;
    }

    Local Core·Map·Fn Core·Map·WriteHex·perfect_fit(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;

      do {
        *(uint16_t *)*w = Core·hex.byte_to_hex(**r);
        if(*r == r1) break;
        (*r)++;
        (*w) += 2;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·perfect_fit;
      return NULL;
    }

    // Hex Encoding: Read Surplus
    Local Core·Map·Fn Core·Map·WriteHex·read_surplus(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.write);
      AU **w = &Core·tf.copy.write_pt;

      do {
        *(uint16_t *)*w = Core·write_hex.byte_to_hex(**r);
        if(*r == r1) break;
        (*r)++;
        (*w) += 2;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·read_surplus;
      return NULL;
    }

    // Hex Encoding: Write Available
    Local Core·Map·Fn Core·Map·WriteHex·write_available(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;
      AU *w1 = Core·Area·position_right(Core·tf.copy.write);

      do {
        *(uint16_t *)*w = Core·write_hex.byte_to_hex(**r);
        if(*w == w1) break;
        (*r)++;
        (*w) += 2;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·write_available;
      return NULL;
    }

    //----------------------------------------
    // copy read hex

    Local AU Core·hex_to_byte(uint16_t hex){
      AU high = hex >> 8;
      AU low = hex & 0xFF;

      high = 
          (high >= '0' && high <= '9') ? (high - '0')
        : (high >= 'A' && high <= 'F') ? (high - 'A' + 10)
        : (high >= 'a' && high <= 'f') ? (high - 'a' + 10)
        : 0;

      low = 
          (low >= '0' && low <= '9') ? (low - '0')
        : (low >= 'A' && low <= 'F') ? (low - 'A' + 10)
        : (low >= 'a' && low <= 'f') ? (low - 'a' + 10)
        : 0;

      return (high << 4) | low;
    }

    Core·Map·Fn Core·Map·read_hex;
    Core·Map·Fn Core·Map·ReadHex·perfect_fit;
    Core·Map·Fn Core·Map·ReadHex·read_surplus;
    Core·Map·Fn Core·Map·ReadHex·write_available;

    Local Core·Map·Fn Core·Map·read_hex(){
      if((Core·Area·extent(Core·tf.copy.read) >> 1) == Core·Area·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·perfect_fit;
      }
      if((Core·Area·extent(Core·tf.copy.read) >> 1) > Core·Area·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·read_surplus;
      }
      return Core·Map·ReadHex·write_available;
    }

    Local Core·Map·Fn Core·Map·ReadHex·perfect_fit(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;

      do {
        **w = Core·hex_to_byte(*(uint16_t *)*r);
        if(*r == r1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·perfect_fit;
      return NULL;
    }

    Local Core·Map·Fn Core·Map·ReadHex·read_surplus(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.write);
      AU **w = &Core·tf.copy.write_pt;

      do {
        **w = Core·tf.read_hex.hex_to_byte(*(uint16_t *)*r);
        if(*r == r1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·read_surplus;
      return NULL;
    }

    Local Core·Map·Fn Core·Map·ReadHex·write_available(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;
      AU *w1 = Core·Area·position_right(Core·tf.copy.write);

      do {
        **w = Core·tf.read_hex.hex_to_byte(*(uint16_t *)*r);
        if(*w == w1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·write_available;
      return NULL;
    }

    //----------------------------------------
    // copy read hex

    Core·Map·Fn Core·Map·read_hex;
    Core·Map·Fn Core·Map·ReadHex·perfect_fit;
    Core·Map·Fn Core·Map·ReadHex·read_surplus;
    Core·Map·Fn Core·Map·ReadHex·write_available;

    Local Core·Map·Fn Core·Map·read_hex(){
      if((Core·Area·extent(Core·tf.copy.read) >> 1) == Core·Area·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·perfect_fit;
      }
      if((Core·Area·extent(Core·tf.copy.read) >> 1) > Core·Area·extent(Core·tf.copy.write)){
        return Core·Map·ReadHex·read_surplus;
      }
      return Core·Map·ReadHex·write_available;
    }

    Local Core·Map·Fn Core·Map·ReadHex·perfect_fit(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;

      do {
        **w = Core·hex_to_byte(*(uint16_t *)*r);
        if(*r == r1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·perfect_fit;
      return NULL;
    }

    Local Core·Map·Fn Core·Map·ReadHex·read_surplus(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.write);
      AU **w = &Core·tf.copy.write_pt;

      do {
        **w = Core·hex_to_byte(*(uint16_t *)*r);
        if(*r == r1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·read_surplus;
      return NULL;
    }

    Local Core·Map·Fn Core·Map·ReadHex·write_available(){
      AU **r = &Core·tf.copy.read_pt;
      AU *r1 = Core·Area·position_right(Core·tf.copy.read);
      AU **w = &Core·tf.copy.write_pt;
      AU *w1 = Core·Area·position_right(Core·tf.copy.write);

      do {
        **w = Core·hex_to_byte(*(uint16_t *)*r);
        if(*w == w1) break;
        (*r) += 2;
        (*w)++;
      } while(true);

      Core·tf.copy.status = Core·Map·Status·write_available;
      return NULL;
    }

    //----------------------------------------
    // Initialization Blocks

    //----------------------------------------
    // Tableaux

    Core·TableauFace tf = {
       .copy = {
          .read = NULL
          ,.write = NULL
          ,.read_fn_8 = Core·Area·read_8_fwd
          ,.read_fn_64 = Core·Area·read_64_fwd
          ,.read_pt = NULL
          ,.write_pt = NULL
          ,.status = Core·Map·Status·uninitialized
       }
    };

    Core·TableauLocal tl = {
      .copy_64 = {
        .area_64 = {NULL ,0}
      }
    };

    Core·M m = {
      .Area·init_pe = Core·Area·init_pe
      ,.Area·init_pp = Core·Area·init_pp
      ,.Area·set_position = Core·Area·set_position
      ,.Area·set_position_left = Core·Area·set_position
      ,.Area·set_position_right = Core·Area·set_position_right
      ,.Area·set_extent = Core·Area·set_extent
      ,.Area·position = Core·Area·position
      ,.Area·position_left = Core·Area·position
      ,.Area·position_right = Core·Area·position_right
      ,.Area·extent = Core·Area·extent
      ,.Area·length_Kung = Core·Area·length_Kung
      ,.Area·empty = Core·Area·empty

      ,.Area·encloses_pt = Core·Area·encloses_pt
      ,.Area·encloses_pt_strictly = Core·Area·encloses_pt_strictly
      ,.Area·encloses_area = Core·Area·encloses_area
      ,.Area·encloses_area_strictly = Core·Area·encloses_area_strictly
      ,.Area·overlap = Core·Area·overlap
      ,.Area·largest_aligned_64 = Core·Area·largest_aligned_64
      ,.Area·complement = Core·Area·complement

      ,.Area·read_8_zero = Core·Area·read_8_zero
      ,.Area·read_8_fwd = Core·Area·read_8_fwd
      ,.Area·read_8_rev = Core·Area·read_8_rev
      ,.Area·read_64_zero = Core·Area·read_64_zero
      ,.Area·read_64_fwd = Core·Area·read_64_fwd
      ,.Area·read_64_rev = Core·Area·read_64_rev

      ,.is_aligned_on_64 = Core·is_aligned_on_64
      ,.floor_64 = Core·floor_64
      ,.ceiling_64 = Core·ceiling_64
      ,.offset_8 = Core·offset_8
      ,.offset_64 = Core·offset_64

      ,.byte_to_hex = Core·byte_to_hex
      ,.hex_to_byte = Core·hex_to_byte

      ,.copy = Core·map
      ,.Map·AU_by_AU = Core·Map·AU_by_AU
      ,.Map·by_8AU = Core·Map·by_8AU
      ,.Map·write_hex = Core·Map·write_hex
      ,.Map·read_hex = Core·Map·read_hex
    };

  #endif // LOCAL

#endif // IMPLEMENTATION
