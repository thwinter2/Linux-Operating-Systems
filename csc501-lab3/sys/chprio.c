/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;
	struct	pentry	*pptr;
	struct 	lentry	*lptr;
	int	i, old_lprio;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pprio = newprio;

	if(pptr->pinh != 0)
		pptr->pinh = newprio;


	if(pptr->pstate == PRWAIT){
		lptr = &locktab[pptr->plock];
                lptr->lprio = max_schedprio_among_procs_waiting_for_lock(lptr);
		for(i=0; i< NPROC; i++){
				if(lptr->lacquired[i])
					inherit_priorities_for_process(&proctab[i]);
		}

	}
	restore(ps);
	return(newprio);
}
