#ifndef __FFTCLASS_H__
#define __FFTCLASS_H__

/*****************************************************************************/
//
// FILE        :  fftclass.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Wrapper class for Num. Recipes FFT.
//
//
// COPYRIGHT   : Jacco Vink,  1999
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include <stdio.h>
#include "nr.h"

class FFTclass
{
 public:
  FFTclass( void );
  FFTclass( const FFTclass &fftobj ); // copy constructor
  FFTclass( const float array[], int nx, int ny);
  ~FFTclass( void );
  FFTclass &operator=( const FFTclass &fftobj );
  Status operator*=( const FFTclass &fftobj );
  friend FFTclass operator*( const FFTclass &fftobj1, const FFTclass &fftobj2);

  Status fft( void );
 protected:
 private:
  int isign;
  unsigned long nn[2];
  float *complex_array;

  Status allocate(int nx, int ny);
  Status deallocate( void );

}; FFTclass

#endif
