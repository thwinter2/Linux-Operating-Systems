#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int newlock();

/*------------------------------------------------------------------------
 *  * lcreate  --  create and initialize a lock, returning its id
 *   *------------------------------------------------------------------------
 *    */
SYSCALL lcreate(void){
	STATWORD ps;
	int i, lock;

	disable(ps);
	if ( (lock=newlock())==SYSERR ) {
		restore(ps);
		return(SYSERR);
	}
	locktab[lock].lockcnt = 1;
	locktab[lock].lnumreaders = 0;
	locktab[lock].lprio = 0;

	for(i=0; i<NPROC; i++)
		locktab[lock].lacquired[i] = 0;

	/* lqhead and lqtail were initialized at system startup */
	restore(ps);
	return(lock);
}

/*------------------------------------------------------------------------
 *  * newlock  --  allocate an unused lock and return its index
 *   *------------------------------------------------------------------------
 *    */
LOCAL int newlock()
{
	int	lock;
	int	i;

	for (i=0 ; i<NLOCKS ; i++) {
		lock=nextlock--;
		if (nextlock < 0)
			nextlock = NLOCKS-1;
		if (locktab[lock].lstate==LFREE) {

			/* initialize ldeleted to 0 only the first time it is created - once a lock is deleted it should remain  marked as deleted throughout program execution */
			if(locktab[lock].ldeleted != 1)
				locktab[lock].ldeleted = 0;
			locktab[lock].lstate = LUSED;
			return(lock);
		}
	}
	return(SYSERR);
}
