/*
  See also https://github.com/18sg/uSHET/blob/master/lib/cpp_magic.h 
  and tutorial at: http://jhnet.co.uk/articles/cpp_magic

  I could not have even thought about writing this without Jonathan Heathcote's
  little tutorial.
 
  The 'twion' is cute. It is reminiscent of a complex number in math. It is a
  pair given to functions that only take singletons.

*/
 


/*===========================================================================
Constants
===========================================================================*/

#define COMMA ,
#define SEMICOLON ;

#define _TWION_0 ~,0
#define _TWION_1 ~,1

// RWR == rewrite rule, RWR_ is followed by macro name it is used in `_<name>_`
// Matching text is replaced with nothing, making it empty
// Potential for aliasing if x or y have '__oo__' embedded in them.
#define _RWR_NOT__0 
#define _RWR_AND__1__oo__1
#define _RWR_OR__0__oo__0

// add more of this form to register other equalities
#define _RWR_EQ__0__oo__0
#define _RWR_EQ__1__oo__1

/*===========================================================================
Logic
===========================================================================*/

// user versions of these functions appear latter in the file

//----------------------------------------
// primitive access

  // note: _FIRST of nothing, _FIRST(), means passing an empty_item as the first item
  // so it will return empty.
  #define _FIRST(a ,...) a
  #define _SECOND(a ,b ,...) b

//----------------------------------------
// existence

  // `##` prevents rewrite of _TWION_ in the _EXISTS_ITEM_1 macro
  #define _EXISTS_ITEM_2(x_item) _SECOND(x_item ,1) 
  #define _EXISTS_ITEM_1(x_item) _EXISTS_ITEM_2(_TWION_0##x_item)

  #define EXISTS_ITEM(x_item)   _EXISTS_ITEM_1(x_item)
  #define EXISTS(...) EXISTS_ITEM( _FIRST(__VA_ARGS__) )

  #define _NOT_EXISTS_ITEM_2(x_item) _SECOND(x_item ,0) 
  #define _NOT_EXISTS_ITEM_1(x_item) _NOT_EXISTS_ITEM_2(_TWION_1##x_item)

  #define NOT_EXISTS_ITEM(x_item)   _NOT_EXISTS_ITEM_1(x_item)
  #define NOT_EXISTS(...) NOT_EXISTS_ITEM( _FIRST(__VA_ARGS__) )

  // useful to use with rewrite rules that substitute to nothing
  #define MATCH(x_item) NOT_EXISTS(x_item)
  #define NOT_MATCH(x_item) EXISTS(x_item)

//----------------------------------------
// primitive connectors

  #define _NOT_1(x_item) MATCH( _RWR_NOT__##x_item )
  #define _NOT(x_item) _NOT_1(x_item)

  #define _AND_1(x_item ,y_item) MATCH( _RWR_AND__##x_item##__oo__##y_item )
  #define _AND(x_item ,y_item) _AND_1(x_item ,y_item)

  #define _OR_1(x_item ,y_item) NOT_MATCH( _RWR_OR__##x_item##__oo__##y_item )
  #define _OR(x_item ,y_item) _OR_1(x_item ,y_item)

//----------------------------------------
// equality 
//   more general than a connector because more rules can be added.
//   each has the form:
//      _RWR_EQ__<x>__oo__<y>
//   for example:
//      _RWR_EQ__0__oo__0

  #define _EQ(x_item ,y_item) MATCH( _RWR_EQ__##x_item##__oo__##y_item )
  #define EQ(x_item ,y_item) _EQ(x_item ,y_item)

  #define _NOT_EQ(x_item ,y_item) EXISTS(_RWR_EQ__##x_item##__oo__##y_item)
  #define NOT_EQ(x_item ,y_item) _NOT_EQ(x_item ,y_item)


//----------------------------------------
// connectors

  #define _BOOL_2(x_item) \
    _AND(\
       EXISTS_ITEM( _FIRST(x_item) )            \
      ,NOT_MATCH( _RWR_EQ__0__oo__##x_item) \
    )
  #define _BOOL_1(x_item) _BOOL_2(x_item)
  #define BOOL(x_item) _BOOL_1(_FIRST(x_item))


  #define NOT(x_item) _NOT(BOOL(x_item))
  #define AND(x_item ,y_item) _AND(BOOL(x_item) ,BOOL(y_item))
  #define OR(x_item ,y_item) _OR(BOOL(x_item) ,BOOL(y_item))
  

