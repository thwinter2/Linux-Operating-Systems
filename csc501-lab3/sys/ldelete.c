/* ldelete.c - ldelete */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  * ldelete  --  delete a lock by releasing its table entry
 *   *------------------------------------------------------------------------
 *    */
SYSCALL ldelete(int lock){
	STATWORD ps;
	int	pid, i;
	struct	lentry	*lptr;


	disable(ps);
	if (isbadlock(lock) || locktab[lock].lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
	lptr = &locktab[lock];
	lptr->lstate = LFREE;
	lptr->ldeleted = 1;	/* lock has been deleted */

	/* bookkeeping for locks - lock released and marked DELETED */
	for(i=0; i< NPROC; i++){
		if(lptr->lacquired[i]){
			lptr->lacquired[i] = 0;
			proctab[i].plocksheld[lock] = DELETED;
		}
	}
	/* wake up processes in lock's wait queue */
	if (nonempty(lptr->lqhead)) {
		while( (pid=getfirst(lptr->lqhead)) != EMPTY)
		  {
		    proctab[pid].pwaitret = DELETED;
		    ready(pid,RESCHNO); // no need to call resched each time a proc unblocked; can call when all have been unblocked
		  }
		resched();
	}
	restore(ps);
	return(OK);
}
