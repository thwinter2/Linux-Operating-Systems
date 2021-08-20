/* sleep.c - sleep */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 * sleep  --  delay the calling process n seconds
 *------------------------------------------------------------------------
 */
SYSCALL	sleep(int n)
{
	unsigned long syscall_start_time;
  struct pentry *proc = &proctab[currpid];
  extern ctr1000;
	if(syscalls_trace == TRUE){     
    syscall_start_time = ctr1000;
		proc->syscall_counts[18] += 1;	//increment count of syscall by 1
	}

	STATWORD ps;
	if (n<0 || clkruns==0){
		if(syscalls_trace == TRUE){
			proc->syscall_times[18] += (ctr1000 - syscall_start_time);
				//add time it took for this syscall to the total time of similar syscalls
		}
		return(SYSERR);
	}
	if (n == 0) {
	        disable(ps);
		resched();
		restore(ps);
		if(syscalls_trace == TRUE){
			proc->syscall_times[18] += (ctr1000 - syscall_start_time);
				//add time it took for this syscall to the total time of similar syscalls
		}
		return(OK);
	}
	while (n >= 1000) {
		sleep10(10000);
		n -= 1000;
	}
	if (n > 0)
		sleep10(10*n);
		if(syscalls_trace == TRUE){
			proc->syscall_times[18] += (ctr1000 - syscall_start_time);
				//add time it took for this syscall to the total time of similar syscalls
		}
	return(OK);
}
