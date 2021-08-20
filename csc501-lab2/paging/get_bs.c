#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {
  STATWORD ps;
	disable(ps);

  /* requests a new mapping of npages with ID map_id */
  if(npages == 0 || npages > NUM_OF_PAGES || bs_id < 0 || bs_id >= NUM_OF_BACKING_STORES){
    if(bsm_tab[bs_id].bs_private_heap == BSM_PRIVATE_HEAP_TRUE){
			restore(ps);
			return SYSERR;
		}
		if(bsm_tab[bs_id].bs_pid != currpid || bsm_tab[bs_id].bs_pid == currpid){
			bsm_tab[bs_id].bs_pid = currpid;
			bsm_tab[bs_id].bs_vpno = 0;
			bsm_tab[bs_id].bs_npages = npages;
		}
  }
  else{
    bsm_tab[bs_id].bs_status = BSM_MAPPED;
    bsm_tab[bs_id].bs_pid = currpid;
  	bsm_tab[bs_id].bs_vpno = 0;
    bsm_tab[bs_id].bs_npages = npages;
    bsm_tab[bs_id].bs_sem = -1;
  }
  restore(ps);
  return npages;
}

    /*restore(ps);
    return SYSERR;  //return system error if # of pages is 0 or backing store ID is not in valid range
  }
  if(bsm_tab[bs_id].bs_status == BSM_MAPPED){
    restore(ps);
    return bsm_tab[bs_id].bs_npages;
  }

  bsm_tab[bs_id].bs_status = BSM_MAPPED;
  bsm_tab[bs_id].bs_pid = currpid;
	bsm_tab[bs_id].bs_vpno = 0;
  bsm_tab[bs_id].bs_npages = npages;
  bsm_tab[bs_id].bs_sem = -1;
  restore(ps);
  return npages;
}*/
