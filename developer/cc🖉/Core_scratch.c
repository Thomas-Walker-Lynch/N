  //----------------------------------------
  // Tape model

    typedef struct Core·Tape·Address;
    typedef struct Core·Tape·Remote;
    typedef struct Core·Tape;

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

    const Core·Tape·Topo Core·Tape·Topo·void =
        Core·Tape·Topo·nonexistent
      | Core·Tape·Topo·empty
      ;

    const Core·Tape·Topo Core·Tape·Topo·can_read =
      Core·Tape·Topo·singleton
      | Core·Tape·Topo·segment 
      | Core·Tape·Topo·circle  
      | Core·Tape·Topo·cyclic  
      | Core·Tape·Topo·infinite
      ;
      
    const Core·Tape·Topo Core·Tape·Topo·bounded =
      Core·Tape·Topo·singleton
      | Core·Tape·Topo·segment 
      ;

    const Core·Tape·Topo Core·Tape·Topo·tail_cyclic =       
        Core·Tape·Topo·circle
      | Core·Tape·Topo·tail_cyclic
      ;

    typedef struct{
      Core·Tape·Topo (*topo)(Core·Tape *tape);
      Core·Tape·Extent·Status (*extent)(Core·Tape *tape ,extent_t *extent_pt);

      Core·Status (*origin)(Core·Tape *tape ,Core·Tape·Address * ,bool *result);
      Core·Status (*rightmost)(Core·Tape *tape ,Core·Tape·Address * ,bool *result);

      Core·Status (*is_leftmot)(Core·Tape *tape ,Core·Tape·Address * ,bool *result);
      Core·Status (*is_rightmost)(Core·Tape *tape ,Core·Tape·Address * ,bool *result);
      Core·Status (*read)(Core·Tape *tape ,Core·Tape·Address *,Core·Tape·Remote *);
      Core·Status (*write)(Core·Tape *tape ,Core·Tape·Address * ,Core·Tape·Remote *);
    }Core·Tape·FG;

    // a default implementation based on an array
    Core·Tape·FG Core·Tape·fg;

  //----------------------------------------
  // Area model

    typedef struct Core·Area;

      
    // addresses must be on the tape/area
    typedef struct{
      // area versions of the functions given tape
      Core·Tape·Topo (*topo)(Core·Area *area);
      Core·Tape·Extent·Status (*extent)(Core·Area *area ,extent_t *write_pt);
      Core·Status (*is_leftmot)(Core·Tape *tape ,Core·Tape·Address *a ,bool *result);
      Core·Status (*is_rightmost)(Core·Tape *tape ,Core·Tape·Address *a ,bool *result);
      Core·Status (*read)(Core·Area *area ,Core·Tape·Address *a ,Core·Tape·Remote *remote);
      Core·Status (*write)(Core·Area *area ,Core·Tape·Address *a ,Core·Tape·Remote *remote);

      // initialize area
      Core·Status (*init_pe)(Core·Area *area ,void *position ,extent_t extent);
      Core·Status (*init_pp)(Core·Area *area ,void *position_left ,void *position_right);
      Core·Status (*set_position)(Core·Area *area ,AU *new_position);
      Core·Status (*set_position_left)(Core·Area *area ,AU *new_position); // synonym
      Core·Status (*set_position_right)(Core·Area *area ,AU *new_position_right);
      Core·Status (*set_extent)(Core·Area *area ,extent_t extent); 

      // read area properties
      AU *(*position)(Core·Area *area);
      AU *(*position_left)(Core·Area *area); // synonym
      AU *(*position_right)(Core·Area *area);

      AU *(*complement)(Core·Area *area ,AU *r);

      // area relationships
      bool (*encloses_pt)(Core·Area *area ,AU *pt);
      bool (*encloses_pt_strictly)(Core·Area *area ,AU *pt);
      bool (*encloses_area)(Core·Area *outer ,Core·Area *inner);
      bool (*encloses_area_strictly)(Core·Area *outer ,Core·Area *inner);
      bool (*overlap)(Core·Area *a ,Core·Area *b);
      void (*largest_aligned_64)(Core·Area *outer ,Core·Area *inner_64);

    } Core·Area·FG;

    Core·Area·FG Core·Area·fg;


----------------------------------------


    //----------------------------------------
    // Area

    // initialize an area

    Local void Core·Area·set_position(Core·Area *area ,void *new_position){
      area->position = new_position;
    }
    Local extent_t Core·Area·set_extent(Core·Area *area ,extent_t extent){
      return area->extent = extent;
    }
    Local void Core·Area·set_position_right(Core·Area *area ,void *new_position_right){
      Core·Area·set_extent(new_position_right - area->position);
    }
    Local void Core·Area·init_pe(Core·Area *area ,AU *position ,extent_t extent){
      Core·Area·set_position(position);
      Core·Area·set_extent(extent);
    }
    Local void Core·Area·init_pp(Core·Area *area ,void *position_left ,void *position_right){
      Core·Area·set_position_left(position_left);
      Core·Area·set_position_right(position_right);
    }

    // read area properties

    Local bool Core·Area·empty(Core·Area *area){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·empty:: given NULL area");
        return true;
      }
      #endif
      return area->position == NULL;
    }

    // Requesting a NULL position is a logical error, because a NULL position
    // means the Area is empty and has no position. Instead, use the `empty`
    // predicate.
    Local AU *Core·Area·position(Core·Area *area){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·position:: given NULL area");
        return NULL;
      }
      if(!area->position){
        fprintf(stderr,"Core·Area·position:: request for position when it is NULL");
      }
      #endif
      return area->position;
    }


    Local AU *Core·Area·position_right(Core·Area *area){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·position_right:: given NULL area");
        return NULL;
      }
      #endif
      return area->position + area->extent;
    }
    Local extent_t Core·Area·extent(Core·Area *area){
      #ifdef Core·Debug
      if(!area){
        fprintf(stderr,"Core·Area·extent:: given NULL area");
        return 0;
      }
      #endif
      return area->extent;
    }
    Local AU Core·Area·length_Kung(Core·Area *area){
      if(!Core·Area·position_left(area)) return 0;
      if(Core·Area·extent(area) >= 2) return 3;
      return Core·Area·extent(area) + 1;
    }

    Local bool Core·Area·encloses_pt(Core·Area *area ,AU *pt){
      return 
        (pt >= Core·Area·position_left(area)) 
        && (pt <= Core·Area·position_right(area));
    }
    Local bool Core·Area·encloses_pt_strictly(Core·Area *area ,AU *pt){
      return 
        (pt > Core·Area·position_left(area)) 
        && (pt < Core·Area·position_right(area));
    }
    Local bool Core·Area·encloses_area(Core·Area *outer ,Core·Area *inner){
      return 
        (Core·Area·position_left(inner) >= Core·Area·position_left(outer)) 
        && (Core·Area·position_right(inner) <= Core·Area·position_right(outer));
    }
    Local bool Core·Area·encloses_area_strictly(Core·Area *outer ,Core·Area *inner){
      return 
        (Core·Area·position_left(inner) > Core·Area·position_left(outer)) 
        && (Core·Area·position_right(inner) < Core·Area·position_right(outer));
    }

    // Possible cases of overlap ,including just touching
    // 1. interval 0 to the right of interval 1 ,just touching p00 == p11
    // 2. interval 0 to the left of interval 1 ,just touching p01 == p10
    // 3. interval 0 wholly contained in interval 1
    // 4. interval 0 wholly contains interval 1
    Local bool Core·Area·overlap(Core·Area *area0 ,Core·Area *area1){
      return 
        Core·Area·position_right(area0) >= Core·Area·position_left(area1)
        && Core·Area·position_left(area0) <= Core·Area·position_right(area1);
    }

     // find the largest contained interval aligned on 64 bit boundaries
    static void Core·Area·largest_aligned_64(Core·Area *outer ,Core·Area *inner_64){
      uintptr_t p0 = (uintptr_t)Core·Area·position_left(outer);
      uintptr_t p1 = (uintptr_t)Core·Area·position_right(outer);

      AU *p0_64 = (AU *)( (p0 + 0x7) & ~(uintptr_t)0x7 );
      AU *p1_64 = (AU *)( (p1 - 0x7) & ~(uintptr_t)0x7 );

      if(p1_64 < p0_64){
        Core·Area·set_position(inner_64 ,NULL);
      }else{
        Core·Area·init_pp(inner_64 ,p0_64 ,p1_64);
      }
    }

    // complement against the extent of the area (reverse direction)
    // works for byte pointer
    // works for aligned word pointer
    Local AU *Core·Area·complement(Core·Area *area ,AU *r){
      return Core·Area·position_left(area) + (Core·Area·position_right(area) - r);
    }


  //----------------------------------------
  // model

    typedef struct{
      Core·Tableau tableau;
    }Core·Tableau·State;


    // some default instances

  //----------------------------------------
  // Tape model - Array Area
  //    an array area is represented by `position` and `extent`.

    // identical to Core·Link, used for typing pointers
    typedef struct{
      Core·Area·ActionTable *action;
      Core·Area·Tableau·Face *face;
      Core·Area·Tableau·State *state;
      Core·NextTable *next_table;
    }Core·Area·Array·Link;

    Core·Link *Core·Area·Array·topo(Core·Link *lnk){
      #ifdef Core·Debug
        if(!lnk){
          fprintf(stderr,"Core·Area·Array·topo:: given NULL lnk");
          return NULL;
        }
        if(!lnk->face){
          fprintf(stderr,"Core·Area·Array·topo:: given NULL face");
          return NULL;
        }
      #endif
      l = (Core·Area·Array·Link *)lnk;
      if(l->face->extent == 0) l->face->status = Core·Area·Topo·singleton;
      l->face->status = Core·Area·Topo·segment;
      return &l->next_table->on_track;
    }

    Core·Link *Core·Area·Array·copy(Core·Link *link){
      #ifdef Core·Debug
        uint error = Core·Link·check(
          link
          ,Core·Link·Mode·action | Core·Link·Mode·face | Core·Link·Mode·next_table
        );
        if(error) return &link->next_table->derailed;
        if(!&link->face->remote) return &link->next_table->derailed;
        Core·Link link2{
          .action = Core·Area·address_valid
          ,.face = link->face
          ,.state = NULL
          ,.next_table = {
            .on_track = NULL
            .derailed = link->next_table->derailed
          }
        }        
        initiate(link2);
      #endif
      l = (Core·Area·Array·Link *)link;
      return &link->next_table->on_track;
    }


    Local Core·Area·ActionTable Core·Area·Array·action_table = {
      .tape = {
        .topo = Core·Area·Array·topo
        .read
        .write
        .extent
      }
      .psoition_right
      .complement
      .address_valid
      .encloses_pt_strictly_q
      .encloses_area_q
      .encloses_area_strictly_q
      .overlap_q
    }

    typedef struct{
      AU *position;
      extent_t extent;
    } Core·Area;

    // I removed the unions, as they are debugging hazards, and also might confuse the optimizer
    typedef struct{
        struct{
        } byte_by_byte;
        struct{
          Area area_64;
        } copy_64;
        struct{
        } read_hex;
        struct{
        } write_hex;
    } Core·TableauLocal;

  // this part goes into Maplib.a
  #ifndef LOCAL
  #endif 

------------------


Local·Topo Area·topo_byte_array(Area *area){
  if(!area) return Core·Area·Topo·nonexistent;
  if(!area->position) return Core·Area·Topo·empty;
  if(area->extent == 0) return Core·Area·Topo·singleton;
  return Core·Area·Topo·finite;
}


    typedef struct{
      AU *position;
      extent_t extent;
    } Core·Area;



    typedef struct{
      Core·Area *area;
      AU *hd;
    } Core·TM·Array;


    Local Node *Core·step_AU(Node *node){
      Core·Step·Node *step_node = (Core·Step·Node *)node;
      step_node->hd = Core·offset(step_node->tm->hd ,1);
      return node->next;
    }

    Local Node *Core·step_8AU(Node *node){
      Core·Step·Node *step_node = (Core·Step·Node *)node;
      step_node->hd = Core·offset_8AU(step_node->tm->hd ,1);
      return node->next;
    }



-----
    typedef struct{
      Core·Tableau *tableau;
      Core·Action *action;
    }Core·Link;


    typedef void (*Core·Fn)(Core·Tableau *);



    typedef struct{
                           void mount(Core·TM *tm ,Core·Area·M *a);
                          void rewind(Core·TM *tm);
                            void step(Core·TM *tm);
                       void step_left(Core·TM *tm);
                      void step_right(Core·TM *tm); // synonym for step
           Core·TM·Head·Status status(Core·TM *tm);
          Core·Tape·Topology topology(Core·TM *tm); // tape machine can abstract the tape
    } Core·TM_NX·FTable;





#include <stddef.h> // for offsetof
#include <stdio.h>  // for printf, etc.

typedef struct Core·TM Core·TM;

// Example of a Head Status enum
typedef enum {
    Core·TM·Head·Status·mu,
    Core·TM·Head·Status·not_on_tape,
    Core·TM·Head·Status·leftmost,
    Core·TM·Head·Status·interim,
    Core·TM·Head·Status·rightmost
} Core·TM·Head·Status;

// Some placeholders for unknown types
typedef struct Core·Area·M { /* ... */ } Core·Area·M;
typedef enum   Core·Tape·Topology { /* ... */ } Core·Tape·Topology;

// The "tableau" struct: holds data relevant to an instance
typedef struct {
    Core·TM *tm;                  // a pointer to some machine
    Core·Area·M *a;               // some area pointer
    Core·TM·Head·Status status;   // the machine's current head status
    Core·Tape·Topology topology;  // tape topology
} Core·TM_NX·Tableau;

// Define a function-pointer type: it operates on a Tableau
typedef void (*Core·TM_NX·Fn)(Core·TM_NX·Tableau *t);

// A table (struct) of function pointers—akin to a “vtable” or dictionary of methods
typedef struct {
    Core·TM_NX·Fn mount;
    Core·TM_NX·Fn rewind;
    Core·TM_NX·Fn step;
    Core·TM_NX·Fn step_left;
    Core·TM_NX·Fn step_right;
    Core·TM_NX·Fn status;
    Core·TM_NX·Fn topology;
} Core·TM_NX·FnTable;

/*
 * A macro to create named enumeration constants that hold
 * the byte offset of each field within Core·TM_NX·FnTable.
 *
 * Example usage inside an enum:
 *   enum {
 *       offset(Core·TM_NX·FnTable, mount),
 *       offset(Core·TM_NX·FnTable, rewind),
 *       ...
 *   };
 */
#define offset(Type, field)  offset_##field = offsetof(Type, field)

// Create an enum with an entry for each function-pointer field.
// Each enumerator will hold the byte offset of that field in the struct.
enum {
    offset(Core·TM_NX·FnTable, mount),
    offset(Core·TM_NX·FnTable, rewind),
    offset(Core·TM_NX·FnTable, step),
    offset(Core·TM_NX·FnTable, step_left),
    offset(Core·TM_NX·FnTable, step_right),
    offset(Core·TM_NX·FnTable, status),
    offset(Core·TM_NX·FnTable, topology)
};

// An example “global” or “default” FnTable (you would define real function pointers here)
static const Core·TM_NX·FnTable defaultFnTable = {
    .mount     = NULL, // or mount_impl,
    .rewind    = NULL, // or rewind_impl,
    .step      = NULL,
    .step_left = NULL,
    .step_right= NULL,
    .status    = NULL,
    .topology  = NULL
};

/*
 * Dispatcher: given a byte offset (one of the offset_XXX enum values)
 * and a tableau, find the correct function pointer in defaultFnTable and call it.
 */
void Core·TM_MX(Core·TM_NX·Tableau *t, size_t fnOffset)
{
    // We know each field in Core·TM_NX·FnTable is of type Core·TM_NX·Fn
    // (i.e. a pointer-to-function). We'll do pointer arithmetic on a
    // (Core·TM_NX·Fn *) pointer, but we must convert the byte offset to an index.
    
    // Point a function-pointer array at the start of defaultFnTable:
    const Core·TM_NX·Fn *fnArray = (const Core·TM_NX·Fn *)&defaultFnTable;
    
    // Convert the byte offset to an index in the function-pointer array.
    // Each element is the size of (Core·TM_NX·Fn).
    // Typically (Core·TM_NX·Fn) is just a pointer, so we do:
    size_t index = fnOffset / sizeof(Core·TM_NX·Fn);

    // Grab the function pointer
    Core·TM_NX·Fn fn = fnArray[index];
    if (!fn) {
        // Handle the case if the pointer is NULL (not implemented)
        printf("Function pointer at offset %zu is not implemented.\n", fnOffset);
        return;
    }

    // Invoke it
    fn(t);
}

// Example usage
int main(void)
{
    Core·TM_NX·Tableau myTableau = { 0 };
    // Suppose we want to call "step" from the table:
    Core·TM_MX(&myTableau, offset_step);

    // If it's NULL, the dispatcher prints a message. If not, it would call the function.
    return 0;
}
