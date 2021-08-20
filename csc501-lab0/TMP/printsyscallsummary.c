#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

void printsyscallsummary(void){
	struct pentry *proc;	//define process structure pointer
	int i = 0, j = 0, average_time = 0;
	Bool process_typed = FALSE;
	const char *syscall_names[27]={"freemem","chprio","getpid","getprio","gettime","kill",
																	"receive","recvclr","recvtim","resume","scount","sdelete",
																	"send","setdev","setnok","screate","signal","signaln","sleep",
																	"sleep10","sleep100","sleep1000","sreset","stacktrace",
																	"suspend","unsleep","wait"};
	kprintf("\nvoid printsyscallsummary()\n");

	while(i < NPROC){
		proc = &proctab[i];
    j = 0;
		process_typed = FALSE;
		
		while(j < 27){	//27 types of syscalls
			if(proc->syscall_counts[j] > 0){
				if(!process_typed){
					kprintf("Process [%d]\n", i);
					process_typed = TRUE;
				}
				average_time = proc->syscall_times[j]/proc->syscall_counts[j];	//compute average execution time of syscall
				kprintf("Syscall: sys_%s, count: %d, average execution time: %d (ms)\n",syscall_names[j],proc->syscall_counts[j],average_time);
			}
			j++;
		}
		i++;
	}
}

void syscallsummary_start(void){
	struct pentry *proc;
	int i = 0, j = 0;
	while(i < NPROC){
		proc = &proctab[i];
    j = 0;
		while(j < 27){
			proc->syscall_counts[j] = 0;
			proc->syscall_times[j] = 0;
			j++;
		}
		i++;
	}
	syscalls_trace = TRUE;
}

void syscallsummary_stop(void){
	syscalls_trace = FALSE;
}
