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
	STATWORD(ps);
	disable(ps);
	int proc_id, bsm;
	struct mblock *bs_block;
	struct pentry *proc;

	if(hsize <= 0 || hsize > NUM_OF_PAGES){
		restore(ps);
		return SYSERR;
	}

	if(get_bsm(&bsm) == SYSERR){
		restore(ps);
		return SYSERR;
	}

  proc_id = create(procaddr,ssize,priority,name,nargs,args);
  if(proc != SYSERR){
    if(bsm_map(proc_id, FIRST_VIRT_PAGE_PROC, bsm, hsize) != SYSERR){
			proc = &proctab[proc_id];
			proc->store = bsm;
			proc->vhpno = FIRST_VIRT_PAGE_PROC;	//4096
			proc->vhpnpages = hsize;
			proc->vmemlist->mnext = FIRST_VIRT_PAGE_PROC*NBPG;
			proc->vmemlist->mlen = FIRST_VIRT_PAGE_PROC*hsize;

			bs_block = BACKING_STORE_BASE + (BACKING_STORE_UNIT_SIZE*bsm);
			bs_block->mlen = hsize*NBPG;
			bs_block->mnext = NULL;

			restore(ps);
			return proc_id;
		}
  }
	else{
		restore(ps);
		return SYSERR;
	}
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
