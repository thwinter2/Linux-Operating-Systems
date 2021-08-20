#include<stdio.h>
#include<kernel.h>
#include<q.h>
#include<proc.h>
#include<lock.h>

int releaseall(int numlocks, int ldesc1, ...){
	struct pentry *pptr;
	struct lentry *lptr;
	long writing_time = 0, reading_time = 0;
	int lock_released_from_time = 0, read_lock = NULL, write_lock = NULL;
	int i = 0, j, lprio = 0, greatest_prio, lock, lockid, next;

	STATWORD ps;
	disable(ps);

	while(i < numlocks){
		lock = (int)(*(&ldesc1 + i));
		lptr = &locktab[lock];
		if(lock < 0){
			while(lptr->lstate == PRFREE){
				lptr->lstate = READ;
				lptr->reader = lptr->proc[lock];
				lptr->lockqhead = q[lock_released_from_time].qkey;
			}
		}

		lptr->proc[currpid] = 0;
		pptr->held_locks[lock] = 0;

		int j = 0, greatest_prio = -1;
		while(j < NLOCKS){
			if(proctab[currpid].held_locks[j] > 0 && greatest_prio < locktab[j].lprio){
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

		if(lptr->reader > 0){
			lptr->reader--;
		}
		else if(lptr->writer > 0){
			lptr->writer--;
		}

		lockid = q[lptr->lockqtail].qprev;
		lock_released_from_time = 0;

		if(q[lockid].qkey == q[q[lockid].qprev].qkey){
			lprio = q[lockid].qkey;
			while(q[lockid].qkey == lprio){
				if(q[lockid].qlock_wait_time > writing_time && q[lockid].qlock_type == WRITE){
					write_lock = lockid;
				}
				else if(q[lockid].qlock_wait_time > reading_time && q[lockid].qlock_type == READ){
					read_lock = lockid;
				}

				if(write_lock >= 0 && read_lock >= 0){
					if(reading_time > writing_time){
						lock_released_from_time = read_lock;
					}
					else if(reading_time - writing_time <= 1000 || reading_time <= writing_time ||
						writing_time - reading_time <= 1000){		//1 second or 1000 ms
							lock_released_from_time = write_lock;
						}
				}
				lockid = q[lockid].qprev;
			}
			if(lock < 0){
	      while(lptr->lstate == PRFREE){
					lptr->reader = lptr->proc[lock];
					lptr->lockqhead = q[lock_released_from_time].qkey;
					lptr->lstate = READ;
				}
			}
			if(lptr->writer == 0 && q[lock_released_from_time].qlock_type == WRITE){
				if(lptr->reader == 0){
					release(lock, lock_released_from_time);
				}

				j = q[lptr->lockqhead].qnext;
				greatest_prio = -1;
				while(j != lptr->lockqtail){
					if(q[j].qlock_type == WRITE && q[j].qkey > greatest_prio){
						greatest_prio = q[j].qkey;
					}
					j = q[j].qnext;
				}
				j = q[lptr->lockqhead].qnext;
				while(j != lptr->lockqtail){
			    if(q[j].qlock_type == READ && q[j].qkey >= greatest_prio){
						next = q[j].qnext;
			      release(lock, j);
			      j = next;
					}
				}
			}
		}
		if(lptr->writer == 0 && q[lockid].qkey != q[q[lockid].qprev].qkey){
			if(q[lockid].qlock_type == READ){
				j = q[lptr->lockqhead].qnext;
				greatest_prio = -1;
				while(j != lptr->lockqtail){
					if(q[j].qlock_type == WRITE && q[j].qkey > greatest_prio){
						greatest_prio = q[j].qkey;
					}
					j = q[j].qnext;
				}
				j = q[lptr->lockqhead].qnext;
				while(j != lptr->lockqtail){
			    if(q[j].qlock_type == READ && q[j].qkey >= greatest_prio){
						next = q[j].qnext;
			      release(lock, j);
			      j = next;
					}
				}
			}
			if(q[lockid].qlock_type == WRITE)
				if(lptr->reader == 0)
					release(lock,lockid);
		}
		i++;
	}
	resched();
	restore(ps);
	return OK;
}

void release(int ldes1, int node){
	struct lentry *lptr = &locktab[ldes1];
	struct pentry *pptr = &proctab[currpid];
	int j = q[lptr->lockqhead].qnext, greatest_prio = 0;

	pptr->held_locks[ldes1] = 1;
	lptr->proc[node] = 1;

	if(q[node].qlock_type == READ)
		lptr->reader++;
	if(q[node].qlock_type == WRITE){
		lptr->writer++;
	}

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

	dequeue(node);
	ready(node,RESCHNO);
}
