/* vcreate.c - vcreate */

#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD ps;
	disable(ps);
	int avail;
	int x = get_bsm(&avail);
	if(hsize < 1 || hsize > NUM_OF_PAGES || x == SYSERR){
		restore(ps);
		return SYSERR;
	}

	int pid=create(procaddr,ssize,priority,name,nargs,args);
	if(pid!=SYSERR){
		int y = bsm_map(pid, 4096, avail, hsize);
		if(y != SYSERR){
			proctab[pid].store = avail;
      proctab[pid].vhpnpages = hsize;
			proctab[pid].vhpno = FIRST_VIRT_PAGE_PROC;
			proctab[pid].vmemlist->mnext = NBPG * FIRST_VIRT_PAGE_PROC;
			struct mblock *bs = (avail * BACKING_STORE_UNIT_SIZE) + BACKING_STORE_BASE;
			bs->mlen = hsize * NBPG;
			bs->mnext = NULL;
			restore(ps);
			return pid;
		}
	}
	restore(ps);
	return SYSERR;
}
/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
