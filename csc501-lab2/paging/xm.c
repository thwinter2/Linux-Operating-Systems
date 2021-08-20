/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  STATWORD(ps);
	disable(ps);
  if(virtpage > FIRST_VIRT_PAGE_PROC){
    bsm_map(currpid, virtpage, source, npages);
    restore(ps);
    return OK;
  }
  restore(ps);
  return SYSERR;
}

/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
  STATWORD(ps);
	disable(ps);
  if(virtpage > FIRST_VIRT_PAGE_PROC){
    bsm_unmap(currpid, virtpage, 1);
    restore(ps);
    return OK;
  }
  restore(ps);
  return SYSERR;
}
