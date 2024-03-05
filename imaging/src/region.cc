/*****************************************************************************/
//
// FILE        :  region.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Implementation of the  Region class.
//                Basic functions.
//
// COPYRIGHT   : Jacco Vink, 2002
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_region.h"
#include "coordinates.h"

using jvtools::Image;
using jvtools::BasicData;
using jvtools::Region;
using JVMessages::setFunctionID;
using JVMessages::error;
using JVMessages::success;
using JVMessages::message;
using JVMessages::warning;


/****************************************************************************/
//
// FUNCTION    : Region()/~Region()
// DESCRIPTION : Constructor/Destructor
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
Region::Region()
{

  exclude = 0;
  system  = 0;
  shape   = 0;
  nregions = 0;
  npoints = 0;

  xpos = ypos = size1 = size2 = 0;
  angle = 0;

}//Region()

Region::~Region()
{
  deallocate();

}//~Region()


/****************************************************************************/
//
// FUNCTION    : allocate()
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
Status Region::allocate( unsigned int nreg, unsigned int np)
{
  setFunctionID("allocate(Region)");

  if( nreg <= 0 || np <= 0 )
    return errorwarning("invalid parameters");

  if( nregions != 0 )
    deallocate( );

  npoints = new unsigned int [nreg];
  if( npoints == 0 )
    return errorwarning("error allocating regions");

  exclude = new bool [nreg];
  if( exclude  == 0 )
    return errorwarning("error allocating regions");

  system  = new RegionSystem [nreg];
  if( system == 0 )
    return errorwarning("error allocating regions");

  shape   = new Shape [nreg];
  if( shape == 0 )
    return errorwarning("error allocating regions");


  nregions = nreg;

  for(unsigned int i=0; i < nregions ; i++){
    npoints[i] = 0;
    exclude[i] = false;
    system[i]  = IMAGE;
    shape[i]   = CIRCLE;
  }// for
  
  if( BasicData::allocate( MAX(np, nregions*5) ) == SUCCESS &&
      x != 0 && y != 0 && detx !=0 && dety != 0 && dety != 0 && time != 0 ){
    // assign pointers to the allocated arrays:
    xpos  = x;
    ypos  = y;
    size1 = detx;
    size2 = dety;
    angle = time;
  }//if

  // based on rectangle xc, yc, xsize, ysize, rotation
  message("allocate #regions = ", (unsigned long)nregions);
  message("allocate #points  = ", buffer_size);

  return success();
}//allocate()




/****************************************************************************/
//
// FUNCTION    : deallocate()
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
Status Region::deallocate()
{
  setFunctionID("deallocate(Region)");

  if( nregions != 0 ){
    delete [] npoints;
    delete [] shape;
    delete [] system;
    delete [] exclude;
    nregions = 0;
    xpos = ypos = size1 = size2 = 0;
    angle = 0;
    BasicData::deallocate();
  }//if


  return success();
}//deallocate()

/****************************************************************************/
//
// FUNCTION    : world2image()
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
Status Region::world2image(const Image & image)
{
  setFunctionID("world2image");
  Coordinate_type imageType = image.getxytype();

  if( (imageType == XMM_RGS_XDSP || imageType == XMM_RGS_PI) && 
      image.cdelt1 != 0){
    RegionSystem sys;
    if( imageType == XMM_RGS_PI)
      sys = BETA_PI;
    else if( imageType == XMM_RGS_XDSP )
      sys = BETA_XDSP;


    message("The image x axis is RGS dispersion data");
    
    unsigned long int np = 0;
    for(unsigned int i=0; i < nregions; i++){
      if( system[i] == BETA_XDSP || system[i] == BETA_PI ){
	for(unsigned int j=0; j < npoints[i]; j++){
	  x[np+j] -= image.crval1;
	  x[np+j] /= image.cdelt1;
	  x[np+j] += image.crpix1;

	  y[np+j] -= image.crval2;
	  y[np+j] /= image.cdelt2;
	  y[np+j] += image.crpix2;

	}
	system[i] = IMAGE;
      }
      np += npoints[i];
    }//for

  }//(imageType == XMM_RGS_XDSP || imageType == XMM_RGS_PI) && ...


  
  return success();
}//world2image

/****************************************************************************/
//
// FUNCTION    : image2world()
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
Status Region::image2world2(const Image & image)
{

  setFunctionID("image2world");

  return success();
}// 
