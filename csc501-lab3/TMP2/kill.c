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
	//disable(ps);
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev,i,temp;
	struct lentry *lptr;

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

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
							/* fall through	*/
	case PRLOCK: lptr = &locktab[pptr->lproc];
			dequeue(pid);
			locktab[pptr->lproc].proc[pid] = 0;

			int j = q[lptr->lockqhead].qnext;
			int greatest_prio = 0;
			while(j != lptr->lockqtail){
				if(proctab[j].pprio > greatest_prio){
					greatest_prio = proctab[j].pprio;
				}
				j = q[j].qnext;
			}
			lptr->lprio = greatest_prio;

			int i = 0;
			while(i < NPROC){
				if(lptr->proc[i]>0){
					j = 0, greatest_prio = -1;
					while(j < NLOCKS){
						if(proctab[i].held_locks[j] > 0 && greatest_prio < locktab[j].lprio){
							greatest_prio = locktab[j].lprio;
						}
						j++;
					}
					if(proctab[i].pprio <= greatest_prio){
						proctab[i].pinh = greatest_prio;
					}
					else{
						proctab[i].pinh = 0;
					}
				}
				i++;
			}
			pptr->pstate = PRFREE;
			break;

	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
