/* sleep1000.c - sleep1000 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include <lab0.h>
#include <i386.h>

/*------------------------------------------------------------------------
 * sleep1000 --  delay the caller for a time specified in 1/100 of seconds
 *------------------------------------------------------------------------
 */
SYSCALL sleep1000(int n)
{
  unsigned long syscall_start_time;
  struct pentry *proc = &proctab[currpid];
  extern ctr1000;
	if(syscalls_trace == TRUE){
		syscall_start_time = ctr1000;
		proc->syscall_counts[21] += 1;	//increment count of syscall by 1
	}

	STATWORD ps;

	if (n < 0  || clkruns==0){
		if(syscalls_trace == TRUE){
			proc->syscall_times[21] += (ctr1000 - syscall_start_time);
				//add time it took for this syscall to the total time of similar syscalls
		}
	  return(SYSERR);
	}
	disable(ps);
	if (n == 0) {		/* sleep1000(0) -> end time slice */
	        ;
	} else {
		insertd(currpid,clockq,n);
		slnempty = TRUE;
		sltop = &q[q[clockq].qnext].qkey;
		proctab[currpid].pstate = PRSLEEP;
	}
	resched();
        restore(ps);

	if(syscalls_trace == TRUE){
		proc->syscall_times[21] += (ctr1000 - syscall_start_time);
			//add time it took for this syscall to the total time of similar syscalls
	}

	return(OK);
}
