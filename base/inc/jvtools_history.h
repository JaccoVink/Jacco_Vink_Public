#ifndef __HISTORY__H__
#define __HISTORY__H__

/*****************************************************************************/
//
// FILE        :  history.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  
//
//
// COPYRIGHT   : Jacco Vink,  2002
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "fitsio.h"
#include "common.h"


#define HIST_LEN 71

namespace jvtools
{
  class History
    {
    public:
      History();// constructor
      ~History(); // destructor
      Status insertText( const char newtext[]);
      Status print( FILE *fp ) const;
      Status add2FitsFile(fitsfile *fptr) const;
    protected:
      
    private:
      char *text;
      History *next;
    };//
};

#endif
