#include<conf.h>
#include<kernel.h>
#include<stdio.h>
#include<q.h>
#include<proc.h>
#include<lock.h>

int lock(int ldes1, int type, int priority){
	STATWORD ps;
	disable(ps);
	struct lentry *lptr = &locktab[ldes1];
  struct pentry *pptr = &proctab[currpid];

	if(lptr->lstate == FREE){
		restore(ps);
		return SYSERR;
	}

	int i;
	Bool reader_writer = FALSE;
	if(ldes1 < 0 && lptr->lstate == PRFREE){
		lptr->reader = lptr->proc[ldes1];
		lptr->lstate = READ;
	}

	if((lptr->reader == 0 && lptr->writer != 0) || (lptr->writer == 0 && type == WRITE && lptr->reader != 0))
		reader_writer = TRUE;
	if(type == READ && lptr->writer==0 && lptr->reader!=0){
		int next;
		for(next = q[lptr->lockqhead].qnext; priority < q[next].qkey; next = q[next].qnext){
			if(q[next].qlock_type == WRITE){
				reader_writer = TRUE;
			}
		}
	}

	if(reader_writer){
		pptr->pstate = PRLOCK;
		pptr->lproc = ldes1;
		insert(currpid, lptr->lockqhead, priority);
		q[currpid].qlock_wait_time = ctr1000;
		q[currpid].qlock_type = type;

		int j = q[lptr->lockqhead].qnext;
		int greatest_prio = 0;
		while(j != lptr->lockqtail){
			if(proctab[j].pprio > greatest_prio){
				greatest_prio = proctab[j].pprio;
			}
			j = q[j].qnext;
		}
		lptr->lprio = greatest_prio;


		i = 0;
		while(i < NPROC){
			if(lptr->proc[i] > 0){
				j = 0;
				int greatest_prio = -1;
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

		resched();
		restore(ps);
		return pptr->lockid;
	}
	else{
		proctab[currpid].held_locks[ldes1] = 1;
		lptr->proc[currpid] = 1;

		int j = 0, greatest_prio = -1;
		while(j < NLOCKS){
			if(proctab[currpid].held_locks[i] > 0 && greatest_prio < locktab[j].lprio){
				greatest_prio = locktab[j].lprio;
			}
			j++;
		}
		if(proctab[currpid].pprio <= greatest_prio){
			proctab[currpid].pinh = greatest_prio;
		}
		else{
			proctab[currpid].pinh = 0;
		}

		if(type == WRITE){
			lptr->writer += 1;
		}
		if(type == READ){
			lptr->reader += 1;
		}
		restore(ps);
		return OK;
	}
	restore(ps);
	return OK;
}
