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

#define CALL$PRINT                  1

#define CALL$NAME_SERVER

#define CALL$ALLOC_PAGE
#define CALL$FREE_PAGE

#define CALL$NEW_PROCESS
#define CALL$NEW_THREAD
#define CALL$KILL_PROCESS
#define CALL$KILL_THREAD

#define CALL$MORE_CORE
