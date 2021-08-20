/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
  STATWORD(ps);
	disable(ps);
  int i = 0;

  for(i; i < NUM_OF_BACKING_STORES; i++){
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
    if(bsm_tab[i].bs_status = BSM_UNMAPPED){
      *avail = i;
      restore(ps);
      return OK;
    }
  }
  restore(ps);
  return SYSERR;  //No free entry
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
  bsm_tab[i].bs_private_heap = BSM_PRIVATE_HEAP_FALSE;

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
  int bsm_num, page;
  page = vaddr / NBPG;

  if(isbadpid(pid)){
    restore(ps);
    return SYSERR;
  }

  for(bsm_num = 0; bsm_num < NUM_OF_BACKING_STORES; bsm_num++){
    if(bsm_tab[bsm_num].bs_pid == pid){
      if((page < bsm_tab[bsm_num].bs_vpno) || (page > (bsm_tab[bsm_num].bs_vpno + bsm_tab[bsm_num].bs_npages))){  //Error if page is outside of first and last page
        restore(ps);
        return SYSERR;
      }

      *store = bsm_num;
      *pageth = page - bsm_tab[bsm_num].bs_vpno;
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
  if(bsm_tab[source].bs_private_heap == BSM_PRIVATE_HEAP_TRUE || isbadpid(pid) || source < 0 || source >= NUM_OF_BACKING_STORES || npages <= 0 || npages > NUM_OF_PAGES){
    restore(ps);
    return SYSERR;
  }

  proctab[currpid].vhpno = vpno;
  proctab[currpid].store = source;
  bsm_tab[source].bs_status = BSM_MAPPED;
  bsm_tab[source].bs_pid = pid;
  bsm_tab[source].bs_vpno = vpno;
  bsm_tab[source].bs_npages = npages;
  bsm_tab[source].bs_sem = 1;
  bsm_tab[source].bs_private_heap = BSM_PRIVATE_HEAP_FALSE;

  restore(ps);
  return OK;
}


/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
  STATWORD(ps);
	disable(ps);
  /*if(isbadpid(pid)){
    restore(ps);
    return SYSERR;
  }
  bsd_t back_store_id = proctab[pid].store;

  bsm_tab[back_store_id].bs_status = BSM_UNMAPPED;
  bsm_tab[back_store_id].bs_pid = -1;
  bsm_tab[back_store_id].bs_vpno = -1;
  bsm_tab[back_store_id].bs_npages = 0;
  bsm_tab[back_store_id].bs_sem = -1;
  bsm_tab[back_store_id].bs_private_heap = BSM_PRIVATE_HEAP_FALSE;
*/
int i = 0,bs_number,pageth;
unsigned long vaddr = vpno*NBPG;

  while(i < NFRAMES){
    if(frm_tab[i].fr_pid == pid && frm_tab[i].fr_type == FR_PAGE){
      bsm_lookup(pid,vaddr,&bs_number,&pageth);
      write_bs( (i+NFRAMES)*NBPG, bs_number, pageth);
      }
      i++;
  }

  bsm_tab[proctab[pid].store].bs_status = BSM_UNMAPPED;
  bsm_tab[proctab[pid].store].bs_pid = -1;
  bsm_tab[proctab[pid].store].bs_sem = -1;
  bsm_tab[proctab[pid].store].bs_npages = 0;
  bsm_tab[proctab[pid].store].bs_vpno = vpno;
  bsm_tab[proctab[pid].store].bs_private_heap = 0;
  restore(ps);
  return OK;
}
