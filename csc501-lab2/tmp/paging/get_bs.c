#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages)
{
	STATWORD ps;
	disable(ps);
	if(npages <= 0 || npages > NUM_OF_PAGES || bs_id < 0|| bs_id >= NUM_OF_BACKING_STORES){
		restore(ps);
		return SYSERR;
	}

	if(bsm_tab[bs_id].bs_status == BSM_MAPPED){
		if(bsm_tab[bs_id].bs_private_heap == BSM_PRIVATE_HEAP_TRUE){
			restore(ps);
			return SYSERR;
		}

		if(bsm_tab[bs_id].bs_pid != currpid || bsm_tab[bs_id].bs_pid == currpid){
			bsm_tab[bs_id].bs_pid == currpid;
			bsm_tab[bs_id].bs_npages = npages;
			bsm_tab[bs_id].bs_vpno = 0;
		}
	}

	if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED){
		bsm_tab[bs_id].bs_status = BSM_MAPPED;
		bsm_tab[bs_id].bs_npages = npages;
		bsm_tab[bs_id].bs_vpno = 0;
		bsm_tab[bs_id].bs_pid = currpid;
		bsm_tab[bs_id].bs_sem = -1;
		bsm_tab[bs_id].bs_private_heap = BSM_PRIVATE_HEAP_FALSE;
	}

	restore(ps);
	return npages;
}
