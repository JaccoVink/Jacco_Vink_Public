#ifndef __JVTOOLS_HISTOGRAM_H__
#define __JVTOOLS_HISTOGRAM_H__

/*****************************************************************************/
//
// FILE        :  jvtools_histogram.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Definition the  Profiles class.
//                Filtering functions and other functions
//
// COPYRIGHT   : Jacco Vink,  2002
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
//#include "jvtools_basic.h"
#include "jvtools_image.h"

//#define NMAX_PEOF 4096


namespace jvtools{
  class Histogram {
    friend class Image;
  public:
    Histogram( );
    ~Histogram( );
    Status allocate( long n);
    Status deallocate( void );
    Status reset( void );
    Status setBinning(float minimum, float maximum, int n);
    Status calcErrors( void );
    double mean( void );
    double median( void );
    double rms( void );
    float maxHistogram(void );
    Status integrate( void );
    Status save( char filename[] );
    Status display( int color, int ix=1, int iy=1, int nx=1, int ny=1 );
    Status equalization( Image *image, float normalization=255.0);
  protected:
    long nbins, npixels, mostFrequentVal;
    long *histogram;
    float binwidth, min, max, mostFrequent;
    double meanValue, rmsValue;
    float *pixValue, *scaledHistogram, *cumulative, *histError, *scaledError;
  };
};

#endif
