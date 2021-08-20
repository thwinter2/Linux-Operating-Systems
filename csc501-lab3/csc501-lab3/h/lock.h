#ifndef _LOCK_H_
#define _LOCK_H_

#define NLOCKS 50
#define FREE 0
#define USED 2
#define READ 3
#define WRITE 4

extern long ctr1000;
extern int num_lock;
extern int next_lock;

struct lentry{
	int lstate;
	int ltype;
	int lprio;
	int proc[NPROC];
	int writer;
	int reader;
	int lockqhead;
	int lockqtail;
};
extern struct lentry locktab[];

void linit(void);
int lcreate(void);
int ldelete(int lockdescriptor);
int lock(int ldes1, int type, int priority);
void release(int ldes1, int node);
int releaseall(int numlocks, int ldesc1, ...);
#endif
