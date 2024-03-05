#ifndef __FFTWCLASS_H__
#define __FFTWCLASS_H__
/*****************************************************************************/
//
// FILE        :  fftwclass.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Wrapper around FFTW library.
//
//
// COPYRIGHT   : Jacco Vink,  2001
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "srfftw.h"
#include "common.h"

class FFTW
{
 public:
  FFTW();
  ~FFTW();

 protected:
  fftw_complex *cpixel;
  unsigned long ncomplex; 
  rfftwnd_plan fftplan;
  fftw_direction fftwdir;

  bool ftransformed, fftplanexist;

   Status allocate_complex( fftw_direction dir= FFTW_REAL_TO_COMPLEX );
 private:

};//


#endif
