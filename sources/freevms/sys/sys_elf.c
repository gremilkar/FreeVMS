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

// If thread != NULL, image is granted to new address space.

vms$pointer
sys$elf_loader(struct thread *thread, vms$pointer start, vms$pointer end,
        cap_t *clist, vms$pointer *pos)
{
    ehdr_t              *eh;
    phdr_t              *ph;

    struct memsection   *memsection;

	vms$pointer			base;
    vms$pointer         dst_end;
    vms$pointer         dst_start;
    vms$pointer         i;
	vms$pointer			size;
    vms$pointer         src_end;
    vms$pointer         src_start;

	// ELF header
    eh = (ehdr_t *) start;

    PANIC((eh->type != 2) || (eh->phoff == 0),
            notice(ELF_F_FMT "cannot load binary stream\n"));

    for(i = 0; i < eh->phnum; i++)
    {
		// Program header
        ph = (phdr_t*) (start + eh->phoff + (eh->phentsize * i));

        if (ph->msize < ph->fsize)
        {
            ph->msize = ph->fsize;
        }

        if (ph->type == PT_LOAD)
        {
            src_start = start + ph->offset;
            src_end = src_start + ph->msize;
            dst_start = ph->paddr;
            dst_end = dst_start + ph->msize;

            // Allocating aligned memory

			base = sys$page_round_down(dst_start, vms$min_pagesize());
			size = (sys$page_round_up(dst_end, vms$min_pagesize()) - base);

for(char * j = (char *) src_start;
		j < (char *) (src_start + size);)
{
	notice("%016lx -> ", (vms$pointer) j);
	char *l = j;
	char *k = j+16; for(; j < k; j++) notice("%02lx ", (unsigned char) *j);
	notice("\n");
	j = l;
	k = j+16; for(; j < k; j++) notice("%c", (unsigned char) *j);
	notice("\n");
}
            memsection = sys$pd_create_memsection(thread->owner, size, base,
                    VMS$MEM_FIXED, vms$min_pagesize());
            clist[(*pos)++] = sec$create_capability((vms$pointer)
                    memsection, CAP$MEMSECTION);
            sys$memcopy(dst_start, src_start, ph->fsize);
for(char * j = (char *) dst_start;
		j < (char *) (dst_start + size);)
{
	notice("%016lx -> ", (vms$pointer) j);
	char *l = j;
	char *k = j+16; for(; j < k; j++) notice("%02lx ", (unsigned char) *j);
	notice("\n");
	j = l;
	k = j+16; for(; j < k; j++) notice("%c", (unsigned char) *j);
	notice("\n");
}
        }
		else
		{
			src_start = start + ph->offset;
			src_end = src_start + ph->msize;
			dst_start = ph->paddr;
			dst_end = dst_start + ph->msize;
notice("%lx %lx %lx %lx\n", src_start, src_end, dst_start, dst_end);
		}
    }

    return(eh->entry);
}

