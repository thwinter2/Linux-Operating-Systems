#include <kernel.h>
#include <lock.h>
#include <q.h>
#include <stdio.h>

void linit(){
int i;
struct	lentry	*lptr;

for (i=0 ; i<NLOCKS ; i++) {
		(lptr = &locks[i])->lstate = LFREE;
		lptr->lqtail = 1 + (lptr->lqhead = newqueue());
	}

}
