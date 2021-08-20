/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern int page_replace_policy;

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
 // kprintf("To be implemented!\n");
	STATWORD(ps);
	disable(ps);
	int i = 0;
	for(i; i < NFRAMES; i++){
		frm_tab[i].fr_status = FRM_UNMAPPED;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_vpno = 0;
		frm_tab[i].fr_dirty = 0;
		frm_tab[i].fr_type = FR_PAGE;
	}
	restore(ps);
	return OK;
}

void init_queue(){
	int i;
	for(i=0;i<NFRAMES;i++){
		frame_q[i].frame_num = i;
		frame_q[i].age = 0;
		frame_q[i].next = -1;
		frame_q[i].previous = -1;
	}
}
/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
//  kprintf("To be implemented!\n");
	STATWORD(ps);
	disable(ps);
	int i = 0;
	int frame_num = -1;
	for(i; i < NFRAMES; i++){
		if(frm_tab[i].fr_status == FRM_UNMAPPED){
			*avail=i;
			restore(ps);
			return OK;
		}
	}

	if(page_replace_policy == AGING){
		frame_num = get_AGE_frame();
	}
	else if(page_replace_policy == SC){
		frame_num = get_SC_frame();
	}
	if(frame_num>=0){
		free_frm(frame_num);
		*avail=frame_num;
	}
	else{
		restore(ps);
		return SYSERR;
	}
	restore(ps);
	return OK;
}

int get_SC_frame()
{
	STATWORD ps;
	disable(ps);
	int frame_num = 0;
	int current = 0,next = -1,previous = -1;
	int vpt_offset,vpd_offset;
	long pdbr,virt_addr;
	pt_t *page_table_entry;
	pd_t *page_directory_entry;
	virt_addr_t *virt_addr_pointer;

	while(current!=-1){	//while loop to find frame
		virt_addr = frm_tab[current].fr_vpno;
		virt_addr_pointer = (virt_addr_t*)&virt_addr;
		vpt_offset = virt_addr_pointer->pt_offset;
		vpd_offset = virt_addr_pointer->pd_offset;
		pdbr = proctab[currpid].pdbr;
		page_directory_entry=pdbr+vpd_offset*sizeof(pd_t);
		page_table_entry = (pt_t*)(page_directory_entry->pd_base*FIRST_VIRT_PAGE_PROC + sizeof(pt_t) * vpt_offset);
		frame_num=head;

		if(page_table_entry->pt_acc == 1){
			page_table_entry->pt_acc = 0;
			previous = current;
			current = frame_q[current].next;
		}
		else{
			if(previous == -1){
				head = frame_q[current].next;
				frame_q[current].next = -1;
				restore(ps);
				return frame_num;
			}
			else{
				frame_q[previous].next = frame_q[current].next;
				frame_q[current].next = -1;
        restore(ps);
        return frame_num;
			}
		}
	}
	head = frame_q[current].next;
	frame_q[current].next = -1;
  restore(ps);
	return frame_num;
}

int get_AGE_frame(){
	STATWORD ps;
  disable(ps);
  int frame_num = 0, current, next = -1, previous = -1;
	int vpt_offset, vpd_offset;
  long pdbr, virt_addr;
	virt_addr_t *virt_addr_pointer;
  pd_t *page_directory_entry;
	pt_t *page_table_entry;

	while(current != -1){
    virt_addr = frm_tab[current].fr_vpno;
    virt_addr_pointer = (virt_addr_t*)&virt_addr;
    vpd_offset = virt_addr_pointer->pd_offset;
		vpt_offset = virt_addr_pointer->pt_offset;
    pdbr = proctab[currpid].pdbr;
    page_directory_entry = pdbr + sizeof(pd_t) * vpd_offset;
    page_table_entry=(pt_t*)(vpt_offset*sizeof(pt_t) + page_directory_entry->pd_base * FIRST_VIRT_PAGE_PROC);
    frame_num=head;
		frame_q[current].age=frame_q[current].age / 2;

		if(page_table_entry->pt_acc == 1)
                {
			int x = frame_q[current].age + 128;
			if(x < MAX_AGE)
				frame_q[current].age= x ;
			else
				frame_q[current].age = MAX_AGE;
		}
		if(frame_q[current].age < frame_q[frame_num].age){
			frame_num = current;
		}
		previous = current;
		current = frame_q[current].next;

	}
	restore(ps);
	return frame_num;
}



/*-------------------------------------------------------------------------
 * free_frm - free a frame
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
	STATWORD ps;
  disable(ps);
  int bsm_id,page,pt_offset,pd_offset,location;
  long pdbr,virt_addr;
  pt_t *page_table_entry;
  pd_t *page_directory_entry;
  if(((i == FR_TBL) && (i < FR_DIR)) || ((i == FR_TBL) && (i < FR_TBL)) || ((i == FR_PAGE) && (i < FR_PAGE))){
		restore(ps);
		return SYSERR;
	}

	if(frm_tab[i].fr_type == FR_PAGE)
	{
		virt_addr = frm_tab[i].fr_vpno;
		pdbr = proctab[frm_tab[i].fr_pid].pdbr;
		pd_offset = virt_addr / NFRAMES;
		pt_offset = virt_addr & 0x3FF;
		page_directory_entry = pdbr + (sizeof(pd_t) * pd_offset);
		page_table_entry = (page_directory_entry->pd_base * NBPG) + (sizeof(pt_t) * pt_offset);
		bsm_id = proctab[frm_tab[i].fr_pid].store;
		page = frm_tab[i].fr_vpno-proctab[frm_tab[i].fr_pid].vhpno;
		location = page_directory_entry->pd_base - FRAME0;

		write_bs(NBPG * (i + FRAME0), bsm_id, page);

		page_table_entry->pt_pres = 0;
		frm_tab[location].fr_refcnt = frm_tab[location].fr_refcnt-1;
		if((frm_tab[location].fr_refcnt) == 0){
			frm_tab[location].fr_status = FRM_UNMAPPED;
			frm_tab[location].fr_type = FR_PAGE;
			frm_tab[location].fr_pid = -1;
			frm_tab[location].fr_vpno = FIRST_VIRT_PAGE_PROC;
			page_directory_entry->pd_pres = 0;
		}
	}
	restore(ps);
	return OK;
}

void insert_frame(int frame_num)
{
	STATWORD ps;
	disable(ps);
	int current = head;
	int next = frame_q[current].next;

	if(head=-1){
		head = frame_num;
		restore(ps);
		return OK;
	}

	while(next != -1){
		current = next;
		next = frame_q[next].next;
	}

	frame_q[current].next = frame_num;
  frame_q[frame_num].next = -1;
  restore(ps);
  return OK;
}
