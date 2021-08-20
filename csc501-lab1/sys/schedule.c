#include <stdio.h>
#include <sched.h>

int sched_type;

void setschedclass(int sched_class){
  sched_type = sched_class;
}

int getschedclass(void){
  return sched_type;
}
