#include <stdio.h>
#include <kernel.h>
#include <proc.h>

static unsigned long *esp;	//stack pointer variable

void printprocstks(int priority){
	struct pentry *proc;	//define pointer to a process entry
	kprintf("\nvoid printprocstks(int priority)\n");
 
  int i = 0;
	while(i < NPROC){  //if process count is less than number of processes
		proc = &proctab[i];
   
		if(proc->pstate != PRFREE){
			if(proc->pprio > priority){	//only if process priority is greater than input variable do we print values
				kprintf("proc [%s]\n", proc->pname);	//process name
				kprintf("pid: %d\n", i);	//process ID
				kprintf("priority: %d\n", proc->pprio);	//process priority
				kprintf("base: 0x%08x\n", proc->pbase);	//stack base
				kprintf("limit: 0x%08x\n", proc->plimit);	//stack limit
				kprintf("len: %d\n", proc->pstklen);	//stack size
        
				if(proc->pstate == PRCURR){
					asm("movl %esp, esp");	//if the proccess i is currently being executed, get its stack pointer
				}
				else{
					esp = proc->pesp;	//else you can just get the stack pointer from the pesp variable
				}
				kprintf("pointer: 0x%08x\n", esp);		//stack pointer
			}
		}
  i++;
	}
}
