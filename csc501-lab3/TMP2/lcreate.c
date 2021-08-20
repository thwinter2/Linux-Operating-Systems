#include<stdio.h>
#include<conf.h>
#include<q.h>
#include<kernel.h>
#include<proc.h>
#include<lock.h>

int lcreate(void){
	int lock, i = 0;
	STATWORD ps;
	disable(ps);

	while(i < NLOCKS){
		lock = next_lock - 1;
		if(next_lock < 0){
			num_lock++;
			next_lock = NLOCKS - 1;
		}
		if(locktab[lock].lstate == FREE){
			locktab[lock].lstate = USED;
			restore(ps);
			return ((--next_lock) + num_lock);
		}
		i++;
	}
	restore(ps);
	return SYSERR;
}
