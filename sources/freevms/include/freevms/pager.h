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

struct mapped_page
{       
    L4_Fpage_t                  phys;
    L4_Fpage_t                  virt;

    struct pd                   *owner;

    vms$pointer                 flags; // VMS$MAPPED | VMS$SWAPPED
                        
    TAILQ_ENTRY(mapped_pages)   list;
};

#define VMS$MAPPED              1
#define VMS$SWAPPED             2

void vms$pagefault_init();
void vms$pagefault(L4_ThreadId_t caller, vms$pointer addr, vms$pointer ip,
        vms$pointer tag);
