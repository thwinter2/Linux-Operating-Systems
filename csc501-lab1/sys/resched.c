/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <math.h>
#include <sched.h>

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
int resched(){
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */
	int current_proc = 0, prev_proc = 0, next_proc = 0;
	double random_value;

	/* Exponential Distribution Scheduler */
	if(sched_type == EXPDISTSCHED){
		random_value = expdev(0.1);		//lambda equals 0.1

		/* Loop through process queue to find process greater than random value */
		current_proc = q[rdytail].qprev;
		prev_proc = q[current_proc].qprev;
		while(random_value < q[prev_proc].qkey){
			if(q[current_proc].qkey != q[prev_proc].qkey){
				current_proc = prev_proc;
			}
			prev_proc = q[prev_proc].qprev;
		}

		/* Set next process as NULLPROC if no other process in ready queue */
		if(current_proc >= NPROC){
			next_proc = NULLPROC;
		}
		else{
			next_proc = current_proc;
		}

		if(((optr= &proctab[currpid])->pstate == PRCURR) && ((next_proc == NULLPROC) || ((optr->pprio > random_value) && (optr->pprio <= q[next_proc].qkey)))) {
			#ifdef	RTCLOCK
				preempt = QUANTUM;		/* reset preemption counter	*/
			#endif
			return OK;
		}

		if(optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);		//ready queue ordered by priority pprio. Tail has highest priority
		}

		if(next_proc >= NPROC){
			currpid = EMPTY;
		}
		else{
			currpid = dequeue(next_proc);
		}

		/* remove highest priority process at end of ready list */
		nptr = &proctab[currpid];	//pointer to new process is set to random_value process
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
		/* The OLD process returns here when resumed. */
		return OK;
	}


	/* LINUX-Like Scheduler */
	else if(sched_type == LINUXSCHED){
		int max_goodness_value = 0, max_goodness_proc = 0, i = 0;
		int new_epoch = TRUE;

		optr = &proctab[currpid];
		optr->goodness_value = optr->goodness_value - optr->counter + preempt;
		optr->counter = preempt;
		if(currpid == NULLPROC || optr->counter <= 0){
			optr->goodness_value = 0;
			optr->counter = 0;
		}
		/* Find process with the greatest goodness value */
		current_proc = q[rdytail].qprev;
		while(current_proc != rdyhead){
			if(proctab[current_proc].goodness_value > max_goodness_value){
				max_goodness_value = proctab[current_proc].goodness_value;
				max_goodness_proc = current_proc;
			}
			current_proc = q[current_proc].qprev;
		}

		/* For a new epoch, set the proccess' counters and goodness values as their priorities */
		if(max_goodness_value == 0 && (optr->pstate != PRCURR || optr->counter == 0)) {
			if(new_epoch){
				struct pentry *proc;
				new_epoch = FALSE;
				for(i = 0; i < NPROC; i++){
					proc = &proctab[i];
					if(proc->pstate != PRFREE){
						proc->counter = ((proc->counter)/2) + proc->pprio;
						proc->goodness_value = proc->counter + proc->pprio;
					}
				}
				preempt = optr->counter;
				optr = &proctab[currpid];
			}

			if(currpid == NULLPROC){
				return OK;
			}

			max_goodness_proc = NULLPROC;
			if (optr->pstate == PRCURR) {
				optr->pstate = PRREADY;
				insert(currpid,rdyhead,optr->pprio);
			}
			nptr = &proctab[max_goodness_proc];
			nptr->pstate = PRCURR;
			currpid = dequeue(max_goodness_proc);

			#ifdef	RTCLOCK
				preempt = QUANTUM;		/* reset preemption counter	*/
			#endif
			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
			/* The OLD process returns here when resumed. */
			return OK;
		}

		/* If goodness values are set and nonzero, set new process and dequeue it from ready queue */
		else if(max_goodness_value > 0 && (optr->counter == 0 || optr->goodness_value < max_goodness_value || optr->pstate != PRCURR)){
			if (optr->pstate == PRCURR) {
				optr->pstate = PRREADY;
				insert(currpid,rdyhead,optr->pprio);
			}
			/* remove highest goodness process at end of ready list */
			nptr = &proctab[max_goodness_proc];
			nptr->pstate = PRCURR;
			currpid = dequeue(max_goodness_proc);
			preempt = nptr->counter;

			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
			/* The OLD process returns here when resumed. */
			return OK;
		}

		/* If current process has the greatest goodness value, no need to context switch */
		else if(optr->pstate == PRCURR && optr->goodness_value >= max_goodness_value && optr->goodness_value > 0){
			preempt = optr->counter;
			return OK;
		}

		/* force context switch */
		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}
		/* remove highest priority process at end of ready list */
		nptr = &proctab[currpid = getlast(rdytail)];
		nptr->pstate = PRCURR;
		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
		/* The OLD process returns here when resumed. */
		return OK;
	}


	/* XINU Scheduler */
	else{
		/* no switch needed if current process priority higher than next*/
		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
		   (lastkey(rdytail)<optr->pprio)) {
			return OK;
		}

		/* force context switch */
		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}

		/* remove highest priority process at end of ready list */
		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif

		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

		/* The OLD process returns here when resumed. */
		return OK;
	}
}
