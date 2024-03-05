/*****************************************************************************/
//
// FILE        :  backgroundfilter.cc
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
#include "jvtools_image.h"
#include "jvtools_timing.h"
#include "jvtools_messages.h"


using namespace::jvtools;
using namespace::JVMessages;


/****************************************************************************/
//
// FUNCTION    : backgroundFilter()
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status BasicData::backgroundFilter( const string &eventFile, 
				    const string &gtiFile,
				    const string &outputName,
				    double tstart, double tstop, 
				    bool epicpn, bool epicmos,
				    Coordinate_type xy_type )
{
  setFunctionID("backgroundFilter");

  Image emptymask;
  Timing curve;
  Instrument instr = UNDEF;
  //  int ccdnr = 1;
  double meanCountRate, countRateRMS;
  double maxcountrate = -100.0;

  message("making lightcurve to search for high background intervals");
  message(" Input GTI File = ", gtiFile.c_str());

  curve.copy_badpix( *this );
  curve.copy_GTIs( *this);

  //  curve.extractCurve(event_file, gti_file, emptymask, 50.0, tstart, 
  //		     tstop, 10000, 12000, ccdnr, xy_type, "PI");

  setChatLevel( JVMessages::CHATTY);
 

  printf("tstart = %e\n", tstart);
  printf("tstop = %e\n", tstop);
  
  curve.extractCurve( eventFile.c_str(), gtiFile.c_str(), 
		      emptymask, 50.0,
		      tstart, tstop,
		      10000,120000,
		      -1, xy_type, "PI");

  instr = curve.get_instrument();
  meanCountRate = curve.meanRate();
  countRateRMS  = curve.rateRMS();

  message("mean count rate = ", meanCountRate);
  message("rms count rate = ", countRateRMS);
  maxcountrate = meanCountRate + 3.0 * countRateRMS;

  message("maximum count rate level = ", maxcountrate);

#if 0
  //    if( instr == XMM_PN || instr == XMM_MOS1 || instr == XMM_MOS2 ){
  //      tstart = 0.0;
  //      tstop  = 1e5;
  //    }//if
  switch( instr ){
  case XMM_PN:
    maxcountrate = DEFAULT_PN_MAX_CNTRATE;
    break;
  case XMM_MOS1:
    maxcountrate = DEFAULT_MOS1_MAX_CNTRATE;
    break;
  case XMM_MOS2:
    maxcountrate = DEFAULT_MOS2_MAX_CNTRATE;
    break;
  default:
    maxcountrate = DEFAULT_MAX_CNTRATE;
  }//switch
#endif
  //  exit(3);
  curve.print( stdout);
  curve.findFlares( maxcountrate );

  //  curve.findFlares( -1.0);
  //    curve.write_gti("temp.fits", -1, true);

  copy_GTIs( curve);

  //  if(!(epicpn == true || epicmos== true) )
  //    strncpy(gti_file, IGNORE_STRING, strlen(IGNORE_STRING));
  
  //  curve.save("lightcurve1.fits");
  //  curve.writeGTI(outputname);

  // message(" Output GTI File = %s\n", gti_file);
  return success();

}//backgroundfilter()






