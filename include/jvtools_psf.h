#ifndef __JVTOOLS_PSF_H__
#define __JVTOOLS_PSF_H__

/*****************************************************************************/
//
// FILE        :  jvtools_montecarlo.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  
//
//
// COPYRIGHT   : Jacco Vink, 2015
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
//
/*****************************************************************************/
#include "jvtools_basic.h"   


using namespace::std;
using namespace::JVMessages;


namespace jvtools{

  class PGPlot;
  
  class PSFmodel{
  public:
    PSFmodel();
    Status setFraction12( float f);
    Status setSigma1(float s);
    Status setSigma2(float s);
    Status toggleSubpixelRnd();
    Status redistribute(float *x, float *y);
    Status plotPSF( PGPlot *window, unsigned long int nevents=1000000);
    void useKingProfile(){
      kingprofile=true;
    }
    Status setPixScale(float p){
      pixscale=p;
      return SUCCESS;
    }
  protected:
  private:
    bool subpixrnd, kingprofile;
    float sigma1, sigma2, f12, eta, pixscale;
  };

}

#endif
