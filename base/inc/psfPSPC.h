#ifndef __PSFPSPC_H__
#define __PSFPSPC_H__

/*****************************************************************************/
//
// FILE        :  psfPSPC.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Header file containing prototyping for the functions in
//	          psfPSPC.cc describing the ROSAT PSPC point spread function.
//
// COPYRIGHT   : Jacco Vink, SRON, 1996
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "astrophys.h"
#include "jvtools_image.h"


float totalPSPCpsf( float r, float energy );

float gaussianpsf( float r, float energy );
float exppsf( float r, float energy);
float lorentzianpsf( float r, float energy );

float gaussianfrac(float energy );
float expfrac(float energy );
float lorentzianfrac( float energy );

float psfPSPCintegral( float rmax, float energy);

Status makePSFimage( jvtools::Image &image, int size, float rmax, float energy);

#endif
