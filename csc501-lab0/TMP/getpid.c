/* getpid.c - getpid */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 * getpid  --  get the process id of currently executing process
 *------------------------------------------------------------------------
 */
SYSCALL getpid()
{
	unsigned long syscall_start_time;
  struct pentry *proc = &proctab[currpid];
  extern ctr1000;
	if(syscalls_trace == TRUE){
		syscall_start_time = ctr1000;
		proc->syscall_counts[2] += 1;	//increment count of syscall by 1
	}

	if(syscalls_trace == TRUE){
		proc->syscall_times[2] += (ctr1000 - syscall_start_time);
			//add time it took for this syscall to the total time of similar syscalls
	}
	return(currpid);
}
