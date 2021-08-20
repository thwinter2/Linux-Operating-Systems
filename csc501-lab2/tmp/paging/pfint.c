/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

extern int page_replace_policy;
/*-------------------------------------------------------------------------
 * pfinn- paging fault ISR
 *-------------------------------------------------------------------------
 */

void init_table(pt_t *table){
	int i = 0;
	for(i; i < NFRAMES; i++){
		table[i].pt_pres = 0;
		table[i].pt_user = 0;
		table[i].pt_pwt = 0;
		table[i].pt_write = 0;
    table[i].pt_pcd = 0;
		table[i].pt_dirty = 0;
    table[i].pt_acc = 0;
    table[i].pt_global = 0;
		table[i].pt_mbz = 0;
    table[i].pt_base = 0;
		table[i].pt_avail = 0;
	}
}

int create_page_table(){
	int frame_num,frame_addr;
	pt_t *table;

	if(get_frm(&frame_num)==SYSERR){
		return -1;
	}

	frame_addr = (frame_num + FRAME0) * NBPG;
	table = (pt_t*)frame_addr;
	init_table(table);

	return frame_num;
}
SYSCALL pfint()
{
	STATWORD ps;
	disable(ps);
	int new_table, new_frame, bs_id, vpt_offset, vpd_offset, page;
	long pdbr, virt_addr;
	virt_addr_t *virt_addr_pointer;
	pd_t *page_directory_entry;
	pt_t *page_table_entry;

	pdbr = proctab[currpid].pdbr;
	virt_addr = read_cr2();
	virt_addr_pointer = (virt_addr_t*)&virt_addr;
	vpt_offset = virt_addr_pointer->pt_offset;
	vpd_offset = virt_addr_pointer->pd_offset;
	page_directory_entry = pdbr + (vpd_offset * sizeof(pd_t));

	if((page_directory_entry->pd_pres) == 0){
		new_table = create_page_table();
		if(new_table==-1){
			kill(currpid);
			restore(ps);
			return SYSERR;
		}
		frm_tab[new_table].fr_status = FRM_MAPPED;
		frm_tab[new_table].fr_pid = currpid;
		frm_tab[new_table].fr_type = FR_TBL;

		page_directory_entry->pd_pres = 1;
		page_directory_entry->pd_pwt = 0;
		page_directory_entry->pd_write = 1;
		page_directory_entry->pd_user = 0;
		page_directory_entry->pd_acc = 0;
    page_directory_entry->pd_pcd = 0;
		page_directory_entry->pd_fmb = 0;
    page_directory_entry->pd_mbz = 0;
		page_directory_entry->pd_avail = 0;
    page_directory_entry->pd_global = 0;
    page_directory_entry->pd_base = new_table + FRAME0;
	}

	page_table_entry = (pt_t*)(page_directory_entry->pd_base * NBPG + sizeof(pt_t) * vpt_offset);
	if(page_table_entry->pt_pres == 0){
		if(get_frm(&new_frame)==SYSERR){
			kill(currpid);
      restore(ps);
      return SYSERR;
		}

		frm_tab[new_frame].fr_status = FRM_MAPPED;
		frm_tab[new_frame].fr_vpno = virt_addr / NBPG;
		frm_tab[new_frame].fr_pid = currpid;
		frm_tab[new_frame].fr_type = FR_PAGE;
		frm_tab[page_directory_entry->pd_base - FRAME0].fr_refcnt++;

		page_table_entry->pt_pres = 1;
		page_table_entry->pt_write = 1;
		page_table_entry->pt_base = new_frame + FRAME0;

		if(bsm_lookup(currpid, virt_addr, &bs_id, &page)==SYSERR){
			restore(ps);
			return SYSERR;
		}
		read_bs((char*)((new_frame + FRAME0) * NBPG), bs_id, page);
		insert_frame(new_frame);
	}
	write_cr3(pdbr);	//update pdbr
	restore(ps);
return OK;
}
