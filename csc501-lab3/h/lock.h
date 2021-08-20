#define NLOCKS 50
//#define DELETE 1
#define READ 100
#define WRITE 200
#define LFREE 400
#define LUSED 500

void linit(void);
int lcreate(void);
int ldelete(int lockdescriptor);
int lock(int ldes1, int type, int priority);
int releaseall(int numlocks, ...);

struct lentry{
  char lstate;
  char ltype;
  int lhead;
  int ltail;
  int readers;
  int procholders[NPROC];
};

extern struct lentry locktab[];




#ifndef _LOCK_H_
#define _LOCK_H_

#define ALL		 -1
#define READ		 10
#define WRITE		 11
#ifndef	NLOCKS
#define	NLOCKS		50	/* number of locks, if not defined		*/
#endif

#define	LFREE	'\01'		/* this lock is free				*/
#define	LUSED	'\02'		/* this lock is used				*/

struct	lentry	{
	char lstate;
	int ltype;
	int lprio;
	int lacquired[NPROC];
	int ldeleted;
	int lnumreaders;
	int lockcnt;
	int lqhead;
	int lqtail;
};
extern struct lentry locktab[];
extern int nextlock;

void linit (void);
void inherit_priorities_for_process(struct pentry *);
int max_schedprio_among_procs_waiting_for_lock(struct lentry *);
