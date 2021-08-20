#include <stdio.h>

unsigned long	*ebp;
unsigned long	*esp;	//stack pointer variable

void printtos(void){
	asm("movl %ebp,ebp");
	asm("movl %esp,esp");

	kprintf("\nvoid printtos()\n");
	kprintf("Before[0x%08x]: 0x%08x\n", (ebp+2), *(ebp+2));	//stack contents of preceding process
	kprintf("After[0x%08x]: 0x%08x\n", ebp, *ebp);	//stack contents of next process
 
  int i = 0; 
	while(i<4 && ebp>esp){
	  kprintf("element[0x%08x]: 0x%08x\n", (esp+i), *(esp+i));	//stack contents of top 4 stack locations in current process
    i++;
	}
}
