
# gt_extent_type is large enough to hold a sizeof(NS·T)-1 or sizeof(TYPE)-1
def conversion(type: str):
  """
  Returns a source code file for cc to munch on
  """
  template = template_conversion()
  code = template.replace("TYPE", type)
  return code

# The Digit array is always least significant digit at d[0]
def template_conversion():
    return r'''

// NS·T -> PNT, possible NS·T leftovers
NS·Status NS·to_TYPE
 (
  const NS·T *source
  ,TYPE *destination
  ,NS·Leftover_N *leftover
  ){

  uint8_t *s = (uint8_t *)source;
  uint8_t *d = (uint8_t *)destination;

  if( sizeof(NS·T) <= sizeof(TYPE) ){
    *destination = 0;
    memcpy( d, s, sizeof(NS·T) );
    return NS·Status·ok;
  }

  memcpy( destination ,s ,sizeof(TYPE) );
  s += sizeof(TYPE);
  const size_t δ = sizeof(NS·T) - sizeof(TYPE);

  if( memcmp(NS·constant + 0 ,s ,δ) == 0 ){
    return NS·Status·ok;
  }
  // else there are leftovers
  
  if( leftover == NULL || leftover->d == NULL){ // then nowhere to put the leftovers
    return NS·Status·ConversionOverflow;
  }

  // copy the part leftover into the leftover struct
  leftover->scale = sizeof(TYPE);
  uint8_t *lod = (uint8_t *)(leftover->d);
  memcpy(lod ,s ,δ);

  // zero out the upper bytes of the leftover struct
  lod += δ;
  memset(lod, 0, sizeof(NS·T) - δ);  // Zero remaining bytes safely

  return NS·Status·ConversionOverflow;
}

// PNT -> NS·T, possible PNT leftovers
NS·Status NS·from_TYPE
 (
  const TYPE *s // source
  ,NS·T *destination
  ,NS·Leftover_PNT *leftover
  ){
 
  uint8_t *d = (uint8_t *)destination; // NS·T

  // source allocation smaller: copy then zero out the top of destination
  if( sizeof(TYPE) <= sizeof(NS·T)  ){
    memcpy( d, s, sizeof(TYPE) );
    d += sizeof(TYPE);
    memset( d, 0, sizeof(NS·T) - sizeof(TYPE) );  // Zero out remaining bytes
    return NS·Status·ok;
  }

  // The source allocation is larger: copy then potentially spill to leftovers

  memcpy( d, s, sizeof(NS·T) );

  TYPE mask = ~(TYPE)0 << (sizeof(NS·T) << 3);
  TYPE unscaled_leftover = *source & mask;

  if(unscaled_leftover == 0) return NS·Status·ok;

  if(leftover == NULL){ // then nowhere to put the leftovers
    return NS·Status·ConversionOverflow;
  }
  
  leftover->scale = sizeof(NS·T);
  leftover->leftover = unscaled_leftover >> (sizeof(NS·T) << 3);

  return NS·Status·ConversionOverflow;

}
'''
