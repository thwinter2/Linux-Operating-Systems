#include <kernel.h>
#include <lock.h>
#include <q.h>
#include <stdio.h>

void linit(){
int i = 0;
struct	lentry	*lockptr;

for (i; i < NLOCKS; i++) {
		(lockptr = &locktab[i])->lstate = LFREE;
		lockptr->lqtail = 1 + (lockptr->lqhead = newqueue());
	}

}
