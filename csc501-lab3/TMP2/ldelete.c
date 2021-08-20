#include<stdio.h>
#include<conf.h>
#include<q.h>
#include<kernel.h>
#include<proc.h>
#include<lock.h>

int ldelete(int lockdescriptor){
	struct lentry *lptr = &locktab[lockdescriptor];
	STATWORD ps;
	disable(ps);

	if(lptr->lstate != FREE){
		lptr->lstate = FREE;
		lptr->lprio = -1;
		lptr->proc[currpid] = -1;

		int pid;
		while((pid = getfirst(lptr->lockqhead)) !=EMPTY){
			proctab[pid].lockid = DELETED;
			ready(pid, RESCHNO);
		}
		resched();
		restore(ps);
		return OK;
	}
	else{
		restore(ps);
		return SYSERR;
	}
}
