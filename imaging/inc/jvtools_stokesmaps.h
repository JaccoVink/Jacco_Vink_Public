#ifndef __JVTOOLS_STOKESMAPS_H__
#define __JVTOOLS_STOKESMAPS_H__

/*****************************************************************************/
//
// FILE        :  jvtools_polarization.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  
//
//
// COPYRIGHT   : Jacco Vink, 2015/24
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
//
/*****************************************************************************/
#include "jvtools_image.h"
#include "jvtools_pgplot.h"


namespace jvtools{
  class StokesMaps
  {
  public:
    StokesMaps(){
      reset();
      return;
    }
    ~StokesMaps(){
      return;
    }

    Status processMaps();
    Status read( string det1cubename, bool useDetName=false);
    Status save( const string &basename, bool saveDets=false);
    Status saveVectors( const string &basename, bool magneticvectors=false, int vectorskip=5, 
			int vector=1, double minChiSqr1=6.18);
    Status rebinMaps(int binf=2);
    //    Status filterMaps(double sm, Filter filter);
    Status filterMaps(int fize, Filter filter);        
    Status applyMask(float clip=3.5);
    
    void reset(){
      b=1;
      dataSummed=false;
      filtered=false;
      filtersize=5;
      processed=false;
      masked=false;
      filterused=nofilter;
    }


    
  protected:
  private:
    bool dataSummed, filtered, processed, masked;
    int b, filtersize;
    float sigma;
    Image stokesI[4], stokesQ[4], stokesU[4];
    Image varI[4], varQ[4], varU[4];    
    Image counts[4];
    Filter filterused;
    
    // derived maps:
    Image mask;
    Image mdp99, pol_intensity, pol_fraction, chiSqrMap, polAngles;
    
    Status sum_detectors();    
  };
};


#endif
