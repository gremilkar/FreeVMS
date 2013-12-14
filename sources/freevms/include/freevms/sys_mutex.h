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

struct mutex
{
    L4_Word_t       holder;
	L4_Word_t		internal;
    L4_Word_t       count;
};

typedef struct mutex    *mutex_t;

extern "C"
{
    vms$pointer try_lock_amd64(vms$pointer mutex, vms$pointer myself);
}

static inline void
mutex_lock(mutex_t m)
{
    L4_Word_t       me;

    me = L4_Myself().raw;

    while(!arch_specific(try_lock)((vms$pointer) m, (vms$pointer) me))
    {
        L4_ThreadSwitch(L4_nilthread);
    }

    return;
}

static inline void
mutex_unlock(mutex_t mutex)
{
    mutex->holder = 0;
    return;
}

void sys$mutex_init(mutex_t mutex);
void sys$mutex_count_lock(mutex_t mutex);
void sys$mutex_count_unlock(mutex_t mutex);

