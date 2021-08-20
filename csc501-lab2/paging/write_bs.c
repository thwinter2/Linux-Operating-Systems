#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <mark.h>
#include <bufpool.h>
#include <paging.h>

int write_bs(char *src, bsd_t bs_id, int page) {
  STATWORD(ps);
	disable(ps);

  /* write one page of data from src
     to the backing store bs_id, page
     page.
  */

  if(bs_id < 0 || bs_id > 7 || page < 0 || page > (NUM_OF_PAGES - 1)){   //Error if ID or page number is out of range
    restore(ps);
    return SYSERR;
  }

  char * phy_addr = BACKING_STORE_BASE + bs_id*BACKING_STORE_UNIT_SIZE + page*NBPG;
  bcopy((void*)src, phy_addr, NBPG);
  restore(ps);
  return OK;
}
