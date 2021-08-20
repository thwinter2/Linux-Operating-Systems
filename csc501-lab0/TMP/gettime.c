/* gettime.c - gettime */

#include <conf.h>
#include <kernel.h>
#include <date.h>
#include <proc.h>
#include <lab0.h>

extern int getutim(unsigned long *);

/*------------------------------------------------------------------------
 *  gettime  -  get local time in seconds past Jan 1, 1970
 *------------------------------------------------------------------------
 */
SYSCALL	gettime(long *timvar)
{
  unsigned long syscall_start_time;
  struct pentry *proc = &proctab[currpid];
  extern ctr1000;
	if(syscalls_trace == TRUE){
		syscall_start_time = ctr1000;
		proc->syscall_counts[4] += 1;	//increment count of syscall by 1
	}

    /* long	now; */

	/* FIXME -- no getutim */
  if(syscalls_trace == TRUE){
		proc->syscall_times[4] += (ctr1000 - syscall_start_time);
			//add time it took for this syscall to the total time of similar syscalls
	}
    return OK;
}
