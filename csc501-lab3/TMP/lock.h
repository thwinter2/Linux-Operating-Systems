#ifndef _LOCK_H_
#define _LOCK_H_

#define	NLOCKS 50
#define ALL -1
#define READ 10
#define WRITE 11

#define	LFREE	'\01'		/* this lock is free				*/
#define	LUSED	'\02'		/* this lock is used				*/

struct	lentry{
	char lstate;
	int ltype;
	int lprio;
	int lacquired[50];
	int ldeleted;
	int lnumreaders;
	int lockcnt;
	int lqhead;
	int lqtail;
};
extern	struct	lentry	locks[];
extern	int	nextlock;

void linit (void);
int lcreate (void);
int ldelete (int lockdescriptor);
int lock (int ldes1, int type, int priority);
void inherit_priorities_for_process(struct pentry *);
int max_schedprio_among_procs_waiting_for_lock(struct lentry *);

#endif
