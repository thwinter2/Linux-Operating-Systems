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
int releaseall (int numlocks, int args){
	struct lentry *lptr;
	struct pentry *pptr = &proctab[currpid];
	int *lockptr;
	int i = 0, lock;
	int retcode = OK;

	for(i; i < numlocks; i++){
		lockptr = &args + i;
		lock = *lockptr;

		if((lptr= &locks[lock])->lstate==LFREE || !(lptr->lacquired[currpid])) {
			retcode = SYSERR;
		}

		else{
			if(lptr->ltype == READ)
				lptr->lnumreaders--;
			lptr->lacquired[currpid] = 0;
			pptr->plocksheld[lock] = 0;

			if(lptr->ltype == WRITE || ((lptr->ltype == READ) && (lptr->lnumreaders == 0)) ){
				lptr->lockcnt++;

				if(nonempty(lptr->lqhead) ){
					int best_r = get_best_in_lockqueue_by_locktype(lock,READ);
					int best_w = get_best_in_lockqueue_by_locktype(lock,WRITE);

					if(q[best_r].qkey > q[best_w].qkey){
						while(best_r > -1 && (q[best_r].qkey > q[best_w].qkey) ){
							lptr->lacquired[best_r] = 1;
							proctab[best_r].plocksheld[lock] = 1;

							lptr->lnumreaders++;
							lptr->ltype = READ;

							ready(dequeue(best_r), RESCHNO);
							lptr->lprio = max_schedprio_among_procs_waiting_for_lock(lptr);
							proctab[best_r].lockid = -1;
							best_r = get_best_in_lockqueue_by_locktype(lock,READ);
						}
					}

					else if(q[best_r].qkey < q[best_w].qkey){
            lptr->lacquired[best_w] = 1;
						proctab[best_w].plocksheld[lock] = 1;
            lptr->ltype = WRITE;
						ready(dequeue(best_w), RESCHNO);
					  lptr->lprio = max_schedprio_among_procs_waiting_for_lock(lptr);
						proctab[best_w].lockid = -1;
					}

					else if((q[best_r].qwaittime >= q[best_w].qwaittime) && (q[best_r].qwaittime - q[best_w].qwaittime <= 600 )) {
						lptr->lacquired[best_w] = 1;
						proctab[best_w].plocksheld[lock] = 1;
            lptr->ltype = WRITE;
						ready(dequeue(best_w), RESCHNO);
					  lptr->lprio = max_schedprio_among_procs_waiting_for_lock(lptr);
						proctab[best_w].lockid = -1;
					}

					else{
						lptr->lacquired[best_r] = 1;
						proctab[best_r].plocksheld[lock] = 1;
            lptr->lnumreaders++;
            lptr->ltype = READ;
						ready(dequeue(best_r), RESCHNO);
            lptr->lprio = max_schedprio_among_procs_waiting_for_lock(lptr);
						proctab[best_r].lockid = -1;
					}
					resched();
				}
			}
		}
	}

	inherit_priorities_for_process(pptr);
	return retcode;
}

void inherit_priorities_for_process(struct pentry * pptr){
	struct lentry *bestlptr = NULL;
	struct lentry *lptr;
	int numlocks = 0;
	int i = 0;

	for(i; i < NLOCKS; i++){
		lptr = &locks[i];
		if(pptr->plocksheld[i]){
			numlocks++;
			if(numlocks == 1 || (bestlptr != NULL && schedprio(pptr) > bestlptr->lprio ) )
				bestlptr = lptr;
		}
	}

	if(!numlocks)
		pptr->pinh = 0;
	else{
		pptr->pinh = bestlptr->lprio;
		if(pptr->pprio > pptr->pinh)
			pptr->pinh = 0;
	}

	if(pptr->lockid != -1){
		lptr = &locks[pptr->lockid];
		lptr->lprio = max_schedprio_among_procs_waiting_for_lock(lptr);

		for(i=0; i< NPROC; i++){
			if(lptr->lacquired[i])
				inherit_priorities_for_process(&proctab[i]);
		}
	}
}

int get_best_in_lockqueue_by_locktype(int lock, int ltype){
	struct lentry *lptr = &locks[lock];

	if(isempty(lptr->lqhead))
		return -1;

	int prev = q[lptr->lqtail].qprev;
	while(prev != lptr->lqhead){
		if(q[prev].qlocktype == ltype)
			return prev;
		prev = q[prev].qprev;
	}
	return -1;
}
