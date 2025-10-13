/*
  Black Box test for Copy.lib.c

*/

#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>

// Pull in the interface portion of the Copy library
#define FACE
#include "Copy.lib.c"
#undef FACE

static sigjmp_buf jump_buffer;

void signal_handler( int sig ){
  siglongjmp( jump_buffer ,1 );
}

typedef bool ( *TestFunction )();
typedef struct{
  TestFunction function;
  const char *name;
} TestEntry;

// -----------------------------------------------------------------------------
// Test Declarations
// -----------------------------------------------------------------------------

bool test_copy_bytes();
bool test_copy_bytes_reverse();
bool test_copy_step();
bool test_copy_step_reverse();

// -----------------------------------------------------------------------------
// Test List and Runner
// -----------------------------------------------------------------------------

TestEntry test_list[] = {
  { test_copy_bytes ,"test_copy_bytes" }
 ,{ test_copy_bytes_reverse ,"test_copy_bytes_reverse" }
 ,{ test_copy_step ,"test_copy_step" }
 ,{ test_copy_step_reverse ,"test_copy_step_reverse" }
 ,{ NULL ,NULL } // terminator
};

int test_head(){
  int pass_count = 0;
  int fail_count = 0;

  // Catch common signals
  signal( SIGSEGV ,signal_handler );
  signal( SIGFPE ,signal_handler );
  signal( SIGABRT ,signal_handler );

  for( TestEntry *entry = test_list ; entry->function != NULL ; entry++ ){
    if( sigsetjmp( jump_buffer ,1 ) == 0 ){
      // Normal path
      if( !entry->function() ){
        printf( "Failed: %s\n" ,entry->name );
        fail_count++;
      }else{
        pass_count++;
      }
    }else{
      // Signal caught
      printf( "Failed due to signal: %s\n" ,entry->name );
      fail_count++;
    }
  }

  printf( "Tests passed: %d\n" ,pass_count );
  printf( "Tests failed: %d\n" ,fail_count );

  return ( fail_count == 0 ) ? 0 : 1;
}

int main( int argc ,char **argv ){
  return test_head();
}

// -----------------------------------------------------------------------------
// Test Definitions
// -----------------------------------------------------------------------------

bool test_copy_bytes(){
  // Test that Copy·bytes() copies data verbatim from src to dst
  uint8_t src[8] = { 0x01 ,0x02 ,0x03 ,0x04 ,0xA0 ,0xB0 ,0xC0 ,0xFF };
  uint8_t dst[8];
  memset( dst ,0x00 ,8 );

  // Copy entire buffer
  Copy·bytes( src ,src + 8 ,dst );

  // Check
  for( int i = 0 ; i < 8 ; i++ ){
    if( dst[i] != src[i] ){
      return false;
    }
  }
  return true;
}

bool test_copy_bytes_reverse(){
  // Test that Copy·bytes_reverse_order() reverses the entire buffer
  uint8_t src[6] = { 'H' ,'e' ,'l' ,'l' ,'o' ,'!' };
  uint8_t dst[6];
  memset( dst ,0x00 ,6 );

  // Reverse copy: Expect "!olleH"
  Copy·bytes_reverse_order( src ,src + 6 ,dst );

  static const uint8_t expected[6] = { '!' ,'o' ,'l' ,'l' ,'e' ,'H' };
  for( int i = 0 ; i < 6 ; i++ ){
    if( dst[i] != expected[i] ){
      return false;
    }
  }
  return true;
}

bool test_copy_step(){
  // Test partial copying with Copy·step()
  // We'll create a source of 10 bytes but allow only 4 bytes of write at a time.
  uint8_t src[10];
  for( int i = 0 ; i < 10 ; i++ ){
    src[i] = (uint8_t)( i + 1 ); // 1..10
  }

  uint8_t dst[10];
  memset( dst ,0 ,10 );

  // Create a Copy·it descriptor
  Copy·it it;
  it.read0 = src;
  it.read_size = 10;
  it.write0 = dst;
  it.write_size = 4;

  // First step: expect incomplete_read (only 4 bytes written)
  {
    Copy·Status st = Copy·step( &it );
    if( st != Copy·Status·incomplete_read ){
      return false;
    }
    // Check that first 4 bytes got copied
    for( int i = 0 ; i < 4 ; i++ ){
      if( dst[i] != (uint8_t)( i + 1 ) ){
        return false;
      }
    }
    // read_size=6, write_size=0, read0 advanced by 4, write0 advanced by 4
    if( it.read_size != 6 || it.write_size != 0 ){
      return false;
    }
  }

  // Provide more write space
  it.write_size = 3;
  // Second step: expect incomplete_read again (only 3 more bytes copied)
  {
    Copy·Status st = Copy·step( &it );
    if( st != Copy·Status·incomplete_read ){
      return false;
    }
    // Check next 3 bytes: now in dst[4..6]
    for( int i = 0 ; i < 3 ; i++ ){
      // i=0 => index=4 => src[4]=5
      if( dst[4 + i] != (uint8_t)( 5 + i ) ){
        return false;
      }
    }
    if( it.read_size != 3 || it.write_size != 0 ){
      return false;
    }
  }

  // Provide final 3 bytes of write space
  it.write_size = 3;
  {
    Copy·Status st = Copy·step( &it );
    // Now we expect either complete or incomplete_write if exactly matched
    // Actually we have 3 left to read, 3 left to write => it should be 'complete'
    if( st != Copy·Status·complete ){
      return false;
    }
    // Check last 3 bytes
    for( int i = 0 ; i < 3 ; i++ ){
      if( dst[7 + i] != (uint8_t)( 8 + i ) ){
        return false;
      }
    }
  }

  // Verify the entire dst array
  for( int i = 0 ; i < 10 ; i++ ){
    if( dst[i] != (uint8_t)( i + 1 ) ){
      return false;
    }
  }
  return true;
}

bool test_copy_step_reverse(){
  // Test partial copying in reverse using Copy·step_reverse_order()

  // Source: 7 bytes => {1,2,3,4,5,6,7}
  uint8_t src[7];
  for( int i = 0 ; i < 7 ; i++ ){
    src[i] = (uint8_t)( i + 1 );
  }

  uint8_t dst[7];
  memset( dst ,0 ,7 );

  Copy·it it;
  it.read0 = src;       // Base of read
  it.read_size = 7;     // 7 bytes total
  it.write0 = dst;
  it.write_size = 4;    // Only 4 bytes can be written initially

  // 1st step: we copy the top 4 bytes in reverse => should be {7,6,5,4}
  {
    Copy·Status st = Copy·step_reverse_order( &it );
    if( st != Copy·Status·incomplete_read ){
      return false;
    }
    // Check that the first 4 reversed bytes ended up in dst
    // We expect: dst[0]=7, dst[1]=6, dst[2]=5, dst[3]=4
    if( dst[0] != 7 || dst[1] != 6 || dst[2] != 5 || dst[3] != 4 ){
      return false;
    }
    // read_size should now be 3, write_size=0
    if( it.read_size != 3 || it.write_size != 0 ){
      return false;
    }
  }

  // Provide 3 more bytes of write space
  it.write_size = 3;
  {
    // 2nd step: copy the remaining 3 bytes in reverse => {3,2,1}
    Copy·Status st = Copy·step_reverse_order( &it );
    // With 3 left to read, 3 left to write => expect complete or incomplete_write
    if( st != Copy·Status·complete ){
      return false;
    }
    // Check we wrote them after the first 4
    if( dst[4] != 3 || dst[5] != 2 || dst[6] != 1 ){
      return false;
    }
    // Now read_size=0, write_size=0
    if( it.read_size != 0 || it.write_size != 0 ){
      return false;
    }
  }

  return true;
}

// for block box testing this goes at the bottom
// for white box testing this goes at the top

#define LOCAL
#include "Copy.lib.c"
#endif
