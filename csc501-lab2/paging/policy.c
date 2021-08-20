/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>


extern int page_replace_policy;
/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
  STATWORD ps;
	disable(ps);
  /* sanity check ! */

  if(policy == SC || policy == AGING){
    page_replace_policy = policy;
    restore(ps);
    return OK;
  }
  restore(ps);
  return SYSERR;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
  return page_replace_policy;
}
