/*
================================================================================
  FreeVMS (R)
  Copyright (C) 2010 Dr. BERTRAND Joël and all.

  This file is part of FreeVMS

  FreeVMS is free software; you can redistribute it and/or modify it
  under the terms of the CeCILL V2 License as published by the french
  CEA, CNRS and INRIA.
 
  FreeVMS is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the CeCILL V2 License
  for more details.
 
  You should have received a copy of the CeCILL License
  along with FreeVMS. If not, write to info@cecill.info.
================================================================================
*/

#include "freevms/freevms.h"

// Find the first application and try to execute if. This application shall
// bootstrap the rest of the system.

void
sys$init(L4_KernelInterfacePage_t *kip, struct vms$meminfo *meminfo,
        vms$pointer pagesize, char *root_device)
{
    extern struct pd                freevms_pd;

    struct initial_obj              *obj;
    struct initial_obj              *init;
    struct pd                       *pd;
    struct thread                   *thread;

    struct memsection               *clist_section;
    struct memsection               *heap;
    struct memsection               *heap_phys;
    struct memsection               *stack;

    vms$pointer                     i;
    vms$pointer                     j;
    vms$pointer                     *init_vars;
    vms$pointer                     memsect;
    vms$pointer                     *user_stack;

    cap_t                           *clist;

    notice(SYSBOOT_I_SYSBOOT "spawning INIT.EXE with supervisor privileges\n");

    // Find INIT.EXE
    for(obj = meminfo->objects, i = 0; i < meminfo->num_objects; i++, obj++)
    {
        if (strstr(obj->name, "init.exe") != NULL)
        {
            break;
        }
    }

    PANIC(i == meminfo->num_objects,
            notice(INIT_F_NOTFOUND "INIT.EXE not found\n"));
    init = obj;

    notice(SYSBOOT_I_SYSBOOT "creating INIT.EXE process descriptor\n");

    // Start VMS$INIT in a new address space
    pd = jobctl$pd_create(&freevms_pd, 0, pagesize);

    notice(SYSBOOT_I_SYSBOOT "creating INIT.EXE UTCB pages\n");

    thread = jobctl$pd_create_thread(pd, -1);

    notice(SYSBOOT_I_SYSBOOT "reserving %ld bytes for %ld kernel threads\n", 
            L4_Size(pd->utcb_area), L4_Size(pd->utcb_area) / L4_UtcbSize(kip)); 
    notice(SYSBOOT_I_SYSBOOT "creating INIT.EXE stack\n");
    stack = vms$pd_create_memsection(pd, 2 * pagesize, 0, VMS$MEM_NORMAL,
            pagesize);

    // FIXME: heapsize (1 MB)
    notice(SYSBOOT_I_SYSBOOT "creating INIT.EXE heap\n");
    heap = vms$pd_create_memsection(pd, 1 * 1024 * 1024, 0,
            VMS$MEM_NORMAL | VMS$MEM_USER, pagesize);
    PANIC(heap == NULL);

    // Back the first 64k
    heap_phys = vms$pd_create_memsection(pd, 0x10000, 0, VMS$MEM_NORMAL,
            pagesize);
    PANIC(heap_phys->base % 0x10000 != 0);
    vms$memsection_page_map(heap, L4_Fpage(heap_phys->base, 0x10000),
            L4_Fpage(heap->base, 0x10000));

    // Create a clist
    notice(SYSBOOT_I_SYSBOOT "creating INIT.EXE clist\n");
    clist_section = vms$pd_create_memsection(pd, pagesize, 0, VMS$MEM_NORMAL,
            pagesize);
    // Check if we have run out of VM already we have trouble...
    PANIC(clist_section == NULL);

    clist = (cap_t *) clist_section->base;
    PANIC(clist == NULL);

    // Create capabilities and add them to clist
    notice(SYSBOOT_I_SYSBOOT "adding capabilities to INIT.EXE clist\n");

    for(obj = meminfo->objects, i = 0, j = 0;
            j < meminfo->num_objects; obj++, j++)
    {
        if (obj->flags & VMS$IOF_VIRT)
        {
            memsect = (vms$pointer) vms$objtable_lookup((void *) obj->base);
            PANIC(memsect == 0);

            clist[i++] = sec$create_capability(memsect, CAP$OBJ);
        }
    }

    clist[i++] = sec$create_capability((vms$pointer) pd, CAP$PD);
    clist[i++] = sec$create_capability((vms$pointer) thread, CAP$THREAD);
    clist[i++] = sec$create_capability((vms$pointer) stack, CAP$MEMSECTION);
    clist[i++] = sec$create_capability((vms$pointer) heap, CAP$MEMSECTION);
    // Reserve a slot for morecore to use
    clist[i++] = sec$create_capability((vms$pointer) heap, CAP$MEMSECTION);
    clist[i++] = sec$create_capability((vms$pointer) clist_section,
            CAP$MEMSECTION);

    init->entry = elf$loader(thread, init->base, init->end, clist, &i);

    jobctl$pd_add_clist(pd, clist);

    // Setup the stack (16 bytes alignment)
    user_stack = (vms$pointer *) vms$page_round_down(stack->end + 1, 16);

    if (vms$back_mem(heap->base, heap->base + (12 * sizeof(vms$pointer)),
            pagesize) != 0)
    {
        PANIC(1, notice(MEM_F_BACKMEM "unable to back heap during startup\n"));
    }

    /*
    init_vars = (vms$pointer *) (heap->base);
    *(init_vars + 0) = NULL;                        // Callback pointer
    *(init_vars + 1) = 0;                           // SYS$INPUT
    *(init_vars + 2) = 0;                           // SYS$OUTPUT
    *(init_vars + 3) = 0;                           // SYS$ERROR
    *(init_vars + 4) = heap->base;
    *(init_vars + 5) = heap->end;
    *(init_vars + 6) = 0;                           // cap_slot
    *(init_vars + 7) = i - 1;                       // cap_used
    *(init_vars + 8) = pagesize / sizeof(cap_t);    // cap_size
    *(init_vars + 9) = clist_section->base;         // cap_addr
    *(init_vars + 10) = 0;                          // naming_server

    *(--user_stack) = 0;                            // argc
    *(--user_stack) = (vms$pointer) init_vars;      // arguments
    */

    jobctl$thread_start(thread, init->entry, (vms$pointer) user_stack);

    return;
}
