/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern fr_map_t frm_tab[];
extern int page_replace_policy;

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
  STATWORD ps;
	disable(ps);
  int i = 0;

  for(i; i < NFRAMES; i++){
    frm_tab[i].fr_status = FRM_UNMAPPED;
    frm_tab[i].fr_pid = -1;
    frm_tab[i].fr_vpno = -1;
    frm_tab[i].fr_refcnt = 0;
    frm_tab[i].fr_type = FR_PAGE;
    frm_tab[i].fr_dirty = 0;
  }
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  STATWORD ps;
	disable(ps);
  int i = 0, frame_num = -1;

  for(i; i < NFRAMES; i++){
    if(frm_tab[i].fr_status = FRM_UNMAPPED){
      *avail = i;
      restore(ps);
      return OK;
    }
  }
  if(page_replace_policy == SC){
    frame_num = get_frame_SC();
  }
  else if(page_replace_policy == AGING){
    frame_num = get_frame_AGING();
  }
  else{
    kprintf("Select a valid page replacement policy");
    restore(ps);
    return SYSERR;
  }
  if(frame_num < 0){
    restore(ps);
    return SYSERR;
  }
  else{
    free_frm(frame_num);
    *avail = frame_num;
  }

  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
  STATWORD ps;
	disable(ps);
  pt_t *page_table_entry;
  pd_t *page_directory_entry;
  int bsm_num, page, location;
  long virt_page_num;

  if(frm_tab[i].fr_type == FR_PAGE) {

    virt_page_num = frm_tab[i].fr_vpno;
    page_directory_entry = proctab[frm_tab[i].fr_pid].pdbr + (virt_page_num / NFRAMES * sizeof(pd_t));
    location = page_directory_entry->pd_base - FRAME0;
    bsm_num = proctab[frm_tab[i].fr_pid].store;
		page = frm_tab[i].fr_vpno - proctab[frm_tab[i].fr_pid].vhpno;
    page_table_entry = (page_directory_entry->pd_base * NBPG) + (sizeof(pt_t) * (virt_page_num & 0x3FF));

    write_bs(NBPG*(FRAME0 + i), bsm_num, page);
    page_table_entry->pt_pres = 0;
    //page_table_entry->pt_write = 0;
    //page_table_entry->pt_dirty = 0;
    frm_tab[location].fr_refcnt--;
    if(frm_tab[location].fr_refcnt == 0){
      frm_tab[location].fr_status = FRM_UNMAPPED;
      frm_tab[location].fr_pid = -1;
      frm_tab[location].fr_type = FR_PAGE;
      frm_tab[location].fr_vpno = FIRST_VIRT_PAGE_PROC;
      page_directory_entry->pd_pres = 0;
    }

    restore(ps);
    return OK;
  }
  else{
    restore(ps);
    return SYSERR;
  }
}
/*
void init_frame_queue(void){
  int i = 0;
  for(i; i < NFRAMES; i++){
    frame_queue[i]->frame_num = i;
    frame_queue[i]->next = NULL;
  }
}
*/
int get_frame_SC(void){
  STATWORD ps;
	disable(ps);
  int current = 0, previous = -1, frame_num = 0;
  pt_t *page_table_entry;
  pd_t *page_directory_entry;
  virt_addr_t *virt_addr;
  long virt_page_num;

  while(current != -1){   //AKA will only exit loop when page replacement occurs since it is a circular queue
    virt_page_num = frm_tab[current].fr_vpno;
    virt_addr = (virt_addr_t*)&virt_page_num;
    page_directory_entry = proctab[currpid].pdbr + (sizeof(pd_t) * virt_addr->pd_offset);
    if(page_directory_entry->pd_pres == 0){ //page not present
      restore(ps);
      return SYSERR;
    }

    page_table_entry = (pt_t*)(page_directory_entry->pd_base * NBPG + (sizeof(pt_t) * virt_addr->pt_offset));
    if(page_table_entry->pt_pres == 0){ //page not present
      restore(ps);
      return SYSERR;
    }

    frame_num = head;
    if(page_table_entry->pt_acc == 0){
      if(previous == -1){
        head = frame_q[current].next;
        frame_q[current].next = -1;
        restore(ps);
        return frame_num;
      }
      frame_q[previous].next = frame_q[current].next;
      frame_q[current].next = -1;
      restore(ps);
      return frame_num;
    }
    else{
      page_table_entry->pt_acc = 0;
      previous = current;
      current = frame_q[current].next;
    }
  }
  head = frame_q[current].next;
  frame_q[current].next = -1;
  restore(ps);
  return frame_num;
}

int get_frame_AGING(void){
  STATWORD ps;
	disable(ps);
  int current, frame_num = 0, previous = -1, smallest_age = frame_q[current].age;
  pt_t *page_table_entry;
  pd_t *page_directory_entry;
  virt_addr_t *virt_addr;
  long virt_page_num;

  while(current != -1){
    virt_page_num = frm_tab[current].fr_vpno;
    virt_addr = (virt_addr_t*)&virt_page_num;
    page_directory_entry = proctab[currpid].pdbr + (sizeof(pd_t) * virt_addr->pd_offset);
    if(page_directory_entry->pd_pres == 0){ //page not present
      restore(ps);
      return SYSERR;
    }

    page_table_entry = (pt_t*)(page_directory_entry->pd_base * NBPG + (sizeof(pt_t) * virt_addr->pt_offset));
    if(page_table_entry->pt_pres == 0){ //page not present
      restore(ps);
      return SYSERR;
    }

    frame_num = head;
    frame_q[current].age >>= 1;
    if(page_table_entry->pt_acc == 1){
      frame_q[current].age += 128;
      if(frame_q[current].age > MAX_AGE){
        frame_q[current].age = MAX_AGE;  //255
      }
    }

    if(frame_q[current].age < smallest_age){
      smallest_age = frame_q[current].age;
      frame_num = current;
    }
    current = frame_q[current].next;
  }
  restore(ps);
  return frame_num;
}

void insert_frame_AGING(int frame_num){
  STATWORD ps;
	disable(ps);
  if(head == -1){
    head = frame_num;
    restore(ps);
    return OK;
  }

  int current;
  current = head;
  int next = frame_q[head].next;
  while(next != -1){    //loop through FIFO queue
    current = next;
    next = frame_q[current].next;
  }

  frame_q[current].next = frame_num;  //insert frame into back of FIFO queue
  frame_q[frame_num].next = -1;
  restore(ps);
  return OK;
}

void modify_page_directory(int proc_id){
  int i, frame_num = 0;
	pd_t *page_directory_entry;
	if(get_frm(&frame_num)==SYSERR)
		return SYSERR;
	proctab[proc_id].pdbr = (frame_num + FRAME0) * NBPG;
	frm_tab[frame_num].fr_pid = proc_id;
	frm_tab[frame_num].fr_type = FR_DIR;
	frm_tab[frame_num].fr_status = FRM_MAPPED;
	frm_tab[frame_num].fr_vpno = 0;
	frm_tab[frame_num].fr_refcnt = 4;
	page_directory_entry = proctab[proc_id].pdbr;
	for(i = 0; i < 1024; i++){
		page_directory_entry[i].pd_pres = 0;
		page_directory_entry[i].pd_write = 1;
		page_directory_entry[i].pd_user = 0;
		page_directory_entry[i].pd_pwt = 0;
		page_directory_entry[i].pd_pcd = 0;
		page_directory_entry[i].pd_acc = 1;
		page_directory_entry[i].pd_mbz = 0;
		page_directory_entry[i].pd_global = 0;
		page_directory_entry[i].pd_avail = 0;
		page_directory_entry[i].pd_base = 0;
		if(i>=0&&i<4){
      page_directory_entry[i].pd_pres = 1;
      page_directory_entry[i].pd_base = i + FRAME0;
    }
	}
}
/*
void insert_frame_SC(struct frame_node *frame){
  STATWORD ps;
	disable(ps);
  if(head == NULL){
    head = frame;
    restore(ps);
    return OK;
  }

  struct frame_node *prev;
  prev = head;
  struct frame_node *current;
  current = prev->next;

  while(current != head){    //loop through FIFO queue
    prev = current;
    current = prev->next;
  }

  frame->next = current;
  prev->next = frame;  //insert frame into back of FIFO queue
  restore(ps);
  return OK;
}*/
/*
int remove_frame_AGING(void){
  int frame_num = head;
  head = frame_queue[head]->next;  //move head to 2nd frame in queue
  frame_queue[frame_num]->next = -1; //remove next pointer from queue
  return frame_num;
}

int remove_frame_SC(void){
  int frame_num = head;
  head = frame_queue_sc[head]->next;  //move head to 2nd frame in queue
  frame_queue_sc[frame_num]->next = -1; //remove next pointer from queue
  return frame_num;
}*/
