/* getprio.c - getprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 * getprio -- return the scheduling priority of a given process
 *------------------------------------------------------------------------
 */
SYSCALL getprio(int pid)
{
	unsigned long syscall_start_time;
  struct pentry *proc = &proctab[currpid];
  extern ctr1000;
	if(syscalls_trace == TRUE){
		syscall_start_time = ctr1000;
		proc->syscall_counts[3] += 1;	//increment count of syscall by 1
	}

	STATWORD ps;
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		if(syscalls_trace == TRUE){
			proc->syscall_times[3] += (ctr1000 - syscall_start_time);
				//add time it took for this syscall to the total time of similar syscalls
		}
		return(SYSERR);
	}
	restore(ps);

	if(syscalls_trace == TRUE){
		proc->syscall_times[3] += (ctr1000 - syscall_start_time);
			//add time it took for this syscall to the total time of similar syscalls
	}
	return(pptr->pprio);
}
