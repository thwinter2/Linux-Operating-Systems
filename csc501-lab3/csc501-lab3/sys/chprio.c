/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include<lock.h>
/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pprio = newprio;
	if(pptr->pstate==PRREADY)
	{
		dequeue(pid);
		insert(pid,rdyhead,pptr->pprio);
	}
	if(isbadpid(pid)||newprio<1||(pptr->pstate==PRFREE))
	{
		restore(ps);
		return OK;
	}

	int j = 0, greatest_prio = -1;
	while(j < NLOCKS){
		if(proctab[pid].held_locks[j] > 0 && greatest_prio < locktab[j].lprio){
			greatest_prio = locktab[j].lprio;
		}
		j++;
	}
	if(proctab[pid].pprio <= greatest_prio){
		proctab[pid].pinh = greatest_prio;
	}
	else{
		proctab[pid].pinh = 0;
	}

	while(pptr->pstate==PRLOCK){
		struct lentry *lptr = &locktab[pptr->lproc];
		j = q[lptr->lockqhead].qnext;
		int greatest_prio = 0;
		while(j != lptr->lockqtail){
			if(proctab[j].pprio > greatest_prio){
				greatest_prio = proctab[j].pprio;
			}
			j = q[j].qnext;
		}
		lptr->lprio = greatest_prio;

		int proc_id = pptr->lproc;
		j = 0, greatest_prio = -1;
		while(j < NLOCKS){
			if(proctab[proc_id].held_locks[j] > 0 && greatest_prio < locktab[j].lprio){
				greatest_prio = locktab[j].lprio;
			}
			j++;
		}
		if(proctab[proc_id].pprio <= greatest_prio){
			proctab[proc_id].pinh = greatest_prio;
		}
		else{
			proctab[proc_id].pinh = 0;
		}
		break;
	}
	restore(ps);
	return(newprio);
}
