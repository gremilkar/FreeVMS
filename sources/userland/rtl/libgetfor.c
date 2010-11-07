// $Id: libgetfor.c,v 1.2 2007-02-04 16:41:34 roart Exp $
// $Locker:  $

// Author. Roar Thron�s.

#include <clidef.h>
#include <cliservdef.h>
#include <descrip.h>
#include <ssdef.h>

// use clicall for this some time in the future

#ifdef __i386__
static void ** comdsc = 0x3f00000c;
#else
static void ** comdsc = 0x3f000018; 
#endif

int lib$get_foreign(void * resultant_string, void * prompt_string, short int * resultant_length, int flags) {
  // fix str$ use later
  struct dsc$descriptor * dsc = *comdsc;
  struct dsc$descriptor * resdsc = resultant_string; 
  memcpy(resdsc->dsc$a_pointer, dsc->dsc$a_pointer, dsc->dsc$w_length);
  if (resultant_length)
    (*resultant_length) = dsc->dsc$w_length;
  return SS$_NORMAL;
}

