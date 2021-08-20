/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include<lock.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	if( (optr= &proctab[currpid])->pstate == PRCURR)	{
		if(isempty(rdyhead))
			return OK;
	}
	int pid;
	int greatest_proc_prio, greatest_proc_prio_id;
	for(pid = q[rdyhead].qnext; pid != rdytail; pid = q[pid].qnext){
		greatest_proc_prio_id = pid;
		greatest_proc_prio = 0;
	}

	for(pid = q[rdyhead].qnext; pid != rdytail; pid = q[pid].qnext){
		if(proctab[pid].pprio > greatest_proc_prio && proctab[pid].pinh == 0){
			greatest_proc_prio_id = pid;
			greatest_proc_prio = proctab[greatest_proc_prio_id].pprio;
		}
		if(proctab[pid].pinh > greatest_proc_prio){
			greatest_proc_prio_id = pid;
			greatest_proc_prio = proctab[greatest_proc_prio_id].pinh;
		}
	}

	if(optr->pstate == PRCURR && ((optr->pinh!=0 && greatest_proc_prio_id < optr->pinh) ||
	(optr->pinh == 0 && greatest_proc_prio_id < optr->pprio))){
		return OK;
	}

	/* no switch needed if current process priority higher than next*/
	if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   (lastkey(rdytail)<optr->pprio)) {
		return(OK);
	}

	/* force context switch */
	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest priority process at end of ready list */
	currpid = greatest_proc_prio_id;
	dequeue(greatest_proc_prio_id);
	nptr=&proctab[currpid];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
#endif

	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

	/* The OLD process returns here when resumed. */
	return OK;
}
