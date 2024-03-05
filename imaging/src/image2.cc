/*****************************************************************************/
//
// FILE        :  image2.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Implementation of the  Image class.
//                This file contains the more sophisticated functions.
//
// COPYRIGHT   : Jacco Vink, SRON, 1998, Columbia Universtity, 2002
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
/* 
   CONTAINS:

   Status Image::sourcestat()
   Status Image::getflux()
   Status Image::getflux2()
   Status Image::mosaic()
   Status Image::mosaic_limits()
   Status Image::exposuremap()
   Status Image::newcoord()
   Status Image::newcoord2()
   Status Image::sin2tan()
   Status Image::getXorY()
   Status Image::concat_h()
   Status Image::concat_v()
*/
#include "jvtools_image.h"
#include "coordinates.h"
#include "common.h"

using namespace::JVMessages;
using namespace::jvtools;



/*****************************************************************************/
//
// FUNCTION    : getCentroid
// DESCRIPTION : 
//               
//               
//               
//         
//
// SIDE EFFECTS: UNDER CONSTRUCTION
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  x0, y0               I
//  rad                  I
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::getCentroid(float x0, float y0, float rad, 
			  float *xc, float *yc, float *xyc, 
			  float *x2c, float *y2c, float *ntotal ) const
{
  setFunctionID("getCentroid");

  x0 -= 1.0; // fits to c
  y0 -= 1.0;
  if( nx <= 2 || ny <= 2 || x0 < 0 || x0 >= nx || y0 < 0 || y0 >= ny ||
      rad <= 1.2 ){
    message("nx = ", nx);
    message("ny = ", ny);
    message("x0 = ", x0);
    message("y0 = ", y0);
    message("radius = ", rad);
    return error("invalid image or input parameters");
  }

  int i1 = MAX( (int)(x0 - rad - 0.5) - 1, 0);
  int j1 = MAX( (int)(y0 - rad - 0.5) - 1, 0);
  int i2 = MIN( (int)(x0 + rad + 0.5) - 1, nx-1);
  int j2 = MIN( (int)(y0 + rad + 0.5) - 1, ny-1);

  float pixval, rad2=rad*rad, sum=0.0, sumx=0.0, sumy=0.0;
  int i;
  for(int j=j1 ; j <= j2; j++){
    for( i=i1; i<= i2; i++)
      if( SQR(i-x0) + SQR(j-y0) < rad2 ){
	pixval=pixel[i + j*nx];
	sum  += pixval;
	sumx += (i-x0)*pixval;
	sumy += (j-y0)*pixval;
      }//if
  }//for
  if( sum > 0.0 ){
    *xc = sumx/sum + x0;
    *yc = sumy/sum + y0;
  }
  else{
    *xc = x0;
    *yc = y0;
    return error("only empty pixels, or negative values encountered");
  }//if

  float sumxx=0.0, sumyy=0.0, sumxy=0.0;
  for(int j=j1 ; j <= j2; j++){
    for( i=i1; i<= i2; i++)
      if( SQR(i-x0) + SQR(j-y0) < rad2 ){
	pixval=pixel[i + j*nx];
	sum  += pixval;
	sumxx += SQR(i- *xc)*pixval;
	sumyy += SQR(j- *yc)*pixval;
	sumxy += (i- *xc)*(j- *yc)*pixval;
      }//if
  }//for

  *xyc = sumxy/sum;  
  *x2c = sumxx/sum;  
  *y2c = sumyy/sum;  
  *ntotal = sum;

  *xc += 1.0; //c to FITS convention
  *yc += 1.0; 
  return success();
}//centroid( )


/*****************************************************************************/
//
// FUNCTION    : sourcestat()
// DESCRIPTION : Calculates the bgd subtracted flux from a source within a
//               circle. Also the moments of the source up to the 3rd degree
//               are calculated
//               
//         
//
// SIDE EFFECTS: UNDER CONSTRUCTION
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  x0, y0               I
//  rad                  I
//  xc, yc               O
//  ncount               O
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::sourcestat( double x0, double y0, double rad, 
			  Moments *pos, double *ncounts, 
			  double *nbgd, bool detBgd )
{

  setFunctionID("sourcestat");

  if( nx <= 0 || ny <= 0 || rad <= 0.0 )
    return error("no image present");

  int i, j, i0, j0, in, jn, npix, npixbgd;
  double xtot, ytot, xx, yy, xy, ntot, rsqr, rbgd, rbgdsqr;

  rsqr  = rad*rad;
  rbgd = sqrt(2.0) * rad;
  rbgdsqr = 2.0 * rad * rad;

  x0 -= 1.0; // From FITS to C convention
  y0 -= 1.0;

  i0 = (int)( x0 - rbgd - 1.0); // Note the FITS convention
  j0 = (int)( y0 - rbgd - 1.0); 
  in = (int)( x0 + rbgd + 1.0); 
  jn = (int)( y0 + rbgd + 1.0); 
  i0 = MAX( i0, 0 );
  j0 = MAX( j0, 0 );
  in = MIN( in, nx-1 );
  jn = MIN( jn, ny-1 );

  if( i0 == 0 || j0 == 0 || in == nx-1 || jn == ny-1){
    warning("selection circle lies at the edge of the image!");
  }// if

  xtot = ytot = ntot = xx = yy = xy = 0.0;
  npix = npixbgd = 0;
  *nbgd = 0.0;
  for( j = j0 ; j < jn  ; j++){
    for( i = i0 ; i < in  ; i++){

      if( SQR(i - x0) + SQR(j - y0) <= rsqr ){
	xtot += (i - x0) * pixel[i + j*nx];
	ytot += (j - y0) * pixel[i + j*nx];
	xx   += SQR(i - x0) * pixel[i + j*nx];
	yy   += SQR(j - y0) * pixel[i + j*nx];
	xy   += (i - x0) * (j - j0)* pixel[i + j*nx];
	ntot += pixel[i + j*nx];
	npix++;
      }
      else{
	if(  SQR(i - x0) + SQR(j - y0) <= rbgdsqr ){
	  *nbgd +=  pixel[i + j*nx];
	  npixbgd++;
	}//if
      }//else

    }//for i
  }//for

  if( ntot == 0.0 )
    return error("division by zero encountered!");


  pos->xc = xtot/ntot;
  pos->yc = ytot/ntot;
  pos->xx = xx/ntot - SQR(pos->xc);
  pos->yy = yy/ntot - SQR(pos->yc);
  pos->xy = xy/ntot - pos->xc * pos->yc;

  pos->xc += x0 + 1.0; // +1 for the C to FITS conversion
  pos->yc += y0 + 1.0;
  *ncounts = ntot;

  if( npixbgd !=0 ){
    *nbgd *= (double)npix/ (double)npixbgd;
  }//if

  return success();
}// end sourcestat()

/****************************************************************************/
//
// FUNCTION    : getflux()
// DESCRIPTION : Get the flux from a circular region. 
//               Subtracts background using an annulus surrounding the region.
//               Assumes the image contains counts and poissonian errors.
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::getcnts( int ic, int jc, float radius, 
		       float *cnts, float *err, 
		       bool bgdsubtr) const
{
  setFunctionID("getcnts");
  if( radius < 1.0 || ic < 0 || jc < 0 || ic >= nx || jc >= ny )
    return error("requested pixel coordinates or input parameters are wrong");


  int i, j, i0, in, j0, jn; 
  long npixels, nbgdpixels;
  float r, bgd, corr, outerradius = sqrt(2.0)*radius;

  ic -= 1; // fortran/fits convention to C
  jc -= 1;
  i0 = MAX( (ic - (int)(outerradius+0.5)), 0 );
  j0 = MAX( (jc - (int)(outerradius+0.5)), 0 );
  in = MIN( (ic + (int)(outerradius+1.5)), nx);
  jn = MIN( (jc + (int)(outerradius+1.5)), ny);



  *cnts = 0.0;
  bgd   = 0.0;
  npixels = 0;
  nbgdpixels = 0;

  for( j = j0 ; j < jn ; j++){
    for( i = i0 ; i < in; i++){

      r = sqrt( (float)(SQR(i-ic) + SQR(j-jc)) );
      if( r <= radius ){
	npixels++;
	*cnts += pixel[i + j*nx];
      }
      else{
	if( bgdsubtr== TRUE && r <= outerradius ){
	  nbgdpixels++;
	  bgd += pixel[i + j*nx];
	}//if
      }//else
    }
  }//for j

  //  printf("%d %d %d %d | %d %d %f %f\n", i0, j0, in, jn, npixels,nbgdpixels, *flux, bgd);

  if( (bgdsubtr==TRUE && nbgdpixels <= 0) || npixels <= 0)
    return error("serious error");


  if( bgdsubtr == TRUE ){
    corr = (float)npixels/nbgdpixels;
    if( *cnts + corr*corr*bgd > 0.0 )
      *err = sqrt( *cnts + corr*corr*bgd);
    *cnts -= bgd*corr;
  }
  else
    *err = sqrt( *cnts);

  return success();
}// getflux()

/****************************************************************************/
//
// FUNCTION    : getflux2()
// DESCRIPTION : Same as getflux(), but this one is periodic.
//
// SIDE EFFECTS: NOT YET TESTED!!!
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::getflux2( int ic, int jc, float radius, 
			    float *flux, float *err)
{
  setFunctionID("getflux2");
  if( radius < 1.0 || ic < 0 || jc < 0 || ic >= nx || jc >= ny || 
      radius > 0.7*nx || radius > 0.7*ny )
    return error("requested pixel coordinates or input parameters are wrong!");


  int i, j, i0, in, j0, jn, ix, jx; 
  long npixels, nbgdpixels;
  float r, bgd, corr, outerradius = sqrt(2.0)*radius;

  ic -= 1; // fortran/fits convention to C
  jc -= 1;
  i0 = ic - (int)(outerradius+0.5);
  j0 = jc - (int)(outerradius+0.5);
  in = ic + (int)(outerradius+1.5);
  jn = jc + (int)(outerradius+1.5);



  *flux = 0.0;
  bgd   = 0.0;
  npixels = 0;
  nbgdpixels = 0;

  for( j = j0 ; j < jn ; j++){
    for( i = i0 ; i < in; i++){
      r = sqrt( (float)(SQR(i-ic) + SQR(j-jc)) );

      if( i < 0 )
	ix = nx + i;
      else
	if( i >= nx )
	  ix = i-nx;
	else
	  ix = i;

      if( j < 0 )
	jx = ny + j;
      else
	if( j >= ny )
	  jx = j - ny;
	else
	  jx = j;

      if( r <= radius ){
	npixels++;
	*flux += pixel[ix + jx * nx];
      }
      else{
	if( r <= outerradius ){
	  nbgdpixels++;
	  bgd += pixel[ix + jx * nx];
	}//if
      }//else
    }
  }//for j

  //  printf("%d %d %d %d | %d %d %f %f\n", i0, j0, in, jn, npixels,nbgdpixels, *flux, bgd);

  if( nbgdpixels <= 0 || npixels <= 0)
    return error("serious bug");

  corr = (float)npixels/nbgdpixels;

  if( *flux + corr*corr*bgd > 0.0 )
    *err = sqrt( *flux + corr*corr*bgd);
  else
    *err = 0.0;

  *flux -= bgd*corr;

  return success();
}// getflux2()


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
float Image::getCountRate(float xc, float yc,
			 float radius, float bgdradius, float *error)
{
  setFunctionID("getCountRate");

  if( nx <= 0 || ny <= 0 ){
    JVMessages::error("no image data");
    return 0.0;
  }
  
  if( bgdradius <= radius ){
    JVMessages::error("background radius must be larger than radius");
    return 0.0;
  }
  xc -= 1.0; // from FITS to C++ convention
  yc -= 1.0; // from FITS to C++ convention

  
  int i1= MAX( (int)( xc - bgdradius - 1.), 0);
  int i2= MIN( (int)( xc + bgdradius + 1.), nx-1);
  int j1= MAX( (int)( yc - bgdradius - 1.), 0);
  int j2= MIN( (int)( yc + bgdradius + 1.), ny-1);

  if( i1 == 0 || i1 == nx-1 || j1 == 0 || j2 == ny-1)
    warning("The source is at the edge of the image");
  if( exposure <= 0.0 ){
    warning("Exposure time not defined, count rate will be signal");
    exposure=1.0;
  }
  
  float rsqr=SQR(radius);
  float rbgdsqr=SQR(bgdradius);
  float sum=0.0, bgdsum=0.0;
  int npixels=0, nbgdpixels=0;

  for( int j=j1; j <= j2; j++){
    for( int i=i1; i <= i2; i++){
      if( SQR((float)i-xc ) + SQR((float)j - yc) <= rsqr ){
	sum += pixel[i + j*nx];
	npixels++;
      }else if( SQR((float)i-xc ) + SQR((float)j - yc) <= rbgdsqr ){
	bgdsum += pixel[i + j*nx];
	nbgdpixels++;
      }
    }
  }

  *error= sqrt( sum + bgdsum*npixels/(float)nbgdpixels );
  sum -= bgdsum * (float)(npixels)/(float)(nbgdpixels);
  
  success();

  *error /= exposure;
  return sum/exposure;
}

/*****************************************************************************/
//
// FUNCTION    : newcoord()
// DESCRIPTION : The image (*this) will be placed on a new coordinate system.
//
//
//
// SIDE EFFECTS: UNDER CONSTRUCTION
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      im                I                      image
//     dx0,dy0            I                    Allow for small shifts
//                                             (default is 0.0)
//     interpolate        I                    Interpolate between pixels?
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::newcoord( const Image &im, double dx0, double dy0, 
			bool interpolate )
{
  setFunctionID("newcoord");
  if( cdelt1 == 0.0 || cdelt2 == 0.0 || 
      (projectiontype != RADECTAN && projectiontype != RADECSIN  ) ){
    return error("error in the coordinate system");
  }

  double icorr, jcorr;
  double phi, theta, rt, x, y, alpha, delta, dx, dy;
  double ipc001001, ipc001002, ipc002001, ipc002002, det;
  double ap = crval1*DEG2RAD, deltp = crval2* DEG2RAD;
  double phip = longpole*DEG2RAD;
  Image newim( im );

  newim *= 0.0;
  newim.exposure = exposure;
  newim.mjd_obs = mjd_obs;
  strncpy(newim.instrument, instrument, FLEN_VALUE);
  strncpy(newim.telescope, telescope, FLEN_VALUE );
  strncpy(newim.object, object, FLEN_VALUE);
  strncpy(newim.obsdate, obsdate, FLEN_VALUE);


  // make the inverted pc matrix:
  det = im.pc001001*im.pc002002 - im.pc002001*im.pc001002; // determinant
  if(det == 0.0 ){
    warning("newcoord(): Error in the input image PC matrix!");
    return FAILURE;
  } //if
  ipc001001 =  im.pc002002/det; // inverted PC matrix of the input image
  ipc001002 = -im.pc001002/det;
  ipc002001 = -im.pc002001/det;
  ipc002002 =  im.pc001001/det;

  int i, newi, newj, i2, j2;
  for( int j = 0 ; j < ny ; j++ ){
    for( i = 0; i < nx ; i++){
      // first calculate phi, theta (i,j)
      //
      // (note that the pixels start by convention on 1 instead 0)
      icorr = (double)i + 1.0 - crpix1;
      jcorr = (double)j + 1.0 - crpix2;
      x =  (pc001001 * icorr + pc001002* jcorr) * cdelt1;
      y =  (pc002001 * icorr + pc002002* jcorr) * cdelt2;
      rt     = sqrt (x*x + y*y );               // Rtheta in GC
      phi   = atan2( x, -y);                   // argument of x and y
      switch( projectiontype ){
      case RADECTAN:
	theta = atan( RAD2DEG/rt );
	break;
      case RADECSIN:
	theta = acos( rt * DEG2RAD );
	break;
      default:
	//	  warning("newcoord(): projectionsystem not set!");
	//	  return FAILURE;
	warning("mosaic(): projectionsystem not set! Assume RA/DEC-TAN.");
	theta = atan( RAD2DEG/rt );
      }//switch();

      //
      // calculate alpha and delta
      ch_coord(phi, theta, phip, ap, deltp, &alpha, &delta);

      if( equinox != 2000.0 ){
	toequinox2000( &alpha, &delta, equinox );
      }

      //
      // convert to the new system
      //
      ch_coord(alpha, delta, ap, phip, deltp, &phi, &theta);

      if( theta != 0.0 ){
	switch( newim.projectiontype ){ 
	case RADECTAN: 
	  rt = RAD2DEG/tan( theta );
	  break;
	case RADECSIN:
	  rt = RAD2DEG * cos( theta );
	  break;
	default:    
	  warning("mosaic(): projectionsystem not set! Assume RA/DEC-TAN.");
	  theta = atan( RAD2DEG/rt );
	  //	  warning("newcoord(): projectionsystem not set!");
	  //	  return FAILURE;
	}//switch()
      }
      else{
	warning("newcoord(): division by zero error!");
      }

      x = rt * sin( phi ) ;
      y =  -rt * cos( phi );

      // Note the 1.0: from fortran to c convention
      //
      icorr = x/newim.cdelt1;
      jcorr = y/newim.cdelt2;
      newi = (int)(ipc001001*icorr + ipc001002*jcorr + 
		   newim.crpix1 - 1.0 + dx0 + 0.5);
      newj = (int)(ipc002001*icorr + ipc002002*jcorr + 
		   newim.crpix2 - 1.0 + dy0 + 0.5);
      // Note that we do not yet make an interpolation to surrounding pixels:
      // in future this will be done
      //
      if( interpolate == FALSE ){
	if( newi >= 0 && newi < newim.nx && newj >= 0 && newj < newim.ny){
	  newim.pixel[ newi + newj * newim.nx ] += pixel[i + j * nx];
	}// if
      }
      else{
	dx = ipc001001*icorr + ipc001002*jcorr + newim.crpix1 - 1.0 + dx0 - newi;
	dy = ipc002001*icorr + ipc002002*jcorr + newim.crpix2 - 1.0 + dy0 - newj;
	if( dx >= 0.0 ){
	  i2 = newi + 1;
	}
	else{
	  i2 = newi -1;
	}
	if( dy >= 0.0 ){
	  j2 = newj + 1;
	}
	else{
	  j2 = newj - 1;
	}
	dx = fabs(dx);
	dy = fabs(dy);
	//
	// The best matching pixel:
	if( newi >= 0 && newi < newim.nx && newj >= 0 && newj < newim.ny){
	  newim.pixel[newi + newj*newim.nx] += (1.0-dx)*(1.0-dy)*pixel[i+j*nx];
	}// if
	
	// Interpolating horizontally
	if( i2 >= 0 && i2 < newim.nx && newj >= 0 && newj < newim.ny){
	  newim.pixel[i2 + newj*newim.nx] += dx * (1.0-dy) * pixel[i + j*nx];
	}// if
	
	// Interpolating vertically
	if( newi >= 0 && newi < newim.nx && j2 >= 0 && j2 < newim.ny){
	  newim.pixel[newi + j2*newim.nx] += (1.0-dx)*dy * pixel[i + j*nx];
	}// if
	
	// Final interpolation
	if( i2 >= 0 && i2 < newim.nx && j2 >= 0 && j2 < newim.ny){
	  newim.pixel[i2 + j2*newim.nx] += dx * dy * pixel[i + j*nx];
	}// if
      }//if else

    }//for i
  }// for j

  *this = newim;
  reset_detector_keywords();

  return SUCCESS;

}// newcoord()

/*****************************************************************************/
//
// FUNCTION    : newcoord2()
// DESCRIPTION : The image (*this) will be placed on a new coordinate system.
//               Idem as newcoord(), but should eventually replace it.
//
//
// SIDE EFFECTS: UNDER CONSTRUCTION
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      im                I                      image
//     dx0,dy0            I                    Allow for small shifts
//                                             pixel size for new image
//                                             (default is 0.0)
//     interpolate        I                    Interpolate between pixels?
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::newcoord2( const Image &im, double dx0, double dy0, 
			 bool interpolate )
{
  setFunctionID("newcoord2");

  if( cdelt1 == 0.0 || cdelt2 == 0.0 || 
      (projectiontype != RADECTAN && projectiontype != RADECSIN  ) ||
      (im.projectiontype != RADECTAN && im.projectiontype != RADECSIN  ) ){
    message("cdelt1 = ", cdelt1);
    message("cdelt2 = ", cdelt2);

    message("ctype1 1 = ", ctype1);
    message("ctype2 1 = ", ctype2);
    message("Object 1 = ", object);
    message("projection type 1 = ", projectiontype);


    message("ctype1 2 = ", im.ctype1);
    message("ctype2 2 = ", im.ctype2);
    message("Object 2 = ", im.object);
    message("projection type 2 = ", im.projectiontype);
    return error("error in the coordinate system!");
  }


  if( interpolate == TRUE && 
      ( fabs((cdelt1 - im.cdelt1)/cdelt1) > 0.05 || 
	fabs((cdelt2 - im.cdelt2)/cdelt2) > 0.05) ){
    warning("unequal pixel sizes; interpolation turned off!");
    interpolate = FALSE;
  }//if

  int i, j, oldi, oldj, i2, j2;
  double icorr, jcorr;
  double phi, theta, rt, x, y, alpha, delta, dx, dy;
  double ipc001001, ipc001002, ipc002001, ipc002002, det;
  double ap = crval1*DEG2RAD, deltp = crval2* DEG2RAD;
  double phip = longpole*DEG2RAD; 
  Image newim( im );

  newim *= 0.0; // important all pixels should be 0
  newim.exposure = exposure;
  newim.mjd_obs = mjd_obs;
  strncpy(newim.instrument, instrument, FLEN_VALUE);
  strncpy(newim.telescope, telescope, FLEN_VALUE );
  strncpy(newim.object, object, FLEN_VALUE);
  strncpy(newim.obsdate, obsdate, FLEN_VALUE);

  dx0 *= -fabs(newim.cdelt1/cdelt1);
  dy0 *= -fabs(newim.cdelt2/cdelt2);

  // make the inverted pc matrix:
  det = pc001001 * pc002002 - pc002001 * pc001002; // determinant

  if(det == 0.0 )
    return error("error in the input image PC matrix");

  ipc001001 =  pc002002/det; // inverted PC matrix of the input image
  ipc001002 = -pc001002/det;
  ipc002001 = -pc002001/det;
  ipc002002 =  pc001001/det;

  for( j = 0 ; j < newim.ny ; j++ ){
    for( i = 0; i < newim.nx ; i++){
      // first calculate phi, theta (i,j)
      //
      // (note that the pixels start by convention on 1 instead 0)
      icorr = (double)i + 1.0 - newim.crpix1;
      jcorr = (double)j + 1.0 - newim.crpix2;
      x =  (newim.pc001001 * icorr + newim.pc001002 * jcorr) * newim.cdelt1;
      y =  (newim.pc002001 * icorr + newim.pc002002 * jcorr) * newim.cdelt2;
      rt  = sqrt (x*x + y*y );               // Rtheta in GC
      phi = atan2( x, -y);                   // argument of x and y
      switch( newim.projectiontype ){
      case RADECTAN:
	theta = atan( RAD2DEG/rt );
	break;
      case RADECSIN:
	theta = acos( rt*DEG2RAD );
	break;
      default:
	  return error("projectionsystem not set");;
      }//switch();

      //
      // calculate alpha and delta
      ch_coord(phi, theta, phip, newim.crval1*DEG2RAD, 
	       newim.crval2*DEG2RAD, &alpha, &delta);

      if( equinox != newim.equinox ){
	if( equinox == 1950.0 && newim.equinox == 2000.0 ){
	  toequinox1950( &alpha, &delta, newim.equinox );
	}
	else{
	  if( equinox == 2000.0 && newim.equinox == 1950.0 ){
	    toequinox2000( &alpha, &delta, newim.equinox );
	  }
	  else
	    return error("does not convert equinoxes other than 1950 or 2000");

	}//else
      }//if

      //
      // convert to the old system
      //
      ch_coord(alpha, delta, ap, phip, deltp, &phi, &theta);

      if( theta != 0.0 ){
	switch( projectiontype ){ 
	case RADECTAN: 
	  rt = RAD2DEG/tan( theta );
	  break;
	case RADECSIN:
	  rt = RAD2DEG * cos( theta );
	  break;
	default:
	  return error(" projectionsystem not set");
	}//switch()
      }
      else{
	warning("division by zero error!");
      }

      x = rt * sin( phi ) ;
      y =  -rt * cos( phi );

      // Note the 1.0: from fortran to c convention
      //
      icorr = x/cdelt1;
      jcorr = y/cdelt2;
      oldi = (int)(ipc001001*icorr + ipc001002*jcorr + 
		   crpix1 - 1.0 + dx0 + 0.5);
      oldj = (int)(ipc002001*icorr + ipc002002*jcorr + 
		   crpix2 - 1.0 + dy0 + 0.5);
      // Note that we do not make yet an interpolation to surrounding pixels:
      // in future this will be done
      //
      if( interpolate == FALSE ){
	if( oldi >= 0 && oldi < nx && oldj >= 0 && oldj < ny ){
	  if( isnan( pixel[oldi + oldj * nx])  ){
	    newim.pixel[ i + j * newim.nx ] = 0.0;
	  }
	  else{
	    newim.pixel[ i + j * newim.nx ] = pixel[oldi + oldj * nx];
	  }
	}// if
      }
      else{
	dx = ipc001001*icorr + ipc001002*jcorr + crpix1 - 1.0 - oldi;
	dy = ipc002001*icorr + ipc002002*jcorr + crpix2 - 1.0 - oldj;
	if( dx >= 0.0 ){
	  i2 = oldi + 1;
	}
	else{
	  i2 = oldi -1;
	}
	if( dy >= 0.0 ){
	  j2 = oldj + 1;
	}
	else{
	  j2 = oldj - 1;
	}
	dx = fabs(dx);
	dy = fabs(dy);
	//
	// The best matching pixel:
	if( oldi >= 0 && oldi < nx && oldj >= 0 && oldj < ny){
	  newim.pixel[i + j*newim.nx] = (1.0-dx)*(1.0-dy)*pixel[oldi+oldj*nx];
	}// if
	
	// Interpolating horizontally
	if( i2 >= 0 && i2 < nx && oldj >= 0 && oldj < ny){
	  newim.pixel[i + j*newim.nx] += dx * (1.0-dy) * pixel[i2 + oldj*nx];
	}// if
	
	// Interpolating vertically
	if( oldi >= 0 && oldi < nx && j2 >= 0 && j2 < ny){
	  newim.pixel[i + j*newim.nx] += (1.0-dx)*dy * pixel[oldi + j2*nx];
	}// if
	
	// Final interpolation
	if( i2 >= 0 && i2 < nx && j2 >= 0 && j2 < ny){
	  newim.pixel[i + j*newim.nx] += dx * dy * pixel[i2 + j2*nx];
	}// if
      }//if else

    }//for i
  }// for j

  *this = newim; // 
  reset_detector_keywords();

  return success();

}// newcoord2()



/*****************************************************************************/
//
// Function    : sin2tan()
// DESCRIPTION : Converts a SIN projection image to a TAN (GNOMIC) system.
//
// literature: Greisen & Calabretta, 1996, A&A
//
//
// SIDE EFFECTS: UNDER CONSTRUCTION
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      im                I                      image
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::sin2tan( const Image &im )
{
  if( im.nx * im.ny <= 0 || 
      ( strcmp(im.ctype1,"RA---SIN")!=0 && strcmp(im.ctype2,"DEC--SIN")!=0 && 
	strcmp(im.ctype1,"RA---NCP")!=0 && strcmp(im.ctype1,"DEC--NCP")!=0)){
    warning("sin2tan(): Invalid input image!");
    return FAILURE;
  }//if

  if( strcmp(im.ctype1,"RA---NCP")!=0 && strcmp(im.ctype1,"DEC--NCP")!=0){
    warning("sin2tan(): NCP projection deprecated!");
      return FAILURE;
  }

  int i, j, oldi,oldj, i0, in, j0, jn;
  double cosr, sinr, x2, y2;
  double x, y, phi, theta, rt;
  double todeg = 180.0/M_PI;

  copy_keywords( im ); // does what it says

  strncpy(ctype1, "RA---TAN", FLEN_VALUE);
  strncpy(ctype2, "DEC--TAN", FLEN_VALUE);
  projectiontype= RADECTAN;

  cosr = cos( crota2/todeg );        // rotations
  sinr = sin( crota2/todeg );

  //
  // First calculate the size of the new image
  // by calculating the borders.
  i0 = 0;
  j0 = 0;
  x2 = ((float) i0 + 1.0 - im.crpix1) * im.cdelt1;
  y2 = ((float) j0 + 1.0 - im.crpix2) * im.cdelt2;
  x =  x2 * cosr + y2 * sinr; // inverse rotation
  y = -x2 * sinr + y2 * cosr;

  rt    = sqrt (x*x + y*y );               // Rtheta in GC
  phi   = atan2( x, -y);                   // argument of x and y
  theta = acos( rt/todeg );  

  if( theta != 0.0 ){
    rt = todeg/tan(theta);
  }
  else{
    warning("sin2tan(): Division by zero encountered, results undefined!");
  }
  x = rt * sin( phi ) ;
  y =  -rt * cos( phi );
  x2 = x * cosr - y * sinr; // rotation  
  y2 = x * sinr + y * cosr;  
  // Note the 1.0: from fortran to c convention
  //

  i0 = (int)(x2/cdelt1  + crpix1 - 1.0 + 0.5);
  j0 = (int)(y2/cdelt2  + crpix2 - 1.0 + 0.5);

  in = im.nx -1;
  jn = im.ny -1;
  x2 = ((float) in + 1.0 - im.crpix1) * im.cdelt1;
  y2 = ((float) jn + 1.0 - im.crpix2) * im.cdelt2;
  x =  x2 * cosr + y2 * sinr; // inverse rotation
  y = -x2 * sinr + y2 * cosr;
  rt     = sqrt (x*x + y*y );               // Rtheta in GC
  phi   = atan2( x, -y);                   // argument of x and y
  theta = acos( rt/todeg );  

  if( theta!=0.0 ){
    rt = todeg/tan(theta);
  }
  else{
    warning("sin2tan(): Division by zero encountered, results undefined!");
  }

  x = rt * sin( phi ) ;
  y =  -rt * cos( phi );
  x2 = x * cosr - y * sinr; // rotation  
  y2 = x * sinr + y * cosr;  
  in = (int)(x2/cdelt1  + crpix1 - 1.0 + 0.5);
  jn = (int)(y2/cdelt2  + crpix2 - 1.0 + 0.5);

  if( allocate(in-i0+1,jn-j0+1)==FAILURE ){
    warning("sin2tan(): allocation failure.");
    return FAILURE;
  }//if
  //  printf("%d %d %d %d\n", i0, j0, in, jn);
  //
  // start the real work
  for( j = 0 ; j < ny; j++){
    for( i = 0 ; i < nx ; i++ ){

      // First calculate in the new image (TAN-projection) the phi,theta
      //
      x2 = ((float) i + 1.0 - im.crpix1) * im.cdelt1;
      y2 = ((float) j + 1.0 - im.crpix2) * im.cdelt2;
      x =  x2 * cosr + y2 * sinr; // inverse rotation
      y = -x2 * sinr + y2 * cosr;

      rt    = sqrt (x*x + y*y );               // Rtheta in GC
      phi   = atan2( x, -y);                   // argument of x and y
      if( fabs(rt) > 1.0E-20 ){
	theta = atan( todeg/rt );
      }
      else{
	theta = _SIGN( theta ) * 0.5 * M_PI;
      }  //else
      
      // Now calculate the corresponding x,y in the old image (SIN-proj.)
      //
      rt = todeg*cos(theta);
      x  = rt * sin( phi ) ;
      y  =  -rt * cos( phi );
      x2 = x * cosr - y * sinr; // rotation  
      y2 = x * sinr + y * cosr;  
      oldi = (int)(x2/im.cdelt1  + im.crpix1 - 1.0 + 0.5);
      oldj = (int)(y2/im.cdelt2  + im.crpix2 - 1.0 + 0.5);

      // replace this in future with an interpolation between the points
      if( oldi >= 0 && oldj >= 0 && oldi < im.nx && oldj < im.ny ){
	pixel[i + j*nx] = im.pixel[ oldi + oldj*im.nx];
      }//if

    }// for i
  }// for j

  reset_detector_keywords();

  return SUCCESS;
}// end sin2tan()


/*****************************************************************************/
//
// FUNCTION    : getXorY()
// DESCRIPTION : Given RA or DEC and x or y it calculates y or x.
//               The equations (C2) & (C3) are used of 
//               Greisen & Calabretta, 1996, A&A 
//
//
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
// xory                  I                      x, when whichxy=0, else y
// raordec               I                      ra, when raordec=0, else y
// whichxy               I
// whichraordec          I
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::getXorY(double xory, double raordec, 
			   int whichxy, int whichradec, double *outputxy)
{
  if( cdelt1 == 0.0 || cdelt2 == 0.0 || 
      (whichxy!= 0 && whichxy!= 1) || (whichradec != 0 && whichradec != 1 ) || 
      ( strncmp(ctype1,"RA---TAN",7) != 0 && 
	strncmp(ctype1,"RA---SIN",7) != 0 )  ){
    warning("getXorY(): Incorrect input parameters!");
    return FAILURE;
  }//if

  int i, maxit=100;
  double x, xi, y, yi;
  double a, b , c, d, f, g, fc, gc, denom=1.0, derdenom = 0.0;
  double delta, deltai, alpha, alphai, alphap;
  double sindp, cosdp, cosad, sinad, mu =0.0;
  double todeg = 180.0/M_PI;

  //
  // a,b,c,d refers to A,B,C,D in eq. (C3) of Greisen & Calabretta
  //
  a = ( cos(longpole/todeg) * pc001001 * cdelt1 + 
	sin(longpole/todeg) * pc002001 * cdelt2 )/todeg;
  b = ( cos(longpole/todeg) * pc001002 * cdelt1 + 
	sin(longpole/todeg) * pc002002 * cdelt2 )/todeg;
  c = ( sin(longpole/todeg) * pc001001 * cdelt1 - 
	cos(longpole/todeg) * pc002001 * cdelt2 )/todeg;
  d = ( sin(longpole/todeg) * pc001002 * cdelt1 - 
	cos(longpole/todeg) * pc002002 * cdelt2 )/todeg;

  alphap = crval1/todeg;
  sindp  = sin( crval2/todeg );
  cosdp  = cos( crval2/todeg );
  if( projectiontype == RADECSTG ){
    mu = 1; //default: 0
  }//
  if( whichxy == 0 ){// we want x
    xi = x = *outputxy - crpix1;
    yi = y = xory - crpix2;
  }
  else{
    xi = x = xory - crpix1;
    yi = y = *outputxy - crpix2;
  }

  if( whichradec == 0 ){ // the declination is given
    alphai = alpha = crval1/todeg;  //1st approximation
    delta = raordec/todeg;
    i = 0;
    do{
      x = xi;
      y = yi;
      alpha = alphai;
      if( alpha < -2.0 * M_PI || alpha < 2.0 * M_PI ){
	alpha = crval1/todeg;
      }//if
      cosad  = cos(alpha - alphap);
      sinad  = sin(alpha - alphap);
      if( projectiontype != RADECSIN ){ // default: denom=1.0
	denom    = mu + sin(delta) * sindp + cos(delta)*cosdp * cosad;
	derdenom = -cos(delta)* cosdp * sinad;
	//derivative of denominator with resp. to alpha
      }//if
      f = a*x + b*y + (mu+1.0) * cos(delta) * sinad/denom;
      g = c*x + d*y + (mu+1.0)*(cos(delta)*sindp*cosad-sin(delta)*cosdp)/denom;

      // derivative of f to alpha:
      fc = (mu+1.0)*( cos(delta) * cosad * denom + 
		    cos(delta)*sinad*derdenom)/SQR(denom);
      // derivative of g to alpha
      gc= -(mu+1.0)*( cos(delta)*sindp * sinad * denom +  
	    ( cos(delta)*sindp*cosad - sin(delta)*cosdp)*derdenom)/SQR(denom);

      if( whichxy == 0 ){// we want to know x
	if( a*gc - c*fc == 0.0 ){
	  warning("getXorY(): No solution exists.");
	  return FAILURE;
	}
	xi     = x - (f*gc - g*fc)/(a*gc - c*fc);
	alphai = alpha  - (g*a - f*c)/(a*gc - c*fc);
      }
      else{// we want to know y
	if( b*gc - d*fc == 0.0 ){
	  warning("getXorY(): No solution exists.");
	  return FAILURE;
	}
	yi     = y - (f*gc - g*fc)/(b*gc - d*fc);
	alphai = alpha  - (g*a - f*c)/(b*gc - d*fc);
      }//else
      i++;
    }while( (fabs( x - xi ) > 0.1 /*fabs(0.1 * cdelt1)*/ ||  
	     fabs( y - yi ) > 0.1 /*fabs(0.1 * cdelt2)*/ ) && i < maxit );

    if( whichxy == 0 ){// we want to know x
      *outputxy = xi + crpix1;
    }
    else{// we want to know y
      *outputxy = yi + crpix2;
    }//else

  }
  else{ //the ra is given
    alpha = raordec/todeg;
    deltai = delta = crval2/todeg; //1st approximation
    cosad  = cos(alpha - alphap);
    sinad  = sin(alpha - alphap);
    i = 0;
    do{
      x = xi;
      y = yi;
      delta = deltai;
      if( delta < -2.0 * M_PI || delta > 2.0 * M_PI ){
	delta =  crval2/todeg;
      }//if
      if( projectiontype != RADECSIN ){ // default: denom=1.0
	denom    = mu + sin(delta) * sindp + cos(delta)*cosdp * cosad;
	derdenom = cos(delta) * sindp - sin(delta)*cosdp * cosad;
	//derivative of denominator with resp. to delta
      }//if
      f = a*x + b*y + (mu+1.0) * cos(delta) * sinad/denom;
      g = c*x + d*y + (mu+1.0)*(cos(delta)*sindp*cosad-sin(delta)*cosdp)/denom;

      // derivative of f to delta:
      fc= -(mu+1.0)*( sin(delta)*sinad*denom + 
		    cos(delta)*sinad*derdenom)/SQR(denom);
      // derivative of g to delta
      gc= -(mu+1.0)*( (sin(delta)*sindp*cosad + cos(delta)*cosdp ) * denom +   
	    ( cos(delta)*sindp*cosad - sin(delta)*cosdp)*derdenom)/SQR(denom);

      if( whichxy == 0 ){// we want to know x
	if( a*gc - c*fc == 0.0 ){
	  warning("getXorY(): No solution exists.");
	  return FAILURE;
	}
	xi     = x - (f*gc - g*fc)/(a*gc - c*fc);
	deltai = delta  - (g*a - f*c)/(a*gc - c*fc);
      }
      else{// we want to know y
	if( b*gc - d*fc == 0.0 ){
	  warning("getXorY(): No solution exists.");
	  return FAILURE;
	}
	yi     = y - (f*gc - g*fc)/(b*gc - d*fc);
	deltai = delta  - (g*a - f*c)/(b*gc - d*fc);
      }//else
      i++;
    }while( (fabs( x - xi ) > 0.1 /* fabs(0.1 * cdelt1)*/ ||  
	     fabs( y - yi ) > 0.1 /* fabs(0.1 * cdelt2)*/) && i < maxit );

    if( whichxy == 0 ){// we want to know x
      *outputxy = xi + crpix1;
    }
    else{// we want to know y
      *outputxy = yi + crpix2;
    }//else


  }//else

  if( i == maxit ){
    warning("getXorY(): No convergence!");
    return FAILURE;
  }//if

  return SUCCESS;
}//getXorY2()

/*****************************************************************************/
//
// FUNCTION    : concat_h()
// DESCRIPTION : Concats two images along side each other
//
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::concat_h( const Image im1, const Image im2)
{

  if( im1.nx <= 0 || im1.ny <= 0 || im2.nx <= 0 || im2.ny <= 0){
    warning("concat_h(): Input images are not well defined!");
    return FAILURE;
  }

  int i, j, newnx, newny;

  newnx = im1.nx + im2.nx;
  newny = MAX( im1.ny, im2.ny);
  reset_keywords();
  // note that this sets nx and ny to newnx, newny
  if( allocate( newnx, newny ) == FAILURE ){
    warning("concat_h(): allocation error!");
    return FAILURE;
  }//if 

  // copy image 1 into the new image
  for( j= 0; j < im1.ny ; j++ ){
    for( i= 0; i < im1.nx ; i++ ){
      pixel[ i + j * nx ] = im1.pixel[i + j* im1.nx]; 
      // note the subtle im1.nx vs nx
    }//for i
  }//for j

  // copy image 2 into the new image
  for( j= 0; j < im2.ny ; j++ ){
    for( i= 0; i < im2.nx ; i++ ){
      pixel[ i+im1.nx + j * nx ] = im2.pixel[i + j* im2.nx]; 
      // note the subtle im2.nx vs nx
    }//for i
  }//for j

  return SUCCESS;

}// concat_h()



/*****************************************************************************/
//
// FUNCTION    : concat_v()
// DESCRIPTION : Concats two images vertically
//
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::concat_v( const Image im1, const Image im2)
{

  if( im1.nx <= 0 || im1.ny <= 0 || im2.nx <= 0 || im2.ny <= 0){
    warning("concat_v(): Input images are not well defined!");
    return FAILURE;
  }

  int i, j, newnx, newny, xshift1=0, xshift2=0;

  newnx = MAX(im1.nx, im2.nx);
  newny = im1.ny + im2.ny;
  reset_keywords();

  if( im1.nx < newnx )
    xshift1 = (int)(0.5*(newnx - im1.nx) + 0.5);
  else if( im2.nx < newnx)
    xshift2 = (int)(0.5*(newnx - im2.nx) + 0.5);

  // note that this sets nx and ny to newnx, newny
  if( allocate( newnx, newny ) == FAILURE ){
    warning("concat_v(): allocation error!");
    return FAILURE;
  }//if 

  // copy image 2 into the new image
  // The order is changed because x,y=0,0 is the lower left pixel
  // whereas we want image one to be on top
  for( j= 0; j < im2.ny ; j++ ){
    for( i= 0; i < im2.nx ; i++ ){
      pixel[ i + xshift2 + j * nx ] = im2.pixel[i + j* im2.nx]; 
      // note the subtle im1.nx vs nx
    }//for i
  }//for j

  // copy image 1 into the new image
  for( j= 0; j < im1.ny ; j++ ){
    for( i= 0; i < im1.nx ; i++ ){
      pixel[ i + xshift1 + (j + im2.ny) * nx ] = im1.pixel[i + j * im1.nx]; 
      // note the subtle im1.nx vs nx
    }//for i
  }//for j

  return SUCCESS;

}// concat_v()
#if 0
/****************************************************************************/
//
// FUNCTION    : MonteCarlo
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
unsigned long getindex(float distribution[], unsigned long n, double value);
Status Image::MonteCarlo( const Image &im, unsigned long int nevents)
{
  void indexxf(unsigned long n, const float arr[], unsigned long indx[]);

  setFunctionID("MonteCarlo");


  if( im.nx <= 0 || im.ny <= 0)
    return JVMessages::error("invalid input image");
  
  if( BasicData::allocate(nevents) == FAILURE )
    return JVMessages::error("error allocating events buffer");

  copy_keywords( im );


  unsigned long npix = im.nx * im.ny;
  unsigned long *index = new unsigned long [npix];
  float min = im.minimum();
  float *distribution = new float [npix];

  if( index == 0 || distribution == 0)
    return JVMessages::error("internal allocation error");

  indexxf( npix, im.pixel-1, index-1);

  JVMessages::message("succesfully generated an index");
  JVMessages::message("minumum pixel value is ",min);

  unsigned long i=0;
  unsigned long  khalf=0, kzero=0, kquart=0, k3quart=0;
  double sum=0.0, tot=im.total() - npix*min;
  for( unsigned long k = 0; k < npix; k++){

    i = index[k] - 1; // -1 for the numerical recipes way of indexing
    sum += im.pixel[i] - min;
    distribution[k] = float(sum)/tot;

    //    if(( k > npix-1000 && k < npix-1000 + 100) || k < 5)
    //      printf("%4i %e %e\n", k, im.pixel[i], sum, distribution[k]);
    if( distribution[k] <= 0.75 )
      k3quart = k;
    if( distribution[k] <= 0.5 )
      khalf = k;
    if( distribution[k] <= 0.25 )
      kquart = k;
    if( distribution[k] <= 0.0 )
      kzero = k;
  }//if

#if 0
  printf("%e %e %e\n",distribution[kzero], distribution[khalf], 
	 distribution[npix-1] );
  JVMessages::message("index value for 0.0 = ", kzero);
  JVMessages::message("index value for 0.5 = ", khalf);
#endif

  *this = im;
  *this *= 0.0;

  double rnd;
  int ix, iy;
  unsigned long k=0, idx;
  for( unsigned long int j=0; j < nevents ; j++){
    rnd = (double)rand()/RAND_MAX;
#if 0
    if( rnd < 0.25 )
      for( k = kzero; k < npix && distribution[k] < rnd ; k++);
    if( rnd < 0.5 )
      for( k = kquart; k < npix && distribution[k] < rnd ; k++);
    if( rnd < 0.75 )
      for( k = khalf; k < npix && distribution[k] < rnd ; k++);
    else
      for( k = k3quart; k < npix && distribution[k] < rnd ; k++);
#endif
    k = getindex( distribution, npix, rnd);
    idx = index[k] - 1;
    iy = idx / nx;
    ix = idx % nx;
    pixel[ix + iy*nx] += 1.0;
#if 0
    if( j < 10 && j>=1){
      printf("rnd= %e distr=%e %e index=%i i=%i j=%i\n",
	     rnd, distribution[k], distribution[k-1], idx, ix,iy);
    }
#endif
    //    else if( j % 1000 == 0 )
    //      printf("%i... ",j);
    rnd = (double)rand()/RAND_MAX;
    x[j] = ix + (rnd - 0.5);
    rnd = (double)rand()/RAND_MAX;
    y[j] = iy + (rnd - 0.5);
  }//for

  delete [] index;
  delete [] distribution;

  return success();
}//Image


#endif

//
// binary search
//
unsigned long getindex(float distribution[], unsigned long n, double value)
{
  //  setFunctionID();
  unsigned long index=0, k1=0, k2=n-1, k;

  if( value < 0.0 || value > 1.0 || n < 2){
    error("invalid value");
    return 0;
  }

  do{
    k = (k1+k2)/2;
    if( value >  distribution[k])
      k1 = k;
    else
      k2 = k;
  }while( k2-k1 > 1 );
  if( fabs(distribution[k1]-value) <  fabs(distribution[k2]-value))
    index = k1;
  else
    index = k2;

  //  success();
  return index;  
}//getindex

#define NRANSI
//#include "nrutil.h"
#define SWAP(a,b) itemp=(a);(a)=(b);(b)=itemp;
#define M 7
#define NSTACK 50

#if 0
void indexxf(unsigned long n, const float arr[], unsigned long indx[])
{
	unsigned long i,indxt,ir=n,itemp,j,k,l=1;
	int jstack=0,*istack;
	float a;

	istack=ivector(1,NSTACK);
	for (j=1;j<=n;j++) indx[j]=j;
	for (;;) {
		if (ir-l < M) {
			for (j=l+1;j<=ir;j++) {
				indxt=indx[j];
				a=arr[indxt];
				for (i=j-1;i>=l;i--) {
					if (arr[indx[i]] <= a) break;
					indx[i+1]=indx[i];
				}
				indx[i+1]=indxt;
			}
			if (jstack == 0) break;
			ir=istack[jstack--];
			l=istack[jstack--];
		} else {
			k=(l+ir) >> 1;
			SWAP(indx[k],indx[l+1]);
			if (arr[indx[l]] > arr[indx[ir]]) {
				SWAP(indx[l],indx[ir])
			}
			if (arr[indx[l+1]] > arr[indx[ir]]) {
				SWAP(indx[l+1],indx[ir])
			}
			if (arr[indx[l]] > arr[indx[l+1]]) {
				SWAP(indx[l],indx[l+1])
			}
			i=l+1;
			j=ir;
			indxt=indx[l+1];
			a=arr[indxt];
			for (;;) {
				do i++; while (arr[indx[i]] < a);
				do j--; while (arr[indx[j]] > a);
				if (j < i) break;
				SWAP(indx[i],indx[j])
			}
			indx[l+1]=indx[j];
			indx[j]=indxt;
			jstack += 2;
			if (jstack > NSTACK) nrerror("NSTACK too small in indexx.");
			if (ir-i+1 >= j-l) {
				istack[jstack]=ir;
				istack[jstack-1]=i;
				ir=j-1;
			} else {
				istack[jstack]=j-1;
				istack[jstack-1]=l;
				l=i;
			}
		}
	}
	free_ivector(istack,1,NSTACK);
}
#undef M
#undef NSTACK
#undef SWAP
#undef NRANSI
/* (C) Copr. 1986-92 Numerical Recipes Software *;#1. */

#endif
