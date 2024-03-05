/*****************************************************************************/
//
// FILE        :  image_polarisation.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Implementation of the  Image class.
//                Filtering functions and other functions
//
// COPYRIGHT   : Jacco Vink, UvA, 2022
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_image.h"
//#include "nr.h"

#include <math.h>

using namespace::JVMessages;
using namespace::jvtools;


/****************************************************************************/
//
// FUNCTION    : polarisationAngle
// DESCRIPTION : Makes a map of the polarisation angle based on
//               input Q & U maps
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//    stokesQ           I                       stokes Q map
//    stokesU           I                       stokes U map
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::polarisationAngle(const Image &stokesQ, const Image &stokesU)
{
  setFunctionID("polarisationAngle");

  if ( (stokesQ.nx * stokesQ.ny <= 0) || (stokesQ.nx != stokesU.nx) || (stokesQ.ny != stokesU.ny) ){
    return error("invalid stokes Q and U input images");
  }

  *this = stokesQ; // sets automatically keywords and allocation of array right
  *this *= 0.0; // initialise
  message("Angle image nx = ", nx);
  message("Angle image ny = ", ny);
  
  for(int j=0; j< ny; j++){
    for(int i=0; i< nx; i++){
      pixel[i+j*nx] = 0.5* atan2( stokesU.pixel[i+j*nx], stokesQ.pixel[i+j*nx]);
    }//for
  }
  
  return success();
}


/****************************************************************************/
//
// FUNCTION    : radiallyRotQU
// DESCRIPTION : rotate Q and U maps around an axis x0, y0
//
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//    stokesQ           I/O                       stokes Q map
//    stokesU           I/O                       stokes U map
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::radiallyRotQU( Image *stokesQ, Image *stokesU, float x0, float y0)
{
  setFunctionID("radiallyRotQU");

  warning("Q and U maps will be rotated according to radially symmetry");

  if( (stokesQ->nx != stokesU->nx) || (stokesQ->ny != stokesU->ny) ||
      (stokesQ->nx <=  0) || (stokesU->ny <= 0)){
    return error("invalid stokes Q and U images");
  }

  *this = *stokesQ;

  
  warning("at y=0 Q and U are unaffected. pixel coordinates according to fits convention (origin 1,1)");
  message("x0 = ",x0);
  message("y0 = ",y0);

  x0 -= 1.0; // fits vs c++
  y0 -= 1.0;

  float dx, dy;
  float oldQ, oldU;
  float newQ=0.0, newU=0.0, phi=0.0;
  for(int j=0; j< nx; j++){
    for(int i=0; i< nx; i++){

      dx = (float)i - x0;
      dy = (float)j - y0;      
      phi = -2*atan2( dy, dx); // counteract rotation, hence - sign

      oldQ = stokesQ->pixel[i+j*nx];
      oldU = stokesU->pixel[i+j*nx];
      
      newQ = oldQ * cos(phi) - oldU * sin(phi);
      newU = oldQ * sin(phi) + oldU * cos(phi);

      if( (i+1 == 8 && j+1 == 20) || (i+1 == 14 && j+1 == 26) ){ 
	printf("\t*** %i %i dx=%.2f dy=%.2f phi=%.3f deg\n",i, j, dx, dy, phi*180/M_PI);
	printf("\t*** Old: Q=%.2f U=%.2f; New: Q=%.2f U=%.2f\n", oldQ, oldU, newQ, newU);
      }

      
      stokesQ->pixel[i+j*nx] = newQ;
      stokesU->pixel[i+j*nx] = newU;
      
    }//for i
  }//for j
  return success();
}






/****************************************************************************/
//
// FUNCTION    : generatePolarisationVectors
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
Status generatePolarisationVectors(const Image &polAngleMap, const Image &mask,
				   const Image &pvalMap, float minP,
				   const string &outputName)
{
  setFunctionID("generatePolarisationVectors");

  int nx, ny, nx2, ny2;
  float min, max;
  
  polAngleMap.getnxny(&nx, &ny);
  mask.getnxny(&nx2, &ny2);


  message("nx = ", nx);
  message("ny = ", ny);

  if( nx*ny <= 0 || (nx != nx2) || (ny != ny2)){
    return error("invalid input images (zero pixels, or mask and angle map do not agree)");
  }
  
  min=polAngleMap.minimum();
  max=polAngleMap.maximum();
  message("minimum angle = ", min);
  message("maximum angle = ", max);
  if( (fabs(min)>  M_PI/2.0 ) || (max > M_PI/2) ){
    return error("angle map should be in radians and be confined to -pi/2 <= phi <=pi/2", M_PI/2);
  }

  FILE *fp;
  string filename=outputName + "_pol_vectors.reg";
  float angle=0.0;

  message("saving region file ", filename);
  if( !(fp=fopen(filename.c_str(), "w")) ){
    return error("error opening file ", filename.c_str() );
  }

  fprintf(fp,"# Region file format: DS9 version 4.1\n");
  fprintf(fp,"global color=green dashlist=8 3 width=1 font=\"helvetica 10 normal roman\" select=1 highlite=1 dash=0 fixed=0 edit=1 move=1 delete=1 include=1 source=1\n");
  fprintf(fp,"image\n");


  float dx=0.5, dy=0.5, x0, y0, x1, y1, x2, y2, l=0.5;
  for(int j=1; j<= ny; j++){ // fits convention i starts at 1
      for(int i=1; i<= nx; i++){
	if( mask.get(i,j) > 0.0 ){
	  angle = polAngleMap.get(i,j);

	  x0 = (float)i;
	  y0 = (float)j;	  
	  x1 = x0 - l * cos(angle);
	  y1 = y0 - l * sin(angle);
	  x2 = x0 + l * cos(angle);
	  y2 = y0 + l * sin(angle);	  	  
	  //	  fprintf(fp,"# vector(%.2f,%.2f, %.4f, %.2f) vector=0\n", (float)i, (float)j , 2.0, angle * 180/M_PI);
	  if( pvalMap.get(i,j) >= minP ){
	    fprintf(fp,"line(%.2f,%2.f,%.2f,%.2f) # line=0 0 color=red width=3\n", x1, y1, x2, y2);	    
	  }else{
	    fprintf(fp,"line(%.2f,%2.f,%.2f,%.2f) # line=0 0\n", x1, y1, x2, y2);
	  }
	    //	  printf("%3i %3i angle = %.3f deg\n", i, j, angle * 180/M_PI);
	}
      }//for i
  }

  fclose(fp);
  return success();
}


/****************************************************************************/
//
// FUNCTION    : 
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
Status Image::chiSqr_to_Pvalue(const Image &chiSQR)
{
  setFunctionID("chiSqr_to_Pvalue");


  if( chiSQR.nx <= 0 || chiSQR.ny<= 0 || chiSQR.minimum() < 0.0 ){
    return error("invalid input image (empty, or negative values)");
  }

  //  std::chi_squared_distribution<double> distr(2.0);


  //  prinf("dof = %f\n", distr.n());
  //  prinf("dof = %f\n", distr.n());

  //  double chisqr(int Dof, double Cv); //forward reference
  
  *this = chiSQR;

  //uses the fact that a ChiSQR distr with dof=2 we have P(c)= exp(-c*0.5):

  double c=10.597;  
  for(int j=0; j<ny; j++){
    for(int i=0; i<nx; i++){
      c= chiSQR.pixel[i+j*nx];
      if( c > 0 )
	pixel[i + j*nx] = 1.0 - exp(-1.0 * c*0.5) ;
    else
      pixel[i + j*nx] = 0.0;
    }
  }
  

#if 0
  printf("***** chisqr= %f\n",  exp(-1.0 * c*0.5) );
  c=9.210;
  printf("***** chisqr= %f\n",  exp(-1.0 * c*0.5) );  
  c=  4.605;
  printf("***** chisqr= %f\n",  exp(-1.0 * c*0.5) );  
#endif
  
  return success();
}


