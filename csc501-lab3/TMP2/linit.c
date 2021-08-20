#include<conf.h>
#include<kernel.h>
#include<stdio.h>
#include<proc.h>
#include<q.h>
#include<lock.h>

struct lentry locktab[NLOCKS];
int next_lock, num_lock;

void linit(void){
	struct lentry *lptr;
	num_lock = 0;
	next_lock = NLOCKS - 1;
	int i = 0, procid = 0;

	while(i < NLOCKS){
		lptr = &locktab[i];
		lptr->lstate = FREE;
		lptr->lprio = -1;
		lptr->lockqtail = (lptr->lockqhead = newqueue()) + 1;
		while(procid < NPROC){
			lptr->proc[procid] = 0;
			procid++;
		}
		i++;
	}
}
