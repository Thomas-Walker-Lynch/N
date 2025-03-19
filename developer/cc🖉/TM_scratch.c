      // This initializes 'inner'.
      // `alignment` is an extent, e.g. extent_of·AU(uint64_t) = 0x7
      Local TM·Status Ξ(TM·NX ,CVT)·largest_aligned(
        TM·NX·AU *outer ,Ξ(TM·NX ,CVT) *inner ,extent·AU alignment
      ){
        uintptr_t p0 = (uintptr_t)outer->position;
        uintptr_t p1 = (uintptr_t)outer->position + outer->extent;

        CVT *p0_aligned = (CVT *)(
          (p0 + alignment) & ~( (uintptr_t)alignment )
        );
        CVT *p1_aligned = (CVT *)(
          ( p1 - extent_of·AU(CVT) ) & ~( (uintptr_t)alignment )
        );

        if( p1_aligned < p0_aligned ) return TM·Status·derailed;

        inner->position = p0_aligned;
        inner->extent = (Ξ(extent_t ,CVT))(p1_aligned - p0_aligned};
        return TM·Status·on_track;
      }



    
      Local TM·Status TM·NX·topo(TM·NX *tm ,TM·Tape·Topo *topo){
        #ifdef TM·Debug
          TM·Guard·init_count(chk);
          TM·Guard·fg.check(&chk ,1 ,tm ,TM·NX_##CVT·Msg·tm);
          TM·Guard·fg.check(&chk ,1 ,topo ,"topo ptr is NULL, so nowhere to put result");
          TM·Guard·if_return(chk);
        #endif
        if(tm->extent·AU == 0){
          *topo = TM·Tape·Topo·singleton; 
        }else{
          *topo = TM·Tape·Topo·segment;
        }
        return TM·Status·on_track;
      }

      // extent·AU is an AU index
      Local TM·Status TM·NX·extent·AU(TM·NX *tm ,extent·AU *extent·AU){
        TM·Tape·Topo topo;
        TM·Status status = TM·NX_##CVT·topo(tm ,&topo);
        boolean good_topo = 
          (status == TM·Status·on_track) && (topo & TM·Tape·Topo·finite_nz)
          ;

        #ifdef TM·Debug
          TM·Guard·init_count(chk);
          TM·Guard·fg.check(&chk ,1 ,tm ,TM·NX·Msg·tm);
          TM·Guard·fg.check(&chk ,1 ,extent·AU ,TM·NX·Msg·extent·AU);
          TM·Guard·fg.check(
            &chk ,0 ,good_topo
            ,"Tape does not exist or topology does not have an extent·AU."
          );
          TM·Guard·if_return(chk);
        #endif

        if(!good_topo) return TM·Status·derailed;
        *extent·AU = tm->array.extent·AU;
        return TM·Status·on_track;
      }

    #endif
