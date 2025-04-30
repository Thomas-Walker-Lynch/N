#include <stdio.h>

int main() {
  int xコ = 5;
  int ㄈx = 7;
  printf("x: %x %x" ,xコ ,ㄈx);
  return 0;
}

/*
2025-04-25T11:25:54Z[developer]
Thomas-developer@StanleyPark§/home/Thomas/subu_data/developer/N/developer/experiment§
> ./a.out
x: 5 7
*/

/*
Identifier	Character(s)	Unicode Category	Allowed?	Font visible?
xコ	Katakana KO	Lo (Letter, Other)	✅ Yes	If font supports Katakana
ㄈx	Bopomofo FA	Lo (Letter, Other)	✅ Yes	Often not visible unless font supports Zhuyin
*/
