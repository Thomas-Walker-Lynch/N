
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
