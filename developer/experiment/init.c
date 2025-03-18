#include <stdio.h>

typedef int (*fn)();

typedef struct {
  fn x;
  fn y;
  fn z;
} vec;

static vec a;

int main(){
  printf("%x %x %x" ,a.x() ,a.y() ,a.z());
}

static int x(){ return 5;}
static int y(){ return 7;}
static int z(){ return 9;}

// works
// we can create the instance at the top, and then use another instance creation line here at the bottom with an an initializer, and we will get one initialized instance at run time.
/* 
static vec a = {
  .x = x
  ,.y = y
  ,.z = z
};
*/
/*
> gcc init.c 
> ./a.out
5 7 9
*/

// does not work
// we can allocate the instance twice, as shown above, but not three times
///*
static vec a = {
  .x = x
  ,.y = y
};

static vec a = {
  .z = z
};
//*/
/*
> gcc init.c 
init.c:44:12: error: redefinition of ‘a’
   44 | static vec a = {
      |            ^
init.c:39:12: note: previous definition of ‘a’ with type ‘vec’
   39 | static vec a = {
      |            ^
*/
