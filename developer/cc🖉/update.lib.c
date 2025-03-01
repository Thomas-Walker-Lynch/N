/*
  Copy - Memory copy operations with attention to alignment.
  Provides optimized copy and byte order reversal functions.
*/

#define Copy·DEBUG

#ifndef FACE
#define Copy·IMPLEMENTATION
#define FACE
#endif 

//--------------------------------------------------------------------------------
// Interface

#ifndef Copy·FACE
#define Copy·FACE

  #include <stdint.h>
  #include <stddef.h>

  typedef struct{
    void *read0 
    ,size_t read_size
    ,void *write0
    ,size_t write_size;
  } Copy·it;

  typedef enum{
     Copy·Status·perfect_fit = 0
    ,Copy·Status·argument_guard
    ,Copy·Status·read_surplus
    ,Copy·Status·read_surplus_write_gap
    ,Copy·Status·write_available
    ,Copy·Status·write_gap;
  } Copy·Status;

  typedef enum{
    Copy·WFIt·Mode·none = 0
    ,Copy·WFIt·Mode·bytes
    ,Copy·WFIt·Mode·bytes_reverse
    ,Copy·WFIt·Mode·write_hex
    ,Copy·WFIt·Mode·read_hex;
  } Copy·WFIt·Mode;

  typedef enum{
    Copy·WFIt·Status·valid = 0
    ,Copy·WFIt·Status·null_read
    ,Copy·WFIt·Status·null_write
    ,Copy·WFIt·Status·zero_buffer
    ,Copy·WFIt·Status·overlap
    ,Copy·WFIt·Status·write_too_small;
  } Copy·WFIt·Status;

  typedef struct{
    void *region( void *read0 ,void *read1 ,void *write0 )
    ,void *reverse_byte_order( void *read0 ,void *read1 ,void *write0 );
  } Copy·M;

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef Copy·IMPLEMENTATION

  // this part goes into Nlib.a
  #ifndef LOCAL
  #endif 

  #ifdef LOCAL

    Local Copy·WFIt·Status Copy·wellformed_it(Copy·it *it){
      char *this_name = "Copy·wellformed_it";
      Copy·WFIt·Status status = Copy·WFIt·Status·valid;

      if(it->read0 == NULL){
        fprintf( stderr ,"%s: NULL read pointer\n" ,this_name );
        status |= Copy·WFIt·Status·null_read;
      }

      if(it->write0 == NULL){
        fprintf( stderr ,"%s: NULL write pointer\n" ,this_name );
        status |= Copy·WFIt·Status·null_write;
      }

      if(it->read_size == 0){
        fprintf( stderr ,"%s: Zero-sized read buffer\n" ,this_name );
        status |= Copy·WFIt·Status·zero_read_buffer;
      }

      if(it->write_size == 0){
        fprintf( stderr ,"%s: Zero-sized write buffer\n" ,this_name );
        status |= Copy·WFIt·Status·zero_write_buffer;
      }

      if( Copy·overlap_size_interval(it->read0 ,it->read_size ,it->write0 ,it->write_size) ){
        fprintf( stderr ,"%s: Read and write buffers overlap!\n" ,this_name );
        status |= Copy·WFIt·Status·overlap;
      }

      return status;
    }

  #endif // LOCAL

#endif // IMPLEMENTATION
