#ifndef __JVTOOLS_EXTRACTION__H__
#define __JVTOOLS_EXTRACTION__H__

/*****************************************************************************/
//
// FILE        :  jvtools_extraction.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  
//
//
// COPYRIGHT   : Jacco Vink,  2003, 2004
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_image.h"  
#include "astrophys.h"

using std::string;
using JVMessages::setFunctionID;
using JVMessages::error;
using JVMessages::success;
using JVMessages::message;
using JVMessages::warning;


typedef enum polarisationMode{
  polI=0, polQ=1, polU=2, polQvar=3, polUvar=4
} PolarisationMode;

namespace jvtools{
  class BasicData;
  class Image; //forward declaration
  class Timing;

  class ExtractionParameters
    {
    public:
      string eventFileName, gtiFileName;
      double tstart , tstop;
      int chanMin, chanMax, detNumber;
      int imageSize, binFactor, ccdnumber;
      double dx, dy, ra0, dec0, binFactorF; 
      bool phaseResolved, useradec, weighting, mhWeighing, usePolarisation;
      float minPolSignificance;
      double backScal;
      double period;
      double phaseMin, phaseMax, phaseShift;
      double sigma, chan0;
      string channelType;
      Coordinate_type wcsType;
      bool applyPSF;
      bool useMask;
      Image mask;
      unsigned long bufferSize;
      WeightingScheme polWeightingMode;
      PolarisationMode polarisation;
      
      ExtractionParameters();
      Status readMask( const char maskFileName[]);
      Status copySome( const ExtractionParameters &par );

    };//class definition


};//namespace

#endif
