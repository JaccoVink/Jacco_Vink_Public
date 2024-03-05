/*****************************************************************************/
//
// FILE        :  jvtools_gti.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  X-ray astronomy base class.
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
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <string>
#include "fitsio.h"
#include "common.h"
#include "xray.h"




namespace jvtools{
  class BasicData;
  class Timing;
  class GoodTimeIntervals{
    friend class BasicData;
    friend class Timing;
  public:
    GoodTimeIntervals(){ 
      nGTI = 0; startGTI = stopGTI = 0; instrumentID = UNDEF; detectorID = -1; };
    GoodTimeIntervals(long n);
    GoodTimeIntervals( const GoodTimeIntervals &gti );
    ~GoodTimeIntervals(){ deallocate(); };
    GoodTimeIntervals & operator=( const GoodTimeIntervals &gti );
    Status allocate(long n);
    Status deallocate( void );
    Status setInstrumentID( Instrument instr);
    double totalExposure();
    double cumulativeExposure(double endTime);
    Status excludeInterval( double tbegin, double tend );
    Status read( const char filename[], int chip = -1 );
    Status save( const char filename[]);
    Status print( FILE *fp );
  protected:
  private:
    long nGTI;
    double *startGTI, *stopGTI, exposure;
    int detectorID;
    Instrument instrumentID;
  };
};//namespace
