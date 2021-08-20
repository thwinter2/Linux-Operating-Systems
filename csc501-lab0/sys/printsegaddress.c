#include <stdio.h>

extern long etext, edata, end;

void printsegaddress(void){
  kprintf("\nvoid printsegaddress()\n");
  kprintf("Current: extext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x\n", &etext, etext, &edata, edata, &end, end); //current segment
  kprintf("Preceding: extext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x\n", &etext-1, *(&etext-1), &edata-1, *(&edata-1), &end-1, *(&end-1)); //previous segment
  kprintf("After: extext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x\n", &etext+1, *(&etext+1), &edata+1, *(&edata+1), &end+1, *(&end+1)); //next segment
}
