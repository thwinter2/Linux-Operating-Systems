/* lock.c - lock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int handle_readrequest(int lock, int priority);
LOCAL int handle_writerequest(int lock, int priority);
extern unsigned long ctr1000;

int	lock(int ldes1, int type, int priority){
	struct	pentry	*proc = &proctab[currpid];
	struct	lentry	*lockptr = &locks[ldes1];

	proc->plockused[ldes1] = 1;
	if (lockptr->lockcnt == 0) {
		if(type == READ){
			handle_readrequest(ldes1, priority);
			return proc->pwaitret;
		}
		if(type == WRITE){
			handle_writerequest(ldes1, priority);
			return proc->pwaitret;
		}
	}

	lockptr->ltype = type;
	lockptr->lockcnt -= 1;
	proc->plocksheld[ldes1] = 1;
	lockptr->lacquired[currpid] = 1;
	if(lockptr->ltype == READ){
		lockptr->lnumreaders = 1;
	}
	return OK;
}

LOCAL int writer_has_lock(int lock){
	struct lentry* lockptr = &locks[lock];
	int i;
	if(lockptr->ltype == WRITE){
		for(i=0;i<NPROC; i++){
			if(lockptr->lacquired[i])
				return 1;
		}
	}
	return 0;

}

LOCAL int higherpriority_writer_waiting(int lock, int priority){
	struct lentry* lockptr;

	if(isempty((lockptr = &locks[lock])->lqhead))
		return 0;


	/* traversing from tail as the queue is sorted on lock priority */
	int prev = q[lockptr->lqtail].qprev;
	while(prev != lockptr->lqhead){
		if(q[prev].qlocktype == WRITE && q[prev].qkey >= priority)
		 	return 1;
		prev = q[prev].qprev;
	}
	return 0;

}

int handle_readrequest(int lock, int priority){
	struct lentry* lockptr = &locks[lock];
	struct pentry* proc = &proctab[currpid];
	int i;

	if( !(writer_has_lock(lock)) && !(higherpriority_writer_waiting(lock, priority)) ){
		(lockptr->lacquired)[currpid] = 1;
		(lockptr->lnumreaders)++;
		proc->plocksheld[lock] = 1;

	}
	else{
		proc->pstate = PRWAIT;
		proc->lockid = lock;

		int	next = q[lockptr->lqhead].qnext;
		while (q[next].qkey < priority){
			next = q[next].qnext;
		}
		q[currpid].qnext = next;
		q[currpid].qprev = q[next].qprev;
		q[currpid].qkey  = priority;
		q[currpid].qlocktype = READ;
		q[currpid].qwaittime = ctr1000;
		q[q[next].qprev].qnext = currpid;
		q[next].qprev = currpid;

		if(schedprio(proc) > lockptr->lprio)
	                lockptr->lprio = schedprio(proc);

		/* waiting triggers priority elevations */
		for(i=0; i< NPROC; i++){
    	if(lockptr->lacquired[i])
      	inherit_priorities_for_process(&proctab[i]);
    }
		proc->pwaitret = OK;
		resched();
	}
	return 1;
}


int handle_writerequest(int lock, int priority){
	struct lentry* lockptr = &locks[lock];
  struct pentry* proc = &proctab[currpid];
	int i;
	int	next;
	proc->pstate = PRWAIT;
  proc->lockid = lock;

	for(next = q[lockptr->lqhead].qnext; q[next].qkey < priority; next = q[next].qnext){
		;
	}
	q[currpid].qnext = next;
	q[currpid].qprev = q[next].qprev;
	q[currpid].qkey  = priority;
	q[currpid].qlocktype = WRITE;
	q[currpid].qwaittime = ctr1000;
	q[q[next].qprev].qnext = currpid;
	q[next].qprev = currpid;

	if(schedprio(proc) > lockptr->lprio)
		lockptr->lprio = schedprio(proc);
    for(i=0; i< NPROC; i++){
    	if(lockptr->lacquired[i])
      	inherit_priorities_for_process(&proctab[i]);
    }
    proc->pwaitret = OK;
    resched();
		return 1;
}

int max_schedprio_among_procs_waiting_for_lock(struct lentry *lockptr){
	if(lockptr == NULL)
		return -1;
	struct pentry *proc;
	int max_schedprio = 0;
	int prev = q[lockptr->lqtail].qprev;
	if(nonempty(lockptr->lqhead)){
		while(prev != lockptr->lqhead){
			proc = &proctab[prev];
			if(schedprio(proc) > max_schedprio)
				max_schedprio = schedprio(proc);
			prev = q[prev].qprev;
		}
	}
	return max_schedprio;

}
