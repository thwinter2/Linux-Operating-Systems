/* lock.c - lock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int writer_has_lock(int lock);
LOCAL int higherpriority_writer_waiting(int lock, int priority);
LOCAL int handle_readrequest(int lock, int priority);
LOCAL int handle_writerequest(int lock, int priority);


/*------------------------------------------------------------------------
 *  * lock  --  make current process wait on a lock
 *   *------------------------------------------------------------------------
 *    */
SYSCALL	lock(int lock, int type, int priority)
{
	STATWORD ps;
	struct	lentry	*lptr;
	struct	pentry	*pptr;

	disable(ps);
	if (isbadlock(lock) || (lptr= &locktab[lock])->lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
	/* if this process has once held the lock or waited for the lock     AND     if this lock has been deleted - raise an error */
	if(((pptr = &proctab[currpid])->plockused[lock] == 1) && lptr->ldeleted == 1){
		restore(ps);
		return(SYSERR);
	}

	/* both waiting and acquiring are considered as using the lock */
	pptr->plockused[lock] = 1;

	if (lptr->lockcnt <= 0) {


		/* handle read request */
		if(type == READ){
			handle_readrequest(lock,priority);
			restore(ps);
			return pptr->pwaitret;
		}

		/* handle write request */
		if(type == WRITE){
			handle_writerequest(lock, priority);
			restore(ps);
			return pptr->pwaitret;
		}
	}

	/* no contention for lock - this process acquires it (first reader/writer) */
	lptr->ltype = type;
	lptr->lockcnt--;
	if(type == READ)
		lptr->lnumreaders = 1;	/* first reader */
	lptr->lacquired[currpid] = 1;
	pptr->plocksheld[lock] = 1;
	restore(ps);
	return(OK);
}

LOCAL int writer_has_lock(int lock){
	struct lentry* lptr = &locktab[lock];
	int i;
	if(lptr->ltype == WRITE){
		for(i=0;i<NPROC; i++){
			if(lptr->lacquired[i])
				return 1;
		}
	}
	return 0;

}

LOCAL int higherpriority_writer_waiting(int lock, int priority){
	struct lentry* lptr;

	if(isempty((lptr = &locktab[lock])->lqhead))
		return 0;


	/* traversing from tail as the queue is sorted on lock priority */
	int prev = q[lptr->lqtail].qprev;
	while(prev != lptr->lqhead){
		if(q[prev].qltype == WRITE && q[prev].qkey >= priority)
		 	return 1;
		prev = q[prev].qprev;
	}
	return 0;

}

LOCAL int handle_readrequest(int lock, int priority){
	struct lentry* lptr = &locktab[lock];
	struct pentry* pptr = &proctab[currpid];
	int i;


	/* if a writer doesn't have the lock and no writer with priority at least as large as the reader's is waiting - reader gets the lock */
	if( !(writer_has_lock(lock)) && !(higherpriority_writer_waiting(lock, priority)) ){
		(lptr->lacquired)[currpid] = 1;
		(lptr->lnumreaders)++;
		pptr->plocksheld[lock] = 1;

	}
	else{
		pptr->pstate = PRWAIT; //process waits
		pptr->plock = lock;
		insertinlqueue(currpid, lptr->lqhead, priority, READ);
		if(schedprio(pptr) > lptr->lprio)
	                lptr->lprio = schedprio(pptr);

		/* waiting triggers priority elevations */
		for(i=0; i< NPROC; i++){
                	if(lptr->lacquired[i])
                		inherit_priorities_for_process(&proctab[i]);
                }

		pptr->pwaitret = OK;
		resched();

	}

	return 1;
}


LOCAL int handle_writerequest(int lock, int priority){
	struct lentry* lptr = &locktab[lock];
        struct pentry* pptr = &proctab[currpid];
	int i;
	pptr->pstate = PRWAIT; //process waits
        pptr->plock = lock;
        insertinlqueue(currpid, lptr->lqhead, priority, WRITE);
	if(schedprio(pptr) > lptr->lprio)
		lptr->lprio = schedprio(pptr);

	/* waiting triggers priority elevations */
        for(i=0; i< NPROC; i++){
        	if(lptr->lacquired[i])
                	inherit_priorities_for_process(&proctab[i]);
        }
        pptr->pwaitret = OK;
        resched();
	return 1;
}

int max_schedprio_among_procs_waiting_for_lock(struct lentry *lptr){
	if(lptr == NULL)
		return -1;
	struct pentry *pptr;
	int max_schedprio = 0;
	int prev = q[lptr->lqtail].qprev;
	if(nonempty(lptr->lqhead)){
		while(prev != lptr->lqhead){
			pptr = &proctab[prev];
			if(schedprio(pptr) > max_schedprio)
				max_schedprio = schedprio(pptr);
			prev = q[prev].qprev;
		}
	}
	return max_schedprio;

}
