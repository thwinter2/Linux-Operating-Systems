/* stacktrace.c - stacktrace */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

static unsigned long	*esp;
static unsigned long	*ebp;

#define STKDETAIL

/*------------------------------------------------------------------------
 * stacktrace - print a stack backtrace for a process
 *------------------------------------------------------------------------
 */
SYSCALL stacktrace(int pid)
{
	unsigned long syscall_start_time;
  struct pentry *proc = &proctab[currpid];
  extern ctr1000;
	if(syscalls_trace == TRUE){
		syscall_start_time = ctr1000;
		proc->syscall_counts[23] += 1;	//increment count of syscall by 1
	}

	//struct pentry	*proc = &proctab[pid];
	unsigned long	*sp, *fp;

	if (pid != 0 && isbadpid(pid)){
		if(syscalls_trace == TRUE){
			proc->syscall_times[23] += (ctr1000 - syscall_start_time);
				//add time it took for this syscall to the total time of similar syscalls
		}
		return SYSERR;
	}
	if (pid == currpid) {
		asm("movl %esp,esp");
		asm("movl %ebp,ebp");
		sp = esp;
		fp = ebp;
	} else {
		sp = (unsigned long *)proc->pesp;
		fp = sp + 2; 		/* where ctxsw leaves it */
	}
	kprintf("sp %X fp %X proc->pbase %X\n", sp, fp, proc->pbase);
#ifdef STKDETAIL
	while (sp < (unsigned long *)proc->pbase) {
		for (; sp < fp; sp++)
			kprintf("DATA (%08X) %08X (%d)\n", sp, *sp, *sp);
		if (*sp == MAGIC)
			break;
		kprintf("\nFP   (%08X) %08X (%d)\n", sp, *sp, *sp);
		fp = (unsigned long *) *sp++;
		if (fp <= sp) {
			kprintf("bad stack, fp (%08X) <= sp (%08X)\n", fp, sp);
			if(syscalls_trace == TRUE){
				proc->syscall_times[23] += (ctr1000 - syscall_start_time);
					//add time it took for this syscall to the total time of similar syscalls
			}
			return SYSERR;
		}
		kprintf("RET  0x%X\n", *sp);
		sp++;
	}
	kprintf("MAGIC (should be %X): %X\n", MAGIC, *sp);
	if (sp != (unsigned long *)proc->pbase) {
		kprintf("unexpected short stack\n");
		if(syscalls_trace == TRUE){
			proc->syscall_times[23] += (ctr1000 - syscall_start_time);
				//add time it took for this syscall to the total time of similar syscalls
		}
		return SYSERR;
	}
#endif
if(syscalls_trace == TRUE){
	proc->syscall_times[23] += (ctr1000 - syscall_start_time);
		//add time it took for this syscall to the total time of similar syscalls
}
	return OK;
}
