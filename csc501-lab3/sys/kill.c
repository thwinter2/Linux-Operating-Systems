/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	struct 	lentry	*lptr;
	int	dev;
	int 	i;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);

	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;
			lptr = &locktab[pptr->plock];

			/* increment the lock count if it is a writer or last reader */
			if(lptr->ltype == WRITE || ((lptr->ltype == READ) && (lptr->lnumreaders == 0)) )
                                lptr->lockcnt++;
			if(schedprio(pptr) == lptr->lprio){
				dequeue(pid);
				lptr->lprio = max_schedprio_among_procs_waiting_for_lock(lptr);

				for(i=0; i< NPROC; i++){
					if(lptr->lacquired[i])
						inherit_priorities_for_process(&proctab[i]);
				}
			}

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
