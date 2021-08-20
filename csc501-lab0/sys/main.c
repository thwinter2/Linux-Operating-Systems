/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{
	kprintf("\n\nHello World, Xinu lives\n\n");
  long param = 0xaabbccdd;
  long result = zfunction(param);
  kprintf("zfunction result is %08x\n",result);
  clktest();
	return 0;
}
