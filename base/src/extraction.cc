/*****************************************************************************/
//
// FILE        :  extraction.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Implementation of ExtractionParameters class
//                which basically serves as a struct with spectral/imaging
//                extraction parameters.
//
//
// COPYRIGHT   : Jacco Vink,  2004
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_extraction.h"  // definitions

using jvtools::ExtractionParameters;

/****************************************************************************/
//
// FUNCTION    : ExtractionParameters()
// DESCRIPTION : Constructor
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/****************************************************************************/
ExtractionParameters::ExtractionParameters()
{
  setFunctionID("ExtractionParameters");
  gtiFileName = "-";
  tstart    = 0.0;
  tstop     =  10.0e7;
  chanMin   = -1;
  chanMax   = 2048;
  detNumber = -1;
  channelType = "PI";
  wcsType   = SKY_XY;
  imageSize = 512;
  dx = dy   = 0.0;
  ra0 = dec0 = 0.0;
  useradec = false;
  binFactor  = 1;
  binFactorF = 1.0;
  phaseResolved=false;
  period    = 1.0;
  phaseMin  = 0.0;
  phaseMax  = 1.0;
  phaseShift= 0.0;
  useMask   = false;
  backScal  = 1.0;

  usePolarisation=false;
  minPolSignificance=3.0;
  applyPSF=false;
  
  weighting=mhWeighing=false;
  sigma=100.0;
  chan0=6700.0;
  bufferSize = DEFAULT_BUFFER_SIZE;
  mask.setChatty( false );
  success();
}//constructor()

      

/****************************************************************************/
//
// FUNCTION    : ExtractionParameters()
// DESCRIPTION : Reads fits image (mask) and checks consistency with the
//               selected WCS 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/****************************************************************************/
Status ExtractionParameters::readMask( const char maskFileName[])
{
  setFunctionID("readMask");


  JVMessages::setChatLevel();
  if(mask.readfits( maskFileName ) == FAILURE )
    return error("error reading file containing mask image");

  if( mask.getxytype()== DET_XY && wcsType == SKY_XY){
    message("Mask has DETECTOR coordinates, changing wcs accordingly");
    wcsType = DET_XY;
  }
  else if( mask.getxytype()== SKY_XY && wcsType==DET_XY)
    return error("conflicting coordinate types mask/events...");


  float min = mask.minimum();
  float max = mask.maximum();
  double total = mask.total();
  float crval1, crval2, crpix1, crpix2, cdelt1, cdelt2, crot,eq;
  mask.getCoordinates( &crval1, &crval2, &crpix1, &crpix2, &cdelt1, 
		       &cdelt2, &crot,&eq);
  message("Min   = ", min);
  message("Max   = ", max);
  message("Total = ", total);
  message("Pixel size in arcsec^2 = ", sqrt( fabs(cdelt1*cdelt2)*SQR(3600)));

  if( max > 0.0 || min != 0.0){
    backScal = total * sqrt( fabs(cdelt1*cdelt2)*SQR(3600))/max;
  }else
    return error("invalid maximum pixel size for mask image");

  return success();
}//readMask


/****************************************************************************/
//
// FUNCTION    : ExtractionParameters()
// DESCRIPTION : Reads fits image (mask) and checks consistency with the
//               selected WCS 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/****************************************************************************/
Status ExtractionParameters::copySome( const ExtractionParameters &par)
{
  setFunctionID("copySome");

  eventFileName = par.eventFileName;
  gtiFileName = par.gtiFileName;
  tstart    = par.tstart;
  tstop     = par.tstop;
  chanMin   = par.chanMin;
  chanMax   = par.chanMax;
  detNumber = par.detNumber;
  channelType = par.channelType;
  wcsType   = par.wcsType;
  imageSize = par.imageSize;
  binFactor = par.binFactor;
  period    = par.period;
  phaseShift= par.phaseShift;
  bufferSize = par.bufferSize;

  return success();
}//readMask
