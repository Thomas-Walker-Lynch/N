/*
  This will print at compile time as:

    example/custom_type.cli.c:6:9: note: ‘#pragma message: C -> 7’
      6 | #pragma message( STR_VAL(C) )
        |         ^~~~~~~

  Note the C->7 on the right side of the first line. That printed
  the name of the macro, and its value. This is a 'note' not an 'error'.
*/

#include "cpp_ext.c"
#include "Core.lib.c"
#include "Binding.lib.c"

/*
  This defines the Bespoke type. 

  Here by 'type' we mean a Tableau to FG table binding.
*/

  #define Binding Bespoke
  #include "Binding.lib.c"
  #define SET__Binding__Bespoke

  // This defines the FG table type for Bespoke (aka vtable). Each instance is a different implementation of the type sharing the same interface.
  typedef struct Bespoke·FG{
    void (*version)(Bespoke tm);
    void (*report)(Bespoke tm ,char *mess);
  }Bespoke·FG;

  // This defines the Tableau, the data that is given as a first argument of each FG table entry.  
  typedef struct Bespoke·Tableau{

  }Bespoke·Tableau;

  // This creates a FG table to Tableau binding type. We will use this to represent the type.
  // The EQ pattern is required for `FIND_ITEM` to recognize the type:

  // these functions will be used for making an instance of the abstract Bespoke type

  void Bespoke·version(Bespoke tm){ printf("version 1.0\n"); };
  void Bespoke·report(Bespoke tm ,char *mess){ printf("The report: %s\n" ,mess);};


int main(){

  printf("running example binding.cli.c on %s at %s\n", __DATE__, __TIME__);

  SHOW(Ξ(Binding·TYPE ,FG));

  #define A 5
  SHOW(A);

  #define B Ξ(X,Y)
  SHOW(B);

  // one FG table will be shared by many instances of Bespoke, maybe all of them
  Bespoke·FG fg = {
     .version = Bespoke·version
    ,.report = Bespoke·report
  };

  // The Bespoke·Tableau carries the instance data', often is one to one to a binding.
  Bespoke·Tableau t;

  // allocate a binding
  Bespoke spoke;

  // Initialize the binding, this the job of a type specific init function.
  // in C we don't know where the user is allocating the data, otherwise we would
  // have a factory instead of an initializer.
  spoke.fg = &fg;
  spoke.tableau = &t;

  // direct use of the fg table, the user/programmer should not do this
  fg.version(spoke);
  fg.report(spoke ,"fg.report");

  // does the binding have null pointers?
  printf("fg: %p, tableau: %p\n", (void*)spoke.fg, (void*)spoke.tableau);

  // calling though the binding 
  spoke.fg->version(spoke);
  spoke.fg->report(spoke ,"spoke.fg->report");
  
  printf("and version again\n");
  spoke.fg->version(spoke);
  printf("before Binding·call to version\n");
  Binding·call(spoke ,version);
  printf("after Binding·call to version.\n");

  printf("before Binding·call to report\n");
  Binding·call(spoke ,report ,"Binding·call to report");
  printf("after Binding·call to report\n");
  
  return 0;
}

#define LOCAL
#include "Core.lib.c"


