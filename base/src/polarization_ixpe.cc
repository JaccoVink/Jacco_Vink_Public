/*****************************************************************************/
//
// FILE        :  polarization.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  contains main() : application of class image.
//
//
// COPYRIGHT   : Jacco Vink, 2015
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_polarization.h"
//#include "jvtools_montecarlo.h"
#include "jvtools_messages.h"
#include "jvtools_pgplot.h"

using namespace::jvtools;
using namespace::JVMessages;

  
#define STRL 1024

/****************************************************************************/
//
// FUNCTION    : PolarizationCorrection
// DESCRIPTION : Class to deal with polarisation correction on event by event
// base.
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

PolarisationCorrection::PolarisationCorrection()
{

  //  setFunctionID("PolarizationCorrection");
  
  
  useCorrection=false;
  polf4keV = 0.43;
  energy_ev0= 1500.0;
  energy_max_ev = 15000.0;
  
  exponent=0.4;

  polf4keV /= pow( 4000.-energy_ev0, exponent);
  
  //  printf("---polff4keV= %f\n",polf4keV);
  //  printf("TESTING\n");
  
  //  success();
  
  return;
}

PolarisationCorrection::~PolarisationCorrection()
{
  return;
}
Status PolarisationCorrection::setCorrectionOn()
{
  useCorrection=true;
  
  return SUCCESS;
}

double PolarisationCorrection::getModulationFactor(double energy_ev)
{
  double factor=1.0;


  if(energy_ev > energy_ev0 && energy_ev0 < energy_max_ev)
    factor= (pow(energy_ev-energy_ev0, exponent)*polf4keV);
  else
    factor = 0.0;

  return factor;
}
double PolarisationCorrection::correctionFactor(double energy_ev)
{
  if( !useCorrection)
    return 1.0;

  double factor=1.0;
  if(energy_ev > energy_ev0)
    factor= 1.0/(pow(energy_ev-energy_ev0, exponent)*polf4keV);
  else
    factor = 0.0;
  
  return factor;
}
