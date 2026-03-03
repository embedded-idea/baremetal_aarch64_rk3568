#include <assert.h>
#include <string.h>
#include <minix/param.h>
#include <minix/board.h>
#include "minixm_main.h"
#include "kernel/glo.h"
#include "kernel/debug.h"


kinfo_t main_kinfo;
struct kmessages main_kmessages;
struct machine main_machine;
/*===========================================================================*
 *			kmain 	                             		*
 *===========================================================================*/
void kmain(kinfo_t *local_cbi)
{
   // Start the ball rolling. 
    struct boot_image *ip;	//boot image pointer
    //register struct proc *rp;	//process pointer
    register int i, j;
    static int bss_test;

    // bss sanity check
    assert(bss_test == 0);
    bss_test = 1;

    // save a global copy of the boot parameters
    memcpy(&main_kinfo, local_cbi, sizeof(main_kinfo));
    memcpy(&main_kmessages, main_kinfo.kmessages, sizeof(main_kmessages));

    // We have done this exercise in pre_init so we expect this code
    //   to simply work! 
    main_machine.board_id = get_board_id_by_name("LUBANCAT2");

    // We can talk now
    DEBUGBASIC(("MINIX booting\n"));

//   // Kernel may use bits of main memory before VM is started 
//   kernel_may_alloc = 1;

//   assert(sizeof(kinfo.boot_procs) == sizeof(image));
//   memcpy(kinfo.boot_procs, image, sizeof(kinfo.boot_procs));

//   cstart();

//   BKL_LOCK();
 
//    DEBUGEXTRA(("main()\n"));

//   //Clear the process table. Anounce each slot as empty and set up mappings
//   //for proc_addr() and proc_nr() macros. Do the same for the table with
//   //privilege structures for the system processes and the ipc filter pool.
//   proc_init();
//   IPCF_POOL_INIT();

//    if(NR_BOOT_MODULES != kinfo.mbi.mi_mods_count)
//    	panic("expecting %d boot processes/modules, found %d",
// 		NR_BOOT_MODULES, kinfo.mbi.mi_mods_count);

//   // Set up proc table entries for processes in boot image.
//   for (i=0; i < NR_BOOT_PROCS; ++i) {
// 	int schedulable_proc;
// 	proc_nr_t proc_nr;
// 	int ipc_to_m, kcalls;
// 	sys_map_t map;

// 	ip = &image[i];				// process' attributes 
// 	DEBUGEXTRA(("initializing %s... ", ip->proc_name));
// 	rp = proc_addr(ip->proc_nr);		// get process pointer 
// 	ip->endpoint = rp->p_endpoint;		// ipc endpoint 
// 	rp->p_cpu_time_left = 0;
// 	if(i < NR_TASKS)			// name (tasks only) 
// 		strlcpy(rp->p_name, ip->proc_name, sizeof(rp->p_name));

// 	if(i >= NR_TASKS) {
// 		// Remember this so it can be passed to VM 
// 		multiboot_module_t *mb_mod = &kinfo.module_list[i - NR_TASKS];
// 		ip->start_addr = mb_mod->mod_start;
// 		ip->len = mb_mod->mod_end - mb_mod->mod_start;
// 	}
	
// 	reset_proc_accounting(rp);

// 	 // See if this process is immediately schedulable.
// 	 // In that case, set its privileges now and allow it to run.
// 	 // Only kernel tasks and the root system process get to run immediately.
// 	 // All the other system processes are inhibited from running by the
// 	 // RTS_NO_PRIV flag. They can only be scheduled once the root system
// 	 // process has set their privileges.
// 	proc_nr = proc_nr(rp);
// 	schedulable_proc = (iskerneln(proc_nr) || isrootsysn(proc_nr) ||
// 		proc_nr == VM_PROC_NR);
// 	if(schedulable_proc) {
// 	    // Assign privilege structure. Force a static privilege id. 
//             (void) get_priv(rp, static_priv_id(proc_nr));

//             // Privileges for kernel tasks. 
// 	    if(proc_nr == VM_PROC_NR) {
//                 priv(rp)->s_flags = VM_F;
//                 priv(rp)->s_trap_mask = SRV_T;
// 		ipc_to_m = SRV_M;
// 		kcalls = SRV_KC;
//                 priv(rp)->s_sig_mgr = SELF;
//                 rp->p_priority = SRV_Q;
//                 rp->p_quantum_size_ms = SRV_QT;
// 	    }
// 	    else if(iskerneln(proc_nr)) {
//                 // Privilege flags. 
//                 priv(rp)->s_flags = (proc_nr == IDLE ? IDL_F : TSK_F);
//                 // Init flags. 
//                 priv(rp)->s_init_flags = TSK_I;
//                 // Allowed traps. 
//                 priv(rp)->s_trap_mask = (proc_nr == CLOCK 
//                     || proc_nr == SYSTEM  ? CSK_T : TSK_T);
//                 ipc_to_m = TSK_M;                  // allowed targets 
//                 kcalls = TSK_KC;                   // allowed kernel calls 
//             }
//             // Privileges for the root system process. 
//             else {
// 	    	assert(isrootsysn(proc_nr));
//                 priv(rp)->s_flags= RSYS_F;        // privilege flags 
//                 priv(rp)->s_init_flags = SRV_I;   // init flags 
//                 priv(rp)->s_trap_mask= SRV_T;     // allowed traps 
//                 ipc_to_m = SRV_M;                 // allowed targets 
//                 kcalls = SRV_KC;                  // allowed kernel calls 
//                 priv(rp)->s_sig_mgr = SRV_SM;     // signal manager 
//                 rp->p_priority = SRV_Q;	          // priority queue 
//                 rp->p_quantum_size_ms = SRV_QT;   // quantum size 
//             }

//             // Fill in target mask. 
//             memset(&map, 0, sizeof(map));

//             if (ipc_to_m == ALL_M) {
//                 for(j = 0; j < NR_SYS_PROCS; j++)
//                     set_sys_bit(map, j);
//             }

//             fill_sendto_mask(rp, &map);

//             // Fill in kernel call mask. 
//             for(j = 0; j < SYS_CALL_MASK_SIZE; j++) {
//                 priv(rp)->s_k_call_mask[j] = (kcalls == NO_C ? 0 : (~0));
//             }
// 	}
// 	else {
// 	    // Don't let the process run for now. 
//             RTS_SET(rp, RTS_NO_PRIV | RTS_NO_QUANTUM);
// 	}

// 	// Arch-specific state initialization. 
// 	arch_boot_proc(ip, rp);

// 	// scheduling functions depend on proc_ptr pointing somewhere. 
// 	if(!get_cpulocal_var(proc_ptr))
// 		get_cpulocal_var(proc_ptr) = rp;

// 	// Process isn't scheduled until VM has set up a pagetable for it. 
// 	if(rp->p_nr != VM_PROC_NR && rp->p_nr >= 0) {
// 		rp->p_rts_flags |= RTS_VMINHIBIT;
// 		rp->p_rts_flags |= RTS_BOOTINHIBIT;
// 	}

// 	rp->p_rts_flags |= RTS_PROC_STOP;
// 	rp->p_rts_flags &= ~RTS_SLOT_FREE;
// 	DEBUGEXTRA(("done\n"));
//  }

  // update boot procs info for VM 
//   memcpy(kinfo.boot_procs, image, sizeof(kinfo.boot_procs));

// #define IPCNAME(n) { \
// 	assert((n) >= 0 && (n) <= IPCNO_HIGHEST); \
// 	assert(!ipc_call_names[n]);	\
// 	ipc_call_names[n] = #n; \
// }

//   arch_post_init();

//   IPCNAME(SEND);
//   IPCNAME(RECEIVE);
//   IPCNAME(SENDREC);
//   IPCNAME(NOTIFY);
//   IPCNAME(SENDNB);
//   IPCNAME(SENDA);

  // System and processes initialization 
//   memory_init();
//   DEBUGEXTRA(("system_init()... "));
//   system_init();
//   DEBUGEXTRA(("done\n"));

//   // The bootstrap phase is over, so we can add the physical
//   // memory used for it to the free list.
//   add_memmap(&kinfo, kinfo.bootstrap_start, kinfo.bootstrap_len);

// #ifdef CONFIG_SMP
//   if (config_no_apic) {
// 	  DEBUGBASIC(("APIC disabled, disables SMP, using legacy PIC\n"));
// 	  smp_single_cpu_fallback();
//   } else if (config_no_smp) {
// 	  DEBUGBASIC(("SMP disabled, using legacy PIC\n"));
// 	  smp_single_cpu_fallback();
//   } else {
// 	  smp_init();
// 	   // if smp_init() returns it means that it failed and we try to finish
// 	   // single CPU booting
// 	  bsp_finish_booting();
//   }
// #else
//    // if configured for a single CPU, we are already on the kernel stack which we
//    // are going to use everytime we execute kernel code. We finish booting and we
//    // never return here
   
//   bsp_finish_booting();
// #endif

//   NOT_REACHABLE;
}