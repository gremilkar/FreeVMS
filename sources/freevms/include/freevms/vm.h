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

struct memdesc
{
    vms$pointer   		base;
    vms$pointer   		end;
};

struct initial_obj
{
#   define              INITIAL_NAME_MAX    80
    char                name[INITIAL_NAME_MAX];
    char                flags;

    vms$pointer   		base;
    vms$pointer   		end;
    vms$pointer   		entry;
};

struct vms$meminfo
{
    unsigned int        num_regions;
    unsigned int        max_regions;
    struct memdesc      *regions;

    unsigned int        num_io_regions;
    unsigned int        max_io_regions;
    struct memdesc      *io_regions;

    unsigned int        num_vm_regions;
    unsigned int        max_vm_regions;
    struct memdesc      *vm_regions;

    unsigned int        num_objects;
    unsigned int        max_objects;
    struct initial_obj  *objects;
};

#define     VMS$MEM_RAM     1
#define     VMS$MEM_IO      2
#define     VMS$MEM_VM      4
#define     VMS$MEM_OTHER   8

#define     NUM_MI_REGIONS      1024
#define     NUM_MI_IOREGIONS    1024
#define     NUM_MI_VMREGIONS    1024
#define     NUM_MI_OBJECTS      1024

#define     VMS$IOF_RESERVED    0x01    // Reserved by the kernel
#define     VMS$IOF_APP         0x02    // Application
#define     VMS$IOF_ROOT        0x04    // Root server
#define     VMS$IOF_BOOT        0x08    // Boot info
#define     VMS$IOF_VIRT        0x10    // Set if memory is physical
#define     VMS$IOF_PHYS        0x20    // Set if memory is virtual
// Both VMS$IOF_VIRT and VMS$IOF_PHYS can be set for direct mapped.

#define		MAX_FPAGE_ORDER		(sizeof(L4_Word_t) * 8)

struct fpage_list
{
	L4_Fpage_t					fpage;
	TAILQ_ENTRY(fpage_list)		flist;
};

TAILQ_HEAD(flist_head, fpage_list);

struct fpage_alloc
{
	struct flist_head		flist[MAX_FPAGE_ORDER + 1];
	struct
	{
		vms$pointer			base;
		vms$pointer			end;
		int					active;
	} internal;
};

struct slab
{
	TAILQ_ENTRY(slab)		slabs;
};

struct slab_cache
{
	vms$pointer							slab_size;
	TAILQ_HEAD(sc_head, memsection)		pools;
};

struct memsection
{
	vms$pointer					magic;
	vms$pointer					base;
	vms$pointer					end;
	vms$pointer					memory_attributes;
	vms$pointer					flags;
	struct pd					*owner;
	struct thread				*server;
	struct slab_cache			*slab_cache;
	TAILQ_ENTRY(memsection)		pools;
	TAILQ_HEAD(sl_head, slab)	slabs;
};

void vms$bootstrap(struct vms$meminfo *mem_info, unsigned int page_size);
void vms$fpage_free_internal(struct fpage_alloc *alloc, vms$pointer base,
		unsigned long end);
void vms$init(L4_KernelInterfacePage_t *kip,
        struct vms$meminfo *MemInfo);
void vms$pager(void);
void *vms$slab_cache_alloc(struct slab_cache *sc);
void vms$slab_cache_free(struct slab_cache *sc, void *ptr);
