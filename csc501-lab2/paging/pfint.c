/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern int currpid;
extern int page_replace_policy;
/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
void create_new_page_table(pt_t *table){
  int i;
  for(i = 0; i < NFRAMES; i++){
    table[i].pt_pres = 0;
		table[i].pt_write = 0;
		table[i].pt_user = 0;
    table[i].pt_pwt = 0;
    table[i].pt_pcd = 0;
    table[i].pt_acc = 0;
    table[i].pt_dirty = 0;
    table[i].pt_mbz = 0;
    table[i].pt_global = 0;
    table[i].pt_avail = 0;
    table[i].pt_base = 0;
  }
}

void update_page_directory(pd_t *page_directory_entry, int frame_num){
  page_directory_entry->pd_pres = 1;
  page_directory_entry->pd_write = 1;
  page_directory_entry->pd_user = 0;
  page_directory_entry->pd_pwt = 0;
  page_directory_entry->pd_pcd = 0;
  page_directory_entry->pd_acc = 0;
  page_directory_entry->pd_mbz = 0;
	page_directory_entry->pd_fmb = 0;
  page_directory_entry->pd_global = 0;
  page_directory_entry->pd_avail = 0;
  page_directory_entry->pd_base = FRAME0 + frame_num;
}

SYSCALL pfint()
{
  STATWORD ps;
	disable(ps);
  int bsm_num, page, frame_num, frame_addr, new_frame;
  long virt_addr = read_cr2();
  virt_addr_t *virt_addr_pointer = (virt_addr_t*)&virt_addr;
  pt_t *page_table_entry, *table;

  pd_t *page_directory_entry = proctab[currpid].pdbr + (virt_addr_pointer->pd_offset * sizeof(pd_t));
  if(page_directory_entry->pd_pres == 0){

    if(get_frm(&frame_num) == SYSERR){
      frame_num = -1;
    }
    else{
      frame_addr = (frame_num + FRAME0) * NBPG; //4096
      table = (pt_t*)frame_addr;
      create_new_page_table(&table);
    }
    if(frame_addr < 0){
      kill(currpid);
      restore(ps);
      return SYSERR;
    }
    update_page_directory(&page_directory_entry, frame_num);

    frm_tab[frame_num].fr_status = FRM_MAPPED;
    frm_tab[frame_num].fr_pid = currpid;
    frm_tab[frame_num].fr_type = FR_TBL;
  }

  page_table_entry = (pt_t*)((page_directory_entry->pd_base) * NBPG + (virt_addr_pointer->pt_offset * sizeof(pt_t)));
  if(page_table_entry->pt_pres == 0){
    if(get_frm(&new_frame) == SYSERR){
      kill(currpid);
      restore(ps);
      return SYSERR;
    }

    page_table_entry->pt_pres = 1;
    page_table_entry->pt_write = 1;
    page_table_entry->pt_base = new_frame + FRAME0;

		frm_tab[new_frame].fr_status = FRM_MAPPED;
		frm_tab[new_frame].fr_type = FR_PAGE;
		frm_tab[new_frame].fr_pid = currpid;
		frm_tab[new_frame].fr_vpno = virt_addr / NBPG;
    frm_tab[page_directory_entry->pd_base - FRAME0].fr_refcnt += 1;

    if(bsm_lookup(currpid, virt_addr, &bsm_num, &page) == SYSERR){
      restore(ps);
      return SYSERR;
    }
    read_bs((char*)((new_frame + FRAME0) * NBPG), bsm_num, page);
/*
    if(page_replace_policy == AGING){
      new_frame_node->age = MAX_AGE;
      new_frame_node->next = NULL;
      insert_frame_AGING(new_frame_node);
    }
    else if(page_replace_policy == SC){
      new_frame_node->age = NULL;
      new_frame_node->next = new_frame_node;
      insert_frame_SC(new_frame_node);
    }*/
    insert_frame_AGING(new_frame);
  }

  write_cr3(proctab[currpid].pdbr);
  restore(ps);
  return OK;
}
