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

  #define EQ__Bespoke__oo__Bespoke
  #undef  Binding·TYPE
  #define Binding·TYPE Bespoke
  #include "../cc🖉/Binding.lib.c"

#if 0

  // This defines the FG table type for Bespoke (aka vtable). Each instance is a different implementation of the type sharing the same interface.
  typedef struct{
    void (*version)(Bespoke c);
    void (*report)(Bespoke c ,*mess);
  }Bespoke·FG;

  // This defines the Tableau, the data that is given as a first argument of each FG table entry.  
  typedef struct{
    void (*version)(Bespoke c);
    void (*report)(Bespoke c ,*mess);
  }Bespoke·Tableau;

  // This creates a FG table to Tableau binding type. We will use this to represent the type.
  // The EQ pattern is required for `FIND_ITEM` to recognize the type:

  // these functions will be used for making an instance of the abstract Bespoke type
  void Bespoke·version(Bespoke c){ printf("A Bespoke instance") };
  void Bespoke·report(Bespoke c ,*mess){ printf("A Bespoke instance says %s" ,mess)};

#endif

int main(){
  printf("running example binding.cli.c on %s at %s\n", __DATE__, __TIME__);

  #define A 5
  SHOW(A);

  #define B Ξ(X,Y)
  SHOW(B);

#if 0
  Bespoke·FG = {
     .version = Bespoke·version
    ,.report = Bespoke·report
  };
#endif

  
  return 0;
}

#define IMPLEMENTATION
#define LOCAL

//#include "../cc🖉/Core.lib.c"
//#include "../cc🖉/Binding.lib.c"
