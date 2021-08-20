/* resume.c - resume */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 * resume  --  unsuspend a process, making it ready; return the priority
 *------------------------------------------------------------------------
 */
SYSCALL resume(int pid)
{
	unsigned long syscall_start_time;
  struct pentry *proc = &proctab[currpid];
  extern ctr1000;
	if(syscalls_trace == TRUE){
		syscall_start_time = ctr1000;
		proc->syscall_counts[9] += 1;	//increment count of syscall by 1
	}

	STATWORD ps;
	struct	pentry	*pptr;		/* pointer to proc. tab. entry	*/
	int	prio;			/* priority to return		*/

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate!=PRSUSP) {
		restore(ps);
		if(syscalls_trace == TRUE){
			proc->syscall_times[9] += (ctr1000 - syscall_start_time);
				//add time it took for this syscall to the total time of similar syscalls
		}
		return(SYSERR);
	}
	prio = pptr->pprio;
	ready(pid, RESCHYES);
	restore(ps);

	if(syscalls_trace == TRUE){
		proc->syscall_times[9] += (ctr1000 - syscall_start_time);
			//add time it took for this syscall to the total time of similar syscalls
	}
	return(prio);
}
