/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	STATWORD(ps);
	disable(ps);
	int i = 0;
	for(i; i < NUM_OF_BACKING_STORES ;i++)
	{
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = -1;
		bsm_tab[i].bs_vpno = FIRST_VIRT_PAGE_PROC;
		bsm_tab[i].bs_npages = 0;
		bsm_tab[i].bs_sem = -1;
		bsm_tab[i].bs_private_heap = BSM_PRIVATE_HEAP_FALSE;
	}
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	STATWORD(ps);
	disable(ps);
	int i = 0;
	for(i; i < NUM_OF_BACKING_STORES; i++){
		if(bsm_tab[i].bs_status == BSM_UNMAPPED){
			*avail = i;
			restore(ps);
			return OK;
		}
	}
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	STATWORD(ps);
	disable(ps);
	bsm_tab[i].bs_status = BSM_UNMAPPED;
  bsm_tab[i].bs_pid = -1;
  bsm_tab[i].bs_vpno = FIRST_VIRT_PAGE_PROC;
  bsm_tab[i].bs_npages = 0;
  bsm_tab[i].bs_sem = -1;
  bsm_tab[i].bs_private_heap = 0;
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	STATWORD(ps);
	disable(ps);
	int i;
	int page = vaddr / NBPG;

	for(i = 0;i < NUM_OF_BACKING_STORES; i++){
		if(bsm_tab[i].bs_pid==pid){
			if(page < bsm_tab[i].bs_vpno || page > bsm_tab[i].bs_vpno+bsm_tab[i].bs_npages){
				restore(ps);
				return SYSERR;
			}

		*pageth = page - bsm_tab[i].bs_vpno;
		*store=i;
		restore(ps);
		return OK;
		}
	}
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	STATWORD(ps);
	disable(ps);
	if(npages <= 0 || npages > NUM_OF_PAGES || source < 0 || source > NUM_OF_BACKING_STORES){
		restore(ps);
		return SYSERR;
	}

	proctab[currpid].vhpno = vpno;
	proctab[currpid].store = source;
	bsm_tab[source].bs_status = BSM_MAPPED;
	bsm_tab[source].bs_npages = npages;
	bsm_tab[source].bs_vpno = vpno;
	bsm_tab[source].bs_pid = 1;
	bsm_tab[source].bs_sem = 1;
  bsm_tab[source].bs_private_heap = 0;

  restore(ps);
  return OK;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	STATWORD ps;
	disable(ps);
	int i = 0;
	int page, bsm_num;
	unsigned long vaddr;

	vaddr = vpno*NBPG;
	while(i < NFRAMES){
		if(frm_tab[i].fr_type == FR_PAGE && frm_tab[i].fr_pid == pid){
			bsm_lookup(pid, vaddr, &bsm_num, &page);
			write_bs((i + NFRAMES) * NBPG, bsm_num, page);
  		}
		i++;
	}

	bsm_tab[proctab[pid].store].bs_status = BSM_UNMAPPED;
	bsm_tab[proctab[pid].store].bs_vpno = vpno;
	bsm_tab[proctab[pid].store].bs_npages = 0;
	bsm_tab[proctab[pid].store].bs_pid = -1;
	bsm_tab[proctab[pid].store].bs_sem = -1;
	bsm_tab[proctab[pid].store].bs_private_heap = 0;
	restore(ps);
	return OK;
}
