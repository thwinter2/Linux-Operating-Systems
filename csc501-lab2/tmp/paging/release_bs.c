#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id)
{
	STATWORD ps;
	disable(ps);

	bsm_tab[bs_id].bs_status = BSM_UNMAPPED;
	bsm_tab[bs_id].bs_npages = 0;
	bsm_tab[bs_id].bs_vpno = FIRST_VIRT_PAGE_PROC;
	bsm_tab[bs_id].bs_pid = -1;
	bsm_tab[bs_id].bs_sem = -1;
	bsm_tab[bs_id].bs_private_heap = BSM_PRIVATE_HEAP_FALSE;

	restore(ps);
 	return OK;
}
