/* releaseall.c - releaseall */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*-----------------------------------------------------------
 * releaseall  --  release lock(s) for the calling  process
 *-----------------------------------------------------------
 */
SYSCALL releaseall (int numlocks, int args)
{
	STATWORD ps;
	struct lentry *lptr;
	struct pentry *pptr = &proctab[currpid];
	int *lockptr;
	int i, lock;
	int retcode = OK;

	disable(ps);

	for(i=0; i< numlocks; i++){
		lockptr = &args + i;
		lock = *lockptr;

		if (isbadlock(lock) || (lptr= &locktab[lock])->lstate==LFREE || !(lptr->lacquired[currpid]) ) {
			retcode = SYSERR;
		}
		else{
			if(lptr->ltype == READ)
				lptr->lnumreaders--;

			/* bookkeeping for locks - released */
			lptr->lacquired[currpid] = 0;
			pptr->plocksheld[lock] = 0;

			/* writer or last reader releases lock, someone else can acquire it */
			if(lptr->ltype == WRITE || ((lptr->ltype == READ) && (lptr->lnumreaders == 0)) ){

				lptr->lockcnt++;

				/* if the lock queue is not empty */
				if( nonempty(lptr->lqhead) ){
					int best_r = get_best_in_lockqueue_by_locktype(lock,READ);
					int best_w = get_best_in_lockqueue_by_locktype(lock,WRITE);

					if(q[best_r].qkey > q[best_w].qkey){
						while(best_r > -1 && (q[best_r].qkey > q[best_w].qkey) ){
							lptr->lacquired[best_r] = 1;
							proctab[best_r].plocksheld[lock] = 1;

							lptr->lnumreaders++;
							lptr->ltype = READ;

							//dequeue the current best reader and make it ready
							ready(dequeue(best_r), RESCHNO);
							lptr->lprio = max_schedprio_among_procs_waiting_for_lock(lptr);

							proctab[best_r].plock = -1; 	/* best_r is no longer waiting in a lock queue */
							best_r = get_best_in_lockqueue_by_locktype(lock,READ);
						}

					}
					else if(q[best_r].qkey < q[best_w].qkey){
                                       		lptr->lacquired[best_w] = 1;
						proctab[best_w].plocksheld[lock] = 1;
                                     		lptr->ltype = WRITE;

						ready(dequeue(best_w), RESCHNO);
					        lptr->lprio = max_schedprio_among_procs_waiting_for_lock(lptr);

                                                proctab[best_w].plock = -1;     /* best_w is no longer waiting in a lock queue */
					}
					/* (ctr1000 - wait start time of writer) - (ctr1000 - wait start time of reader) <= 600 milliseconds */
					else if((q[best_r].qlwstime >= q[best_w].qlwstime) && (q[best_r].qlwstime - q[best_w].qlwstime <= 600 )) {	/* wait priorities for the best reader and best writer are equal */
						lptr->lacquired[best_w] = 1;
						proctab[best_w].plocksheld[lock] = 1;
                                                lptr->ltype = WRITE;

						ready(dequeue(best_w), RESCHNO);
					        lptr->lprio = max_schedprio_among_procs_waiting_for_lock(lptr);

		                                proctab[best_w].plock = -1;     /* best_w is no longer waiting in a lock queue */
					}
					else{								/* wait priorities for the best reader and best writer are equal */
						lptr->lacquired[best_r] = 1;
						proctab[best_r].plocksheld[lock] = 1;
                                                lptr->lnumreaders++;
                                                lptr->ltype = READ;

						ready(dequeue(best_r), RESCHNO);
                                                lptr->lprio = max_schedprio_among_procs_waiting_for_lock(lptr);

                                                proctab[best_r].plock = -1;     /* best_r is no longer waiting in a lock queue */

					}

					resched();
				}

			}

		}

	}

	inherit_priorities_for_process(pptr);



	restore(ps);
	return retcode;
}

void inherit_priorities_for_process(struct pentry * pptr){
	struct lentry *bestlptr = NULL;
	struct lentry *lptr;
	int numlocks = 0;
	int i;

	// find the highest priority lock among the locks
	for(i =0; i < NLOCKS; i++){
		lptr = &locktab[i];
		if(pptr->plocksheld[i]){
			numlocks++;
			if(numlocks == 1 || (bestlptr != NULL && schedprio(pptr) > bestlptr->lprio ) )
				bestlptr = lptr;
		}

	}

	if(!numlocks)		/* all locks released */
		pptr->pinh = 0;
	else{

		pptr->pinh = bestlptr->lprio;
		if(pptr->pprio > pptr->pinh)
			pptr->pinh = 0;

	}
	if(pptr->plock != -1){
		lptr = &locktab[pptr->plock];
		lptr->lprio = max_schedprio_among_procs_waiting_for_lock(lptr);

		for(i=0; i< NPROC; i++){
			if(lptr->lacquired[i])
				inherit_priorities_for_process(&proctab[i]);

		}
	}

}

int get_best_in_lockqueue_by_locktype(int lock, int ltype){
	struct lentry *lptr = &locktab[lock];

	if(isempty(lptr->lqhead))
		return -1;

	int prev = q[lptr->lqtail].qprev;
	while(prev != lptr->lqhead)
	{
		if(q[prev].qltype == ltype)
			return prev;
		prev = q[prev].qprev;
	}

return -1;	// no proc in queue with lock type = ltype
}
