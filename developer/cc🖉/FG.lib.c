/*
Template parameters:

FG·Type - Type of FG table, will be used to name the binding struct.

Note that template variables are _use once_ per #include. (They are #undef at the bottom of this file.)
       
*/

#ifndef FACE
#define FG·IMPLEMENTATION
#define FACE
#endif 

//--------------------------------------------------------------------------------
// Interface 

#ifndef Ξ(FG·FACE ,FG·Type)
#define Ξ(FG·FACE ,FG·Type)


  #define FG·DEBUG
  #ifdef FG·DEBUG
    #include <stdio.h>
  #endif

  #define FG·ALL  ( defined(FG·Type) )
  #define FG·NONE ( !defined(FG·Type) )
  #if !( FG·ALL || FG·NONE )
    #error "FG template inconsistency: must define all or none of: FG·Type"
  #endif

  //----------------------------------------
  // Macro to call a function in the FG table with debug checks
  // Usage: FG·call(tm, function_name, arg1, arg2, ...)
  // Expands to: (tm.fg->function_name)((tm)->t, arg1, arg2, ...)
  // With debug checks for NULL pointers when FG·DEBUG is defined
  //----------------------------------------
  #if FG·ALL

    //#define  ALL_VAL V0··V1··V2 ...
    // the double cdot underscore non aliasing across identifiers
    #define  FG·ALL_VAL FG·Type
    // FG is not yet a type, so we don't have this
    // #define FG_t Ξ(FG_t ,FG·ALL_VAL)

    // a simplifying naming convention
    #define FG·Binding Ξ(FG·Type)
    #define FG·FG      Ξ(FG·Type ,FG)
    #define FG·Tableau Ξ(FG·Type ,Tableau)

    // as a convention, we name the binding after the type
    typedef struct{
      FG·FG *fg;
      FG·Tableau *tableau;
    } FG·Binding;

    inline void FG·wellformed_binding(FG·Binding b){
      #ifdef FG·DEBUG
        FG·Guard·init_count(chk);
        FG·Guard·fg.check(&chk, 1, b.fg,      "NULL fg table");
        FG·Guard·fg.check(&chk, 1, b.tableau, "NULL tableau");
        FG·Guard·assert(chk);
      #endif
    }

     // note the use of the comma operator to return the result from the b.fg->fn call
    #define FG·call(b, fn, ...) \
      ( FG·wellformed_binding(b) ,b.fg->fn(b.tableau, ##__VA_ARGS__) )

  #endif

#endif // FACE

//--------------------------------------------------------------------------------
// Implementation

#ifdef FG·IMPLEMENTATION

  //----------------------------------------
  // implementation to go into the lib.a file
  //----------------------------------------
  #ifndef LOCAL
  #endif //#ifndef LOCAL

  //----------------------------------------
  // implementation to go at the bottom of the source file
  //----------------------------------------
  #ifdef LOCAL
  #endif // #ifdef LOCAL

#endif // IMPLEMENTATION

// template variables are use once
#undef FG·Type

