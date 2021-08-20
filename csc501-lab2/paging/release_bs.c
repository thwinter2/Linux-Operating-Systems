#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {
  STATWORD(ps);
	disable(ps);

  /* release the backing store with ID bs_id */
  if(bs_id < 0 || bs_id >= NUM_OF_BACKING_STORES){
    restore(ps);
    return SYSERR;
  }

  free_bsm(bs_id);
  restore(ps);
  return OK;
}
