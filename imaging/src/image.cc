/*****************************************************************************/
//
// FILE        :  Image.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Implementation of the  Image class.
//                Basic functions.
//
// COPYRIGHT   : Jacco Vink, SRON, 1996, 2001, 2003
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_image.h"
#include "coordinates.h"

using jvtools::Image;
using namespace::JVMessages;

/*****************************************************************************/
//
// FUNCTION    : Image()
// DESCRIPTION : constructor
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/*****************************************************************************/
Image::Image( void  )
{
  setFunctionID("Image");

  if( strcmp(classid,"IMAGE")== 0){
   error("Constructor illegaly called!");
   return;
  }
  strcpy(classid,"IMAGE");

  reset_keywords();
  drdelt1 = 0.0;
  drdelt2 = 0.0;
  drpix1 = 0.0;
  drpix2 = 0.0;

  pixel = 0;  
  nx  = 0;
  ny  = 0;
  //  ncomplex = 0;
  pixel= 0;
  //  fftw_pixels = 0;
  //  pixels_fft = 0;
  emin = emax=0.0;

  //  ftransformed = FALSE;
  //  fftplanexist = FALSE;
  //  fftwdir =  FFTWF_REAL_TO_COMPLEX;

  imgbin = 1;

  success();
}// end Image()


/****************************************************************************/
//
// FUNCTION    : Image()
// DESCRIPTION : constructor
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   xsize, nysize          I                     x/y size
//
//  RETURNS    :
//
/****************************************************************************/
Image::Image( int xsize, int ysize, float val )
{
  setFunctionID("Image");

  if( strcmp(classid,"IMAGE")== 0)
    error("Constructor illegaly called!");

  strcpy(classid,"IMAGE");

  reset_keywords();
  drdelt1 = 0.0;
  drdelt2 = 0.0;
  drpix1 = 0.0;
  drpix2 = 0.0;
  
  nx  = 0;
  ny  = 0;
  //  ncomplex = 0;
  pixel = 0;
  //  fftw_pixels = 0;
  //  pixels_fft = 0;

  //  ftransformed = FALSE;
  //  fftplanexist = FALSE;

  imgbin = 1;

  allocate( xsize, ysize, val );

  success();
}// end Image()

/****************************************************************************/
//
// FUNCTION    : Image()
// DESCRIPTION : copy constructor
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/****************************************************************************/
Image::Image( const Image &im )
{ 
  setFunctionID("Image/copy");

  if( this == &im || strcmp(classid,"IMAGE")== 0){
    error("Constructor illegaly called!");
    Image();
  }

  nx  = 0;
  ny  = 0;
  //  ncomplex = 0;
  pixel = 0;
  //  fftw_pixels = 0;
  //  pixels_fft = 0;

  //  ftransformed = FALSE;
  //  fftplanexist = FALSE;
  //  fftwdir = FFTWF_REAL_TO_COMPLEX;

  strcpy(classid,"IMAGE");

  if( im.nx > 0 && im.ny > 0 ){
    if( allocate( im.nx, im.ny ) == FAILURE ){
      warning("Allocation error");
    }
    else{
      
      copy_keywords( im ); // copy the keywords from object im
      imgbin = im.imgbin;
      
      long nxy = nx*ny;
      float *p1 = &pixel[0], *p2 = &im.pixel[0];
      for( long i = 0; i < nxy ; i++ ){
	*p1++ = *p2++;
      }// end for
      
    }//else	
  }
  else
    warning("Illegal function call!");

  setprojection();
  //  chatty = true;

  success();
}// end Image()


/*****************************************************************************/
//
// FUNCTION    : ~Image()
// DESCRIPTION : destructor
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/*****************************************************************************/
Image::~Image()
{

  deallocate();

  strcpy(classid,"ENDED");

}// end ~Image()

/*****************************************************************************/
//
// FUNCTION    :  operator=()
// DESCRIPTION :  copy operator. Only the image parameters are copied, 
//		not the event list.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : reference to the object
//
/*****************************************************************************/
Image &Image::operator=(Image const &im )
{
  setFunctionID("operator=[Image]");

  if( this == &im ){
    error("cannot assign to itself");
    return *this;
  }//if

  if( (this!= &im) && ((im.nx != this->nx) || (im.ny != this->ny)) ){
    if( allocate( im.nx, im.ny ) == FAILURE ){
      error("allocation error");
      return *this;
    } // end if
  }// end if
  
  copyKeywords( im ); // copy the keywords from object im
  long nxy = nx*ny;
  float *p1 = pixel;
  float *p2 = im.pixel;


  blank = im.blank;

  for( long i = 0; i < nxy ; i++ )
    *p1++ = *p2++;

#ifdef USEFFTW
  if( fftplanexist == true){
    //    rfftwnd_destroy_plan( fftplan);
    fftwf_destroy_plan( fftplan );
    fftplanexist = FALSE;
    //    fftwdir = FFTWF_REAL_TO_COMPLEX;
  }
#endif
  
  //  chatty = true;
  success();
  return *this;

}// operator=()

/*****************************************************************************/
//
// FUNCTION    :  operator=()
// DESCRIPTION :  copy operator. Only the image parameters are copied, 
//		not the event list.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : reference to the object
//
/*****************************************************************************/
Image &Image::operator=(float value )
{
  setFunctionID("operator=[float]");

  if( nx <= 0 && ny <= 0 ){
    error("image not allocated");
    return *this;
  }//if

  long nxy = nx*ny;

  for( long i = 0; i < nxy ; i++ )
    pixel[i] = value;

  success();
  return *this;

}// operator=()

/****************************************************************************/
//
// FUNCTION    : allocate()
// DESCRIPTION : Allocates memory for the Image().
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//       n               I                 number of rows
//       m               I                           columns
//
//  RETURNS    :  SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::allocate( int n, int m, float initval)
{
  setFunctionID("allocate[image]");

  if( n <= 0 || m <= 0 || n *m > MAX_SIZE*MAX_SIZE ){
    message("nx = ", n);
    message("ny = ", m);
    return error("error in the input parameters.");
  }


  long nxy = n*m;

  if( nx != n || ny != m ){
    deallocate();
    pixel = new float [nxy]; // allocate
  }//if


  nx = n;
  ny = m;

  if( pixel == 0 ){ // memory allocated?
    nx = ny = 0;
    return error("allocation error");
  }// end if

  for(long i = 0; i < nxy; i++ )
    pixel[i] = initval;

  setprojection();
  //  message("allocation succeeded");
  return success();

}// end allocate()


#if USEFFTW
/*****************************************************************************/
//
// FUNCTION    : allocate_complex()
// DESCRIPTION : Allocates memory for the complex array and creates a
//               plan for the FFT (see manual of fftw).
//               The size of the array depends on nx, ny of the real image.
//               NOTE that fftw uses a different convention for the x and y
//               coordinates, y is the fastest changin coordinate.
//               So the naming convention for x and y are interchanged!
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//     dir               I           FFT direction for which the plan is made.
//
//  RETURNS    :  SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::allocate_complex( fftw_direction dir )
{

  setFunctionID("allocate_complex");

  if( nx <= 0 || ny <= 0 || nx*ny > MAX_SIZE * MAX_SIZE )
    return error("Error in the input parameters.");


  //
  // Note that nx is here the fastest running index
  // not ny (as in the FFTW manual)!
  //
  if( ncomplex != (unsigned long)(nx/2+1)*ny ){

    if( ncomplex != 0 || fftplanexist == true){
      fftplanexist = false;
      //      rfftwnd_destroy_plan( fftplan);
      fftwf_destroy_plan( fftplan );
      delete [] pixels_fft;
    }//if

    //
    // Note that nx is here the fastest running index
    // not ny (as in the FFTW manual)!
    //
    ncomplex = (nx/2+1)*ny;
    pixels_fft = new complex <float> [ncomplex];
    fftw_pixels = reinterpret_cast<fftwf_complex*>(pixels_fft);

    //    for( unsigned long i=0; i < ncomplex ; i++)
    //      fftw_pixels[i].re = fftw_pixels[i].im =0.0;
    if( dir == FFTW_REAL_TO_COMPLEX )
	fftplan=fftwf_plan_dft_r2c_2d(nx, ny, pixel,fftw_pixels,FFTW_ESTIMATE );
    else if( dir == FFTW_COMPLEX_TO_REAL )
	fftplan=fftwf_plan_dft_c2r_2d(nx, ny, fftw_pixels,pixel,FFTW_ESTIMATE );

    // create the plan
    //    fftplan = rfftw2d_create_plan(ny, nx, dir, FFTWF_ESTIMATE);
    fftwdir = dir;

    fftplanexist = true;

  }
  else
    if( fftplanexist == false || fftwdir != dir ){ 
      //this can happen ncomplex=okay, fftwplan not
      // create the plan
      if( fftplanexist == true && fftwdir != dir ){
	fftwf_destroy_plan( fftplan );
	//	  rfftwnd_destroy_plan( fftplan);
      }
      if( dir == FFTW_REAL_TO_COMPLEX )
	fftplan=fftwf_plan_dft_r2c_2d(nx, ny,pixel,fftw_pixels, FFTW_ESTIMATE );
      else if( dir == FFTW_COMPLEX_TO_REAL )
	fftplan=fftwf_plan_dft_c2r_2d(nx, ny,fftw_pixels,pixel, FFTW_ESTIMATE );

      //rfftw2d_create_plan(ny, nx, dir, FFTWF_ESTIMATE);
      fftwdir = dir;
      fftplanexist = true;
    }

  ftransformed = false;

  //  message("complex array allocated");

  return success();
}// end allocate_complex()


#endif

/*****************************************************************************/
//
// FUNCTION    : deallocate()
// DESCRIPTION : Deallocates the Image.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :  SUCCESS
//
/*****************************************************************************/
Status Image::deallocate()
{

  setFunctionID("deallocate[Image]");

  if( nx > 0 && ny > 0 ){
    delete [] pixel;
    pixel = 0;
  } // end if
  nx = ny = 0;

#ifdef USEFFTW
  if( ncomplex > 0 ){
    delete [] pixels_fft;
    pixels_fft = 0;
    fftw_pixels = 0;
    ftransformed = FALSE;
  }
  ncomplex = 0;


  if(   fftplanexist == true){
    //    rfftwnd_destroy_plan( fftplan);
    fftwf_destroy_plan( fftplan );
    fftplanexist = FALSE;
  }
#endif
  
  return success();
}// end deallocate()


/****************************************************************************/
//
// FUNCTION    : reset_detector_keywords()
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS
//
/****************************************************************************/
Status Image::reset_detector_keywords()
{

  drpix1    = 0.0;
  drpix2    = 0.0;
  drdelt1   = 0.0;
  drdelt2   = 0.0;
  imgbin    = 1;

  return SUCCESS;
}// reset_detector_keywords()


/*****************************************************************************/
//
// FUNCTION    : replaceBlank()
// DESCRIPTION : 
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
Status Image::replaceBlank(float val)
{
  setFunctionID("replaceBlank");

  if( nx <= 0 || ny <= 0)
    return error("nx and/or ny == 0 ");
  //  testPrint();

  message("replacing NaN with ",val);
  for( int j=0; j < ny; j++){
    for( int i=0; i < nx; i++){
      if( j<100 && i < 100){
	//	printf("x=%i y=%i pixval=%f ?=%i\n",i,j, pixel[i + j *ny], isnormal(pixel[i + j * ny]));
      }
      if( isnormal(pixel[i + j * ny]) != FP_NORMAL ){
	pixel[i + j *ny] = val;
	//	printf("->x=%i y=%i pixval=%f\n",i,j, pixel[i + j *ny], isnormal(pixel[i + j * ny]));
      }//if
    }//for
  }//for

  //return SUCCESS;
  return success();
}//replaceBlank()


/*****************************************************************************/
//
// FUNCTION    : bilevel( )
// DESCRIPTION : Converts an image into a bilevel image.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
// low                   I                  lower limit
// high                  I                  upper limit
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::bilevel( float low, float up, float val )
{
  setFunctionID("bilevel");
  if( nx <= 0 || ny <= 0)
    return error("image does not exist");


  long int nxy =nx*ny;
  float pixval;

  for( long int i=0 ; i < nxy ; i++){
    pixval = pixel[i];
    if( pixval != blank || blank == 0.0 ){
      if( pixval > low && pixval <= up )
	pixel[i] = val;
      else
	pixel[i] = 0.0;
    }//if
  }//for

  //  message("bilevel(): Transforming image to bilevel image, tag = ",val);

  return success();
}// bilevel()

/*****************************************************************************/
//
// FUNCTION    : threshold()
// DESCRIPTION : thresholds the image, i.e. when a pixel has  
//               a value > t it is set to a value val
//               otherwise it is set to 0.0
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS OR FAILURE
//
/*****************************************************************************/
Status Image::threshold( float t , float val )
{
  setFunctionID("threshold");

  if(nx*ny <= 0.0 )
    return error("Image does not exist");
  
  long nxy= nx*ny;
  for( long i = 0; i < nxy; i++){
    
    if( pixel[i] < t )
      pixel[i] = 0.0;
    else
      pixel[i] = val;
    
  }//for

  return success();
}// threshold()

/*****************************************************************************/
//
// FUNCTION    : flipX()
// DESCRIPTION : 
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
Status Image::flipX( void )
{
  setFunctionID("flipX");
  if( nx < 1 || ny < 1)
    return error("No image available!");


  int i, j;
  Image temp(*this );
  
  temp.cdelt1 *= -1;
  temp.crpix1 = temp.nx -1 - temp.crpix1;

  for( i = 0; i < nx ; i++){
    for( j=0 ; j < ny ; j++){
      temp.pixel[ nx - 1 -i + j*nx] = pixel[i + j*nx];
    }//for j
  }//for i

  *this = temp;

  return success();
}// flipX()

/*****************************************************************************/
//
// FUNCTION    : flipY()
// DESCRIPTION : 
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
Status Image::flipY( void )
{
  setFunctionID("flipY");
  if( nx < 1 || ny < 1)
    return error("No image available!");


  int i, j;
  Image temp(*this );
  
  temp.cdelt2 *= -1;
  temp.crpix2 = temp.ny -1 - temp.crpix2;

  for( i = 0; i < nx ; i++){
    for( j=0 ; j < ny ; j++){
      temp.pixel[ i + (ny-1-j)*nx] = pixel[i + j*nx];
    }//for j
  }//for i

  *this = temp;

  return success();
}// flipY()


/*****************************************************************************/
//
// FUNCTION    : flipXY()
// DESCRIPTION : 
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
Status Image::flipXY( void )
{
  setFunctionID("flipXY");
  if( nx < 1 || ny < 1)
    return error("No image available!");


  int i, j;
  Image temp(*this );
  
  temp.nx = ny; // Note that the total array length is equal, no new allocation
  temp.ny = nx;
  temp.cdelt1 = cdelt2;
  temp.cdelt2 = -cdelt1;
  temp.crval1 = crval2;
  temp.crval2 = crval1;
  strcpy(temp.ctype1, ctype2);
  strcpy(temp.ctype2, ctype1);
  temp.crpix1 = crpix2;
  temp.crpix2 = nx-crpix1;

  for( i = 0; i < nx ; i++){
    for( j = 0 ; j < ny ; j++){
      temp.pixel[ j + (nx-1-i)*ny] = pixel[i + j*nx];
    }//for j
  }//for i

  *this = temp;

  return success();
}// flipXY()



/*****************************************************************************/
//
// FUNCTION    : invert()
// DESCRIPTION : Invert image, so: max-> min min->max
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
Status Image::invert( void )
{

  setFunctionID("invert");
  long i, nxy = (long)(nx * ny);
  float min = minimum();
  float max = maximum();

  if( nxy <= 0 || min == max )
    return error("No image available or min==max");


  for( i = 0 ; i < nxy ; i++){
    pixel[i] = min + max - pixel[i];    
  }//for i

  return success();
}//invert()


/*****************************************************************************/
//
// FUNCTION    : clip()
// DESCRIPTION : Sets all pixel values of pixels with values outside a certain
//               range to BLANK.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   minval, maxval     I                the boundaries of the valid interval
//   saturate           I                will replace value > maxval by maxval
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::clip(float minval, float maxval, bool saturate)
{

  setFunctionID("clip");

  if(minval >= maxval)
    return error("minval >= maxval!");

  if( saturate && minval!=0.0){//
    *this -= minval;
    maxval -= minval;
    minval=0.0;
    message("Offset level subtracted. New maxval = ",maxval);
  }

  int nxy  = nx*ny;
  for(long i=0; i < nxy; i++ ){
    if(( pixel[i] < minval || ( pixel[i] > maxval) ) && (pixel[i] != blank) ){
      if( saturate && pixel[i] > maxval )
	pixel[i] = maxval;
      else
	pixel[i] = blank;
    }//end if
  }// end for
  
  return success();
}// end clip()


/*****************************************************************************/
//
// FUNCTION    : operator+=()
// DESCRIPTION : Adds an other image to the image.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      im                I                      image
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::operator+=( const Image &im )
{
  setFunctionID("operator+= (image)");

  if( nx != im.nx || ny != im.ny || nx < 1  || ny < 1 )
    return error("the image sizes do not match");


  if( cdelt1 != im.cdelt1 ||  cdelt2 != im.cdelt2 || crval1 != im.crval1 ||
      crval2 != im.crval2 || crpix1 != im.crpix1 || crpix2 != im.crpix2 ||
      equinox !=im.equinox )
    warning("physical coordinates are not the equal");
  
  float *ptr1 = pixel;
  float *ptr2 = im.pixel;
  double tot1=total(), tot2=im.total();
  
  long nxy = nx*ny;
  for( long i = 0; i < nxy ; i++ ){
    
    if( ( *ptr1 == blank && blank != 0.0) || 
	( *ptr2 == im.blank && im.blank!= 0.0)){
      *ptr1 = blank;
    }
    else{
      *ptr1 += *ptr2;
    }//if
    ptr1++;
    ptr2++;
    
  }// end for

  mjd_obs= MIN( mjd_obs, im.mjd_obs);
  if( tot1 >= 0.0 && tot2 >= 0.0)
    mjdmean =  (tot1 * getMjdMean() + tot2 * im.getMjdMean())/(tot1 + tot2);
  else{
    mjdmean = 0.0;
    warning("MJDMEAN not set");
  }
  
  if( exposure > 0.0 && im.exposure > 0.0)
    exposure += im.exposure;

  if( strcmp( obsdate, im.obsdate ) > 0 )
    strcpy(obsdate,im.obsdate);
  if( strcmp( enddate, im.enddate ) < 0 )
    strcpy(enddate,im.enddate);
  
  reset_detector_keywords();
  return success();

}// end operator+=() 


/*****************************************************************************/
//
// FUNCTION    : operator-=()
// DESCRIPTION : Subtracts an other image from the image.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      im                I                      image
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::operator-=( const Image &im )
{
  setFunctionID("operator-=(im)");

  if( nx != im.nx || ny != im.ny || nx < 1  || ny < 1 )
    return error("the image sizes do not match");


  if( cdelt1 != im.cdelt1 ||  cdelt2 != im.cdelt2 || crval1 != im.crval1 ||
      crval2 != im.crval2 || crpix1 != im.crpix1 || crpix2 != im.crpix2  ||
      equinox !=im.equinox )
    warning("physical coordinates are not the equal");

  float *ptr1 = pixel;
  float *ptr2 = im.pixel;
  long nxy = nx*ny;
  for( long i = 0; i < nxy ; i++ ){
    
      if( ( *ptr1 == blank && blank != 0.0) || 
	  ( *ptr2 == im.blank && im.blank != 0.0) || 
	  isnan( *ptr1) || isnan( *ptr2) ){
	*ptr1 = blank;
      }
      else{
	*ptr1 -= *ptr2;
      }//if
      ptr1++;
      ptr2++;

  }// end for
  
  if( exposure > 0.0 && im.exposure > 0.0)
    exposure -= im.exposure;
  
  
  reset_detector_keywords();
  return success();

}// end operator-=() 


/*****************************************************************************/
//
// FUNCTION    : operator*=()
// DESCRIPTION : Multiplies an other image with the image.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      im                I                      image
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::operator*=( const Image &im )
{
  setFunctionID("operator*=[Image]");

  if( nx != im.nx || ny != im.ny || nx < 1  || ny < 1 )
    return error("the image sizes do not match");

  const float tol=1.0e-6; 
  if( fabs(cdelt1 - im.cdelt1) > tol || fabs(cdelt2 - im.cdelt2) > tol )
    JVMessages::warning("physical coordinates are unequal (cdelt)");
  if( fabs(crval1 - im.crval1) > tol || fabs(crval2 - im.crval2) > tol )
    JVMessages::warning("physical coordinates are unequal (crval)");
  if( crpix1 != im.crpix1 || crpix2 != im.crpix2)
    JVMessages::warning("physical coordinates are unequal (crpix)");
  if( equinox != im.equinox )
    JVMessages::warning("physical coordinates are unequal (equinox)");

  
  float *ptr1 = pixel;
  float *ptr2 = im.pixel;
  long nxy = nx*ny;
  for( long i = 0; i < nxy ; i++ ){
      
    if( ( *ptr1 == blank && blank != 0.0) || 
	( *ptr2 == im.blank && im.blank!= 0.0)){
      *ptr1 = blank;
    }
    else{
      *ptr1 *= *ptr2;
    }//if
    ptr1++;
    ptr2++;
    
  }// end for

  reset_detector_keywords();

  return success();

}// end operator*=() 

/*****************************************************************************/
//
// FUNCTION    : operator/=()
// DESCRIPTION : Devides the image by an another image. Checks for division by
//		 zero.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      im                I                      image
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::operator/=( const Image &im )
{

  setFunctionID("operator/=[Image]");

  if( nx != im.nx || ny != im.ny || nx < 1  || ny < 1 ){
    message("nx1=",nx);
    message("ny1=",ny);
    message("nx2=",im.nx);
    message("ny2=",im.ny);
    return error("the image sizes do not match");
  }

  const float tol=1.0e-6; 
  if( fabs(cdelt1 - im.cdelt1) > tol || fabs(cdelt2 - im.cdelt2) > tol )
    JVMessages::warning("physical coordinates are unequal (cdelt)");
  if( fabs(crval1 - im.crval1) > tol || fabs(crval2 - im.crval2) > tol )
    JVMessages::warning("physical coordinates are unequal (crval)");
  if( crpix1 != im.crpix1 || crpix2 != im.crpix2)
    JVMessages::warning("physical coordinates are unequal (crpix)");
  if( equinox != im.equinox )
    JVMessages::warning("physical coordinates are unequal (equinox)");

  bool zerodivision = false;
  float *ptr1 = pixel;
  float *ptr2 = im.pixel;
  float newblank = nanf("is");  
  newblank = 0.0;

  long nxy = nx*ny;
  for( long i = 0; i < nxy ; i++ ){   

    if( ABS( *ptr2 ) > MIN_DIV && 
	( *ptr2  != im.blank || im.blank==0.0) && 
	( *ptr1 != blank || blank == 0.0 ) ){
      *ptr1 /=  *ptr2;
    }
    else{
      *ptr1 = newblank;
      zerodivision = true;
    }// end if
    ptr1++;
    ptr2++;

  }// end for

  blank = newblank;
  reset_detector_keywords();

  if( zerodivision == FALSE ){
    return success();
  }
  else{
    //    warning("operator/=(im): Divison by zero encountered!");
    //    return FAILURE;
  }// end if

  //  message("blank = ", blank);
  return success();

}// end operator/=() 

/*****************************************************************************/
//
// FUNCTION    : operator+=()
// DESCRIPTION : Adds a constant to all pixels.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//       a              I                         float value
//
//  RETURNS    :  SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::operator+=( double a )
{
  setFunctionID("operator+=");
  if( nx < 1  || ny < 1 )
    return error("the image has no pixels");

  
  long nxy = nx*ny;
  for( int i = 0; i < nxy ; i++ ){
        if( pixel[i] != blank || blank == 0.0 ){
	  pixel[i]+= a;
        }
  }// end for

  return success();

}// end operator+=() 


/*****************************************************************************/
//
// FUNCTION    : operator-=()
// DESCRIPTION : Subtracts a constant from all pixels.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//       a              I                         float value
//
//  RETURNS    :  SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::operator-=( double a )
{
  setFunctionID("operator-=");
  if( nx < 1  || ny < 1 )
    return error("void image");


  long nxy = nx*ny;
  for( long i = 0; i < nxy ; i++ ){
    if( pixel[i] != blank || blank == 0.0 ){
      pixel[i] -= a;
    }// end if
  }// end for
  
  return success();

}// end operator-=() 

/*****************************************************************************/
//
// FUNCTION    : operator*=(double )
// DESCRIPTION : Multiplies all pixels by a constant.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//       a              I                         float value
//
//  RETURNS    :  SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::operator*=( double a )
{
  setFunctionID("operator*=");
  if( nx < 1  || ny < 1 )
    return error("void image");

  
  long nxy = nx*ny;
  for( long i = 0; i < nxy ; i++ ){
    if( pixel[i] != blank || blank==0.0){
      pixel[i]*= a;
    }
  }// end for


  exposure *= a;  


  return success();

}// end operator*=() 

/*****************************************************************************/
//
// FUNCTION    : operator/=()
// DESCRIPTION : Divides by a value.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//       a              I                         float value
//
//  RETURNS    :  SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::operator/=( double a )
{
  setFunctionID("operator/=");
  if( nx < 1  || ny < 1  || a == 0.0 )
    return error("this image has no pixels or division by zero");


  long nxy = nx*ny;
  for( long i = 0; i < nxy ; i++ ){
    if( pixel[i] != blank || blank==0.0 ){
      pixel[i] /= a;
    }
  }// end for

  exposure /= a;

  return success();

}// end operator/=() 


/*****************************************************************************/
//
// FUNCTION    : squareroot()
// DESCRIPTION : Replaces all entries by thee square root.
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
Status Image::squareroot( void )
{
  setFunctionID("squareroot");

  if( nx < 1  || ny < 1 )
    return error("this image has no pixels or division by zero");


  bool nosqrt = false;
  long nxy = nx*ny;

  //  blank = nanf("none");
  //  blank = MAXFLOAT;
  blank = 0.0;
  //  message("blank = ", blank);

  
  for( long i = 0; i < nxy ; i++ ){  
    if( pixel[i] >= 0.0 && !isnan(pixel[i]) && (pixel[i] != blank || blank==0.0)){
      pixel[i]= sqrt( pixel[i]);
    }
    else{
      if( pixel[i] < 0.0 )
	nosqrt = true;
      pixel[i] = blank;
    }// end if
  }// end for
  
  if( !nosqrt )
    return success();
  else
    return error("square root of negative value");
  
  return success();

}// end squareroot() 

/*****************************************************************************/
//
// FUNCTION    : logarithm()
// DESCRIPTION : Replaces all entries by the logarithm()
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
Status Image::logarithm( void )
{
  setFunctionID("logarithm");
  setFunctionID("logarithm");

  if( nx < 1  || ny < 1 )
    return JVMessages::error("no image available");


  bool nolog = FALSE;
  long nxy = nx*ny;
  for( long i = 0; i < nxy ; i++ ){  
    if( pixel[i] > 0.0 && (pixel[i] != blank || blank==0.0)){
      pixel[i]= log( pixel[i]);
    }
    else{
      pixel[i] = blank;
      nolog = true;
    }// end if
  }// end for
  
  if( nolog == FALSE )
    return success();
  else
    return JVMessages::error("negative value encountered");
  
  return success();
  
}// end logarithm() 


/*****************************************************************************/
//
// FUNCTION    : countrate()
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/*****************************************************************************/
Status Image::countrate()
{
  setFunctionID("countrate");
  if( exposure <= 0.0 )
    return error("no exposure specified");


  long nxy=nx*ny;
  for( long i  = 0; i < nxy; i++){
    if( pixel[i]!= blank ){
      pixel[i] /= exposure;
    }//if
  }//for

  return success();
}//countrate




/*****************************************************************************/
//
// FUNCTION    : minimum()
// DESCRIPTION : Return the minimum pixel value.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//    
//
//  RETURNS    :  minimum of pixel values
//
/*****************************************************************************/
float Image::minimum( int i1, int j1, int i2, int j2 ) const
{

  setFunctionID("minimum");
  if( i1 == -1 && j1 == -1 && i2 == -1 && j2 == -1 ){
    i1 = j1 = 1;
    i2 = nx;
    j2 = ny; 
  }
  if( i1 >= i2 || j1 >= j2 || i1 < 1 || j1 < 1 || i2 > nx || j2 > ny )
    return error("error in input parameters.");

  if( nx <= 1 || ny <= 1 )
    return error("minimum undefined.");


  i1 -= 1;
  i2 -= 1;
  j1 -= 1;
  j2 -= 1;

  long nxy = nx*ny;
  float pixval, min = pixel[nxy/2];

  int i;
  for( int j =j1; j <= j2; j++)
    for( i = i1; i <= i2; i++){
      pixval = pixel[i + j*nx];
      if( !isnan(pixval) )
	//      if( pixval != blank || blank == 0.0)
	min =  MIN( min , pixval);
    }// end for( int i 


  if( nx < 1 || ny < 1 ){
    error("pixel minimum is not defined");
    return 0.0;
  }// end if

  success();

  return min;
}// end minimum()
float Image::minimum( int *imin, int *jmin, int i1, int j1, int i2, int j2 ) const
{

  setFunctionID("minimum");
  if( i1 == -1 && j1 == -1 && i2 == -1 && j2 == -1 ){
    i1 = j1 = 1;
    i2 = nx;
    j2 = ny; 
  }
  if( i1 >= i2 || j1 >= j2 || i1 < 1 || j1 < 1 || i2 > nx || j2 > ny )
    return error("error in input parameters.");

  if( nx <= 1 || ny <= 1 )
    return error("minimum undefined.");


  i1 -= 1;
  i2 -= 1;
  j1 -= 1;
  j2 -= 1;

  long nxy = nx*ny;
  float pixval, min = MAX_FLOAT;//pixel[0];


  *imin = -1;
  *jmin = -1;
  for( int j =j1; j <= j2; j++)
    for( int i = i1; i <= i2; i++){
      pixval = pixel[i + j*nx];
      if( !isnan(pixval) )
	//      if( pixval != blank || blank == 0.0)
	//	min =  MIN( min , pixval);
	if( pixval < min ){
	  min = pixval;
	  *imin = i+1;
	  *jmin = j+1;
	}
    }// end for( int i 


  if( nx < 1 || ny < 1 ){
    error("pixel minimum is not defined");
    return 0.0;
  }// end if

  success();

  return min;
}// end minimum()






/*****************************************************************************/
//
// FUNCTION    : maximum()
// DESCRIPTION : Return the maximum pixel value.
// 
//
// SIDE EFFECTS: 
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//    
//
//  RETURNS    :  maximum of pixel values
//
//
/*****************************************************************************/
float Image::maximum( int i1, int j1, int i2, int j2  ) const
{

  setFunctionID("maximum");
  if( i1 == -1 && j1 == -1 && i2 == -1 && j2 == -1 ){
    i1 = j1 = 1;
    i2 = nx;
    j2 = ny; 
  }
  if( i1 >= i2 || j1 >= j2 || i1 < 1 || j1 < 1 || i2 > nx || j2 > ny ){
    error("error in input parameters");
    return 0.0;
  }//if
  if( nx <= 1 || ny <= 1 ){
    error("maximum undefined.");
    return 0.0;
  }//if

  i1 -= 1;
  i2 -= 1;
  j1 -= 1;
  j2 -= 1;

  float pixval, max = pixel[(i1+i2)/2 + (j1+j2)/2*(long)nx];

  int i;
  for( int j =j1; j <= j2; j++)
    for( i =i1; i <= i2; i++){
      pixval = pixel[i + j*nx];
      if( !isnan(pixval) )
	max =  MAX( max , pixval );
    }// end for( int i 

  success();
  return max;

}// end maximum()


/*****************************************************************************/
//
// FUNCTION    : total()
// DESCRIPTION : Calculates the integrated value. The sommation is 
//               internally done using a long double.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/*****************************************************************************/
double Image::total( void ) const
{

  setFunctionID("total");

  if( nx < 1 || ny < 1 ){
    error("the sum is not defined");
    return 0.0;
  }// end if

  double sum=0.0;
  long nxy = nx*ny;
  for( int i =0; i < nxy ; i++)
    if( (pixel[i] != blank || blank == 0.0) && !isnan( pixel[i]))
      sum += pixel[i];


  success();  
  return sum;
  
}// end total()

/*****************************************************************************/
//
// FUNCTION    : total()
// DESCRIPTION : Calculates the integrated value. The sommation is 
//               internally done using a long double.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/*****************************************************************************/
double Image::total( int i1, int j1, int i2, int j2 ) const
{
  setFunctionID("total");
    if( nx < 1 || ny < 1 || i2 < i1 || j2 < j1 || j1<= 0 || j2 <= 0 ||
	i2 > nx || j2 > ny){
        error("the sum is not defined");
        return 0.0;
    }// end if

    i1 -= 1;//fits convention
    i2 -= 1;
    j1 -= 1;
    j2 -= 1;

    int i , j;
    double sum=0.0, pix;

    for( j  = j1; j <= j2 ; j++){
      for( i  = i1; i <= i2 ; i++){
	pix = pixel[i + j*nx];
        if(  (pix != blank || blank == 0.0) && !isnan(pix))
            sum += pix;
      }
    }
    
    //    printf("total = %e\n", sum);
    success();
    return sum;

}// end total()

/*****************************************************************************/
//
// FUNCTION    : center()
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/*****************************************************************************/
Status Image::center(float *x, float *y, float *dr ) const
{
  setFunctionID("center");

  if( nx<= 0 || ny <= 0 )
    return error("no image present");


  float dr2, tot = total();

  int i , j;
  *x = 0.0;
  *y = 0.0;
  for( j = 0; j < ny ; j++)
    for(i = 0; i <nx; i++){
      *x+= i * pixel[i+j*nx];
      *y+= j * pixel[i+j*nx];
    }

  *x /= tot;
  *y /= tot;
  dr2 = 0.0;
  for( j = 0; j < ny ; j++)
    for(i = 0; i <nx; i++){
      dr2+= (SQR(i - *x) + SQR(j - *y)) * pixel[i+j*nx];
    }
  
  *x += 1; // for FITS definition of origin
  *y += 1;
  *dr = sqrt(dr2/tot);

  return success();
}// center()


/*****************************************************************************/
//
// FUNCTION    : setpixel()
// DESCRIPTION : Sets a pixel to a new value
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   i, j               I                 pixel position.
// val                  I                 new pixel value
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::setpixel( int i, int j, float val )
{

  if( i <= 0 || i > nx || j <= 0 || j> ny ){
    setFunctionID("set");
    return error("pixel does not exist");
  }


  pixel[i-1 + (j-1)*nx] = val; // NOTE that we correct for 
                               // the FORTRAN-FITS convention

  return SUCCESS;
}// setpixel()

/*****************************************************************************/
//
// FUNCTION    : get()
// DESCRIPTION : Return the pixel value of pixel (i,j)
//               Note that the inputs follows the FITS convention, 
//               so the first pixel 1,1
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   i, j               I                 pixel position.
//
//  RETURNS    : pixel value
//
/*****************************************************************************/
float Image::get( int i, int j ) const
{

  if( i > nx || j > ny || i <= 0  || j <= 0 ){
    //    setFunctionID("get()"); // set here for speed reasons
    //    JVMessages::warning("the requested coordinate falls outside the image");
    return 0.0;
  }

  return pixel[i-1 + (j-1)*nx];

}//end get()
float Image::get( int pixnr ) const
{
  if( pixnr < 0 || pixnr >= nx*ny){
    return 0.0;
  }

  return pixel[pixnr];
}

/*****************************************************************************/
//
// FUNCTION    : getMax( int i1, int j1, int i2, int j2 )
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   m, n               O                nx and ny
//
//  RETURNS    : #valid pixels
//
/*****************************************************************************/
float Image::getMax( int i1, int j1, int i2, int j2, 
		     int *imax, int *jmax ) const
{


  if( (i1 > i2) || (j1 > j2) || (i2 > nx) || (j2 > ny) || (i1 <= 0)  ||
      (j1 <= 0) ){    
    //    setFunctionID("get()"); // set here for speed reasons
    warning("the requested coordinates fall outside the image");
    return 0.0;
  }

  *imax = i1;
  *jmax = j1;
  float max = pixel[*imax-1 + (*jmax-1)*nx];
  for( int j = j1-1 ; j <= j2 -1; j++)
    for( int i = i1-1 ; i <= i2 -1; i++){
      if( !isnan(pixel[i + j*nx]) &&  pixel[i + j*nx]> max ){
	max = pixel[i + j*nx];
	*imax = i + 1;
	*jmax = j + 1;
      }
    }
      
  return max;

}//end get()


/*****************************************************************************/
//
// FUNCTION    : getMax( int i1, int j1, int i2, int j2 )
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   m, n               O                nx and ny
//
//  RETURNS    : maximum pixel value
//
/*****************************************************************************/
float Image::getMax( double x0, double y0, double rad,
		     int *imax, int *jmax ) const
{

  if( (x0 < 0.5) || (y0 < 0.5) || (x0 > nx ) || ( y0 > ny) ||  (rad < 1.0) ){
    //    setFunctionID("get()"); // set here for speed reasons
    warning("the requested coordinates fall outside the image");
    return 0.0;
  }


  int i1, i2, j1, j2;

  x0 -= 1.0;
  y0 -= 1.0; // C++ instead of Fortran

  i1 = MAX( (int)( x0 - rad - 1.0), 0);
  i2 = MIN( (int)( x0 + rad + 1.0), nx - 1);

  j1 = MAX( (int)( y0 - rad - 1.0), 0);
  j2 = MIN( (int)( y0 + rad - 1.0), ny - 1);


  *imax = i1+1;
  *jmax = j1+1;
  float max = pixel[*imax-1 + (*jmax-1)*nx];
  for( int j = j1 ; j <= j2 ; j++)
    for( int i = i1 ; i <= i2 ; i++){
      if( !isnan(pixel[i + j*nx]) &&  (SQR(i - x0) + SQR(j - y0) <= rad*rad) && pixel[i + j*nx]> max ){
	max = pixel[i + j*nx];
	*imax = i + 1;
	*jmax = j + 1;
      }//if
    }//for
      
  return max;

}//end get()

/*****************************************************************************/
//
// FUNCTION    : getMax( int i1, int j1, int i2, int j2 )
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   m, n               O                nx and ny
//
//  RETURNS    : maximum pixel value
//
/*****************************************************************************/
Status Image::weightedPosition( double *x0, double *y0, double rad) const
{
  
  setFunctionID("weightedPosition");

  if( *x0 < 0.5 || *y0 < 0.5 || *x0 > nx || *y0 > ny || rad < 1.0)
    return error("the requested coordinates fall outside the image");


  double xpos=0.0, ypos=0.0, pixval, totval=0.0, offset=0.0;
  int i1, i2, j1, j2;

  *x0 -= 1.0;
  *y0 -= 1.0; // C++ instead of Fortran

  i1 = MAX( (int)( *x0 - rad - 1.0), 0);
  i2 = MIN( (int)( *x0 + rad + 1.0), nx - 1);

  j1 = MAX( (int)( *y0 - rad - 1.0), 0);
  j2 = MIN( (int)( *y0 + rad - 1.0), ny - 1);

  offset = -1.0 * minimum( i1, j1, i2, j2);
  if( offset < 0.0 )
    offset = 0.0;
  for( int j = j1 ; j <= j2 ; j++)
    for( int i = i1 ; i <= i2 ; i++){
      if( !isnan(pixel[i + j*nx]) &&  (SQR(i - *x0) + SQR(j - *y0) <= rad*rad) ){
	pixval = pixel[i + j*nx] + offset;
	totval += pixval;
	xpos += pixval * (i - *x0);
	ypos += pixval * (j - *y0);
      }//if
    }//for

  if( totval > 0.0 ){
    xpos /= totval;
    ypos /= totval;
    *x0 = xpos + *x0 + 1.0;
    *y0 = ypos + *y0 + 1.0;
  }
  else
    return error("Could not determine weighted pixel position");
      
  return success();
}//weightedPosition

/*****************************************************************************/
//
// FUNCTION    : getnxny()
// DESCRIPTION : Return the size of the image as m, n, and the total valid
//               pixels as return value;
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   m, n               O                nx and ny
//
//  RETURNS    : #valid pixels
//
/*****************************************************************************/
long Image::getnxny( int *m, int *n ) const
{

  *m= nx;
  *n = ny;

  long nxy = nx * ny;
  if( nxy > 0 ){
    float pix;
    long npix = 0;
    for( long i = 0; i < nxy; i++){
      pix = pixel[i];
      if( !isnan(pix) && (pix != blank || blank == 0.0 ))
	npix++;
    }//for
    return npix;
  }//if

  return nxy;
}//end getnxny()


/*****************************************************************************/
//
// FUNCTION    : getnxny()
// DESCRIPTION : Return the size of the image
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   m, n               O                nx and ny
//
//  RETURNS    : SUCCESS
//
/*****************************************************************************/
Status Image::getChanMinMax( int *chMin, int *chMax ) const
{

  *chMin = chanMin;
  *chMax = chanMax;

  return SUCCESS;
}//end getnxny()

/*****************************************************************************/
//
// FUNCTION    : getPixelsize()
// DESCRIPTION : Return the pixel scales of the image
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   m, n               O                nx and ny
//
//  RETURNS    : SUCCESS
//
/*****************************************************************************/
Status Image::getPixelsize( double *xscale, double *yscale) const
{

  if( cdelt1 != 1.0 && cdelt2 != 1.0 ){
    *xscale = cdelt1;
    *yscale = cdelt2;
    //    message("cdelt1 = ",cdelt1);
    //    message("cdelt2 = ",cdelt2);
  }else{
    warning("PLEASE CHECK THIS PIXEL SIZE");
    double scale=sqrt( (SQR(pc001001) + SQR(pc001002)+ SQR(pc002001)+SQR(pc002002))/2.0);
    *xscale= -scale;
    *yscale= scale;
  }

  return SUCCESS;
}//end getPixelsize( )


/*****************************************************************************/
//
// FUNCTION    : statistics( )
// DESCRIPTION : 
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
/*****************************************************************************/
Status Image::statistics( float *mean, float *sdev, 
			  float *skewn, float *kurt,
			  int i1, int j1, int i2, int j2)
{

  setFunctionID("statistics");

  if( i1 == -1 && j1 == -1 && i2 == -1 && j2 == -1 ){
    i1 = j1= 1;
    i2 = nx;
    j2 = ny;
  }

  *mean = 0.0;
  *sdev = 0.0;
  if( i1 >=  i2 || j1 >= j2  || i1 < 1 || j1 < 1 || i2 > nx || j2 >ny)
    return error("invalid input parameters");

  int n, m;
  long int npix = getnxny( &m, &n);
  *mean = total( i1, j1, i2, j2)/npix;
  i1 -= 1;
  j1 -= 1;
  i2 -= 1;
  j2 -= 1;


  int i;
  long ncheck=0;
  float pix, ressum2=0.0, ressum3=0.0, ressum4=0.0, res2;
  for( int j = j1 ; j <= j2 ; j++)
    for( i = i1 ; i <= i2 ; i++){
      pix = pixel[i + j*nx];
      if( !isnan(pix) && (pix != blank || blank == 0.0)){
	res2 =  SQR(pix - *mean); 
	ressum2 += res2;
	ressum3 += res2*(pix - *mean);
	ressum4 += res2*res2;
	ncheck++;
      }//if
    }//for



  *sdev = sqrt( ressum2/npix );

  message("npix       = ", npix);
  message("check npix = ", ncheck);
  message("std deviation = ", *sdev);

  if( *sdev != 0.0 ){
    *skewn = ressum3/( SQR(*sdev) * (*sdev) *npix );
    *kurt  = ressum4/(SQR(*sdev)*SQR(*sdev)*npix ) - 3.0;
  }
  else
    *skewn = *kurt = 0.0;

  return success();
}// statistics()

// calculates median pixel value
//
// Simple sorting routine used by medianfilter() and selectregion()
//
// inline for speed reasons
inline void sort(float data[], int n)
{
  float a;
  int i, j;

  if(n<2 ){
    data[0] = 0.0;
    return;
  }

  for( j = 1; j < n; j++){
    a = data[j];
    i= j-1;
    while( i>= 0 &&  data[i] > a ){
      data[i+1] = data[i];
      i--;
    }//while
    data[i+1] = a;
  }//for
  
}//end sort()

//
// based on select
// from Numerical Recipes in C, Press et al.
#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;



double Image::median( bool ignoreZero ) const
{
  setFunctionID("median");
  double med=0.0;
  int n=nx*ny;

  if( n == 0 ){
    error("empty image");
    return 0.0;
  }//if

  float *values = new float [n];
  float val;
  int j=0;
  for(int i=0; i < n ; i++){
    val=pixel[i];
    if( val != 0.0 || ignoreZero == false ){
      values[j++] = pixel[i];
    }
  }//for

  //  message("number of pixels = ", n);
  //  message("number of values copied = ", j);

  int k;
  if( j > 0 ){
    sort(values,j);
    k=(int)(j/2);
    if( j % 2 == 0)
      med=(values[k-1] + values[k])/2.0;
    else
      med=values[(int)(j/2)];
    message("median index = ", k);
    message("median value = ", values[k]);
  }else{
    error("no values copied");
    return med;
  }

#ifdef USEFFTW
  for(int i=MAX(0,k-3); i < MIN(k+3,j); i++)
    printf("k=%i v=%.3e\n",i, values[k]);
  message("median = ",med);
#endif

  delete [] values;
  return med;
}

/*****************************************************************************/
//
// FUNCTION    : setExposure()
// DESCRIPTION : Sets the exposure time to a new value.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      expotime        I                        new exp. time value
//
//  RETURNS    :  SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::setExposure( double expotime )
{
  setFunctionID("setExposure");

  message("setting exposure time value to ", expotime); 
  if( exposure <= 0.0 )
    warning("possibly an invalid exposure time");
  
  exposure = expotime;
  
  return success();
    
}//end setExposure()


/*****************************************************************************/
//
// FUNCTION    : set()
// DESCRIPTION : Set pixel (i, j) to a new value.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//     val              I               new value of pixel (i,j)
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::set(int i , int j, float val )
{

  if( i > nx || j > ny || i <= 0  || j <=0 ){
    setFunctionID("set");
    message("nx = ",nx);
    message("ny = ",ny);
    message("i = ",i);
    message("j = ",j);
    return error("pixel coordinate is not defined");
  }

  
  i -= 1;
  j -= 1;
  pixel[j*nx + i] = val;

  return SUCCESS;

}//end set()



/****************************************************************************/
//
// FUNCTION    : setexposure()
// DESCRIPTION : Changes the exposure keyword
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS
//
/****************************************************************************/
Status Image::setexposure(float newexposure  )
{
  exposure = newexposure;
  return SUCCESS;
}// setexposure()


/****************************************************************************/
//
// FUNCTION    : setCoordinates()
// DESCRIPTION : Changes the world coordinates.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS
//
/****************************************************************************/
Status Image::setCoordinates( float nwcrval1, float nwcrval2, 
			      float nwcrpix1, float nwcrpix2, 
			      float nwcdelt1, float nwcdelt2, 
			      float nwcrota2, float nwequinox,
			      const char ct1[], const char ct2[],
			      const char sys[] )
{
  setFunctionID("setCoordinates");
  double s=1.0, todeg = 180.0/M_PI;
  //  warning("RA/DEC - TAN /FK5 system is assumed.");

  crval1 = nwcrval1;
  crval2 = nwcrval2;
  crpix1 = nwcrpix1;
  crpix2 = nwcrpix2;
  cdelt1 = nwcdelt1;
  cdelt2 = nwcdelt2;
  crota2 = nwcrota2;
  equinox = nwequinox;
  strcpy(ctype1,ct1);
  strcpy(ctype2,ct2);
  strcpy(radecsys, sys);
  setprojection();

  pc001001 = 1.0;
  pc001002 = 0.0;
  pc002001 = 0.0;
  pc002002 = 1.0;

  if( crota2 != 0.0 ){
    if( cdelt1 != 0.0 && cdelt2 != 0.0 ){
      s = cdelt2/cdelt1;
    }//if
    pc001001 =  cos(crota2/todeg);
    pc001002 = -sin(crota2/todeg) * s;
    pc002001 =  sin(crota2/todeg) / s;
    pc002002 =  cos(crota2/todeg);
  }//if

  return success();
}// setCoordinates()


/****************************************************************************/
//
// FUNCTION    : getCoordinates()
// DESCRIPTION : Get the world coordinate system.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS
//
/****************************************************************************/
Status Image::getCoordinates( float *nwcrval1, float *nwcrval2, 
				   float *nwcrpix1, float *nwcrpix2, 
				   float *nwcdelt1, float *nwcdelt2, 
				   float *nwcrota2, float *nwequinox) const
{


  *nwcrval1 = crval1;
  *nwcrval2 = crval2;
  *nwcrpix1 = crpix1;
  *nwcrpix2 = crpix2;
  *nwcdelt1 = cdelt1;
  *nwcdelt2 = cdelt2;
  *nwcrota2 = crota2;
  *nwequinox = equinox;

  return SUCCESS;
}// getCoordinates()

/****************************************************************************/
//
// FUNCTION    : getCoordinates()
// DESCRIPTION : Get the world coordinate system.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS
//
/****************************************************************************/
Status Image::getCoordinates( float *nwcrval1, float *nwcrval2, 
				   float *nwcrpix1, float *nwcrpix2, 
				   float *nwcdelt1, float *nwcdelt2, 
				   float *nwcrota2, float *nwequinox,
				   char *oradecsys, char *octype1, 
				   char *octype2) const
{


  *nwcrval1 = crval1;
  *nwcrval2 = crval2;
  *nwcrpix1 = crpix1;
  *nwcrpix2 = crpix2;
  *nwcdelt1 = cdelt1;
  *nwcdelt2 = cdelt2;
  *nwcrota2 = crota2;
  *nwequinox = equinox;


  strncpy(oradecsys,radecsys,FLEN_VALUE-1);
  strncpy(octype1, ctype1, FLEN_VALUE-1);
  strncpy(octype2, ctype2, FLEN_VALUE-1);

  return SUCCESS;
}// getCoordinates()


/****************************************************************************/
//
// FUNCTION    : setblank()
// DESCRIPTION : Change the value indicating an undefined value.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS
//
/****************************************************************************/
Status Image::setblank( float val )
{
  
  long nxy = nx*ny;
  for( int i =0; i < nxy ; i++)
  {
    if( pixel[i] == blank )
    {
      pixel[i] = val;
    }// end if
  }// end for( int i
  
  blank = val;
  
  return SUCCESS;

}// end setblank()


/*****************************************************************************/
//
// FUNCTION    : rebin()
// DESCRIPTION : Rebin the spectrum with a value n>0 and nx,ny should be 
//               an integer value of nx, ny.
//               First contract all neighbour pixels to one pixel and then
//               shift those pixels.
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      n               I                 rebinning factor
//
//  RETURNS    :  SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::rebin( int n )
{
  setFunctionID("rebin(image)");

  if( n < 1 )
    return error("wrong input image");


  if( nx % n != 0 || ny % n != 0 ){

    warning("the image size is not a multiple of the bin factor");
    warning("the image size will be first increased");
    message("Old nx = ", nx);
    message("Old nx = ", ny);
    if( subset( 1, 1,  n*(nx/n + 1),  n*(ny/n + 1), 0.0) == FAILURE ){
      setFunctionID("rebin(image)");
      return error("could not resize the input image");
    }
    setFunctionID("rebin(image)");
    message("New nx = ", nx);
    message("New nx = ", ny);
  }

  if( n==1 ){
    warning("nothing happened"); 
    return success();
  }// end if

  float newval;
  bool undef = FALSE;
  int newnx = nx/n;
  int newny = ny/n;
  int oldi, oldj;

  int i, j;
  register int  k, l;
  for( j = 0; j < newny ; j++){
      for( i = 0 ; i < newnx ; i++){

	newval = 0.0;
	for( k = 0; k < n && !undef; k++ ){  // add neighbouring pixels
	  for( l = 0; l < n && !undef; l++ ){
	    oldi = (i*n + k);
	    oldj = (j*n + l);
	    if( (pixel[ oldi + oldj * nx]== blank && blank!=0.0) || 
		oldi >= nx || oldj >= ny ){
	      undef = true; // undefined pixel value encountered   
	    }
	    else{
	      newval+= pixel[(i*n + k) + (j*n + l)*nx];
	    }// end if
	  }//end for(l
	}//end for(k
	if( i < newnx && j < newny ){
	  if( undef == true ){
	    pixel[i + j*newnx] = blank;     // assign value to the pixel
	    undef = FALSE;
	  }
	  else{
	    pixel[i + j*newnx] = newval;
	  }//end if
	}// end if

      }//end for j
    }//end for i


    nx = newnx;
    ny = newny;
    // correct the scale keywords
    cdelt1 *= n;
    cdelt2 *= n;
    crpix1  = (crpix1-0.5)/n + 0.5; // FORTRAN convention first pixel=(1,1)
    crpix2  = (crpix2-0.5)/n + 0.5;
    imgbin *= n;

    return success();
}// rebin()

/*****************************************************************************/
//
// FUNCTION    : rebiny()
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  n                   I             rebin factor
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::rebinx( int n )
{

  setFunctionID("rebinx");

  if( n < 1 )
      return error("incorrect input image");


  if( nx % n != 0 ){
    warning("the size of the image is not a multiple of the rebinning factor");
    //    return FAILURE;
  }// end if


  if( n == 1 ){
    warning("nothing happened"); 
    return success();
  }// end if

  float newval;
  bool undef = FALSE;
  int newnx = nx/n;
  int oldi;

  int i, j;
  register int  k;
  for( j = 0; j < ny ; j++){
    for( i = 0 ; i < newnx ; i++){

	newval = 0.0;
	for( k = 0; k < n ; k++ ){
	  oldi = (i*n + k);
	  if( (pixel[ oldi + j * nx]== blank && blank!=0.0) || oldi >= nx )
	    undef = true; // undefined pixel value encountered   
	  else
	    newval+= pixel[ oldi  + j*nx];
	}//end for(k

	if( i < newnx ){
	  if( undef == true ){
	    //       	    pixel[i + j*newnx] = blank;     
	    undef = FALSE;
	  }
	  //	  else
	  pixel[i + j*newnx] = newval;
	}// end if

      }//end for j
    }//end for i


    nx = newnx;
    // correct the scale keywords
    cdelt1 *= n;
    crpix1  = (crpix1-0.5)/n + 0.5; // FORTRAN convention first pixel=(1,1)
    imgbin *= n;

    return success();
}// rebinx()

/*****************************************************************************/
//
// FUNCTION    : rebiny()
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  n                   I             rebin factor
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::rebiny( int n )
{
  setFunctionID("rebiny");

  if( n < 1 )
    return error("incorrect input image");

  if( ny % n != 0 ){
    warning("the size of the image is not a multiple of the rebinning factor");
    //    return FAILURE;
  }// end if


  if( n == 1 ){
    warning("nothing happened"); 
    return success();
  }// end if

  float newval;
  bool undef = false;
  int newny = ny/n;
  int oldj;

  int i, j;
  register int  k;
  for( i = 0 ; i < nx ; i++){
    for( j = 0; j < newny ; j++){

	newval = 0.0;
	for( k = 0; k < n ; k++ ){
	  oldj = (j*n + k);
	  if( (pixel[ i + oldj * nx]== blank && blank!=0.0) || oldj >= ny )
	    undef = true; // undefined pixel value encountered   
	  else
	    newval+= pixel[i + oldj * nx];
	}//end for(k

	if( j < newny ){
	  if( undef == true ){
	    //       	    pixel[i + j*newnx] = blank;     
	    undef = FALSE;
	  }
	  //	  else
	  pixel[i + j*nx] = newval;
	}// end if

      }//end for j
    }//end for i


    ny = newny;
    // correct the scale keywords
    cdelt2 *= n;
    crpix2  = (crpix2-0.5)/n + 0.5; // FORTRAN convention first pixel=(1,1)
    imgbin *= n;

    return success();
}// rebiny()

/*****************************************************************************/
//
// FUNCTION    : rebin()
// DESCRIPTION : Rebin the spectrum with a value n>0 and nx,ny should be 
//               an integer value of nx, ny.
//               First contract all neighbour pixels to one pixel and then
//               shift those pixels.
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      n               I                 rebinning factor
//
//  RETURNS    :  SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::rebin( const Image &im, int n )
{
  setFunctionID("rebin(image)");

  if( n < 1 || &im == this)
    return error("wrong input image");


  if( im.nx % n != 0 || im.ny % n != 0 || im.nx*im.ny <= 0.0)
    return error("the image size is not a multiple of the bin factor");



  if( n==1 ){
    warning("nothing happened"); 
    return success();
  }// end if

  if( allocate(im.nx/n, im.ny/n, 0.0) == FAILURE)
    return error("allocation error");

  copy_keywords( im );
  blank = im.blank;

  float newval;
  bool undef = FALSE;
  int oldi, oldj;

  int i, j;
  register int  k, l;
  for( j = 0; j < ny ; j++){
      for( i = 0 ; i < nx ; i++){

	newval = 0.0;
	for( k = 0; k < n && !undef; k++ ){  // add neighbouring pixels
	  for( l = 0; l < n && !undef; l++ ){
	    oldi = (i*n + k);
	    oldj = (j*n + l);
	    if((im.pixel[ oldi + oldj * im.nx]== im.blank && im.blank!=0.0) || 
		oldi >= im.nx || oldj >= im.ny ){
	      undef = true; // undefined pixel value encountered   
	    }
	    else{
	      newval+= im.pixel[oldi + oldj*im.nx];
	    }// end if
	  }//end for(l
	}//end for(k
	if( i < nx && j < ny ){
	  if( undef ){
	    pixel[i + j*nx] = blank;     // assign value to the pixel
	    undef = FALSE;
	  }
	  else{
	    pixel[i + j*nx] = newval;
	  }//end if
	}// end if

      }//end for j
    }//end for i


    // correct the scale keywords
    cdelt1 *= n;
    cdelt2 *= n;
    crpix1  = (crpix1-0.5)/n + 0.5; // FORTRAN convention first pixel=(1,1)
    crpix2  = (crpix2-0.5)/n + 0.5;
    imgbin *= n;

    return success();
}// rebin()

/****************************************************************************/
//
// FUNCTION    : block()
// DESCRIPTION : Bins the data but with keeping the number of pixels
//               and averaging rather than adding blocks of pixels.
//               
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
Status Image::block( int n)
{
  setFunctionID("block");

  if( n < 1 )
    return error("incorrect input image");


  if( nx % n != 0 || ny % n != 0 )
    return error("the size of the image is not a multiple of the rebinning factor");


  if( n==1 ){
    warning("nothing happened!"); 
    return success();
  }// end if

  Image temp;
  if( temp.rebin( *this, n)== FAILURE )
    return error("error in a function called");

  for( int j=0 ; j < ny; j++ )
    for( int i=0 ; i < nx; i++ ){

      pixel[i + j*nx] = temp.pixel[i/n + j/n*temp.nx]/(n*n);

    }//for

  return success();
}//block()

/****************************************************************************/
//
// FUNCTION    : scaledown()
// DESCRIPTION : Rebinning with a floating point factor.
//               
//               
//               
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//
//  RETURNS    :  SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::scaledown( double xscale, double yscale )
{

  setFunctionID("scaledown");

  if( nx <= 0 || ny <= 0 || xscale > 10.0 || yscale > 10.0 || 
      xscale <= 0.01 || yscale <= 0.01 )
    return error("incorrect input parameters");


  // Note that this function uses the scales as reciproces, so
  xscale = 1/xscale; 
  yscale = 1/yscale; 

  setblank( 0.0 );

  int newnx, newny;
  float scalemax = MAX( xscale, yscale );
  Image newim( *this );

  newnx = (int)( nx/xscale + 0.5);
  newny = (int)( ny/yscale + 0.5);

  if( newim.allocate( newnx, newny, 0.0 ) == FAILURE)
    return error("allocation error");

  if( xscale <= 1.0 && yscale <= 1.0 ){
    float xweight, yweight;  
    int i, j, k0, l0, kmax, lmax;;
    register int k, l;
    kmax = (int) ceil( xscale );
    lmax = (int) ceil( yscale );
    
    for( j = 0; j < newny ; j++){
      for( i = 0 ; i < newnx ; i++){
	
	k0 = (int) floor( i * xscale );
	l0 = (int) floor( j * yscale );
	for( l = 0 ; l <= lmax  &&   (l+l0) < ny ; l++){
	  for( k = 0 ; k <= kmax   && (k+k0) < nx ; k++ ){
	    //
	    // Calculate weights
	    //
	    xweight = 1.0;
	    if( ((i+1)*xscale - (k+k0)) < 1.0 ){
	      xweight = MAX( (i+1)*xscale - (k0+k), 0.0); 
	    }
	    if( i*xscale - (k0+k) > 0.0 ){
	      xweight = 1.0 - (i*xscale - (k0+k)); 
	    }
	    yweight = 1.0;
	    if( ((j+1)*yscale - (l+l0)) < 1.0 ){
	      yweight = MAX((j+1)*yscale - (l0+l), 0.0); 
	    }
	    if( j*yscale - (l0+l) > 0.0 ){
	      yweight = 1.0 - (j*yscale - (l0+l)); 
	    }

	    newim.pixel[i + j*newnx] += xweight*yweight * pixel[k+k0+(l+l0)*nx];
	    
	  }//for k
	}//for l
	
      }//for i
    }//for j
  }
  else{
    message("stretching is still under construction");
    int k0, l0;
    for( int j = 0; j < newny ; j++){
      for( int i = 0 ; i < newnx ; i++){
	k0 = (int)( i * xscale + 0.5);
	l0 = (int)( j * yscale + 0.5);
	if( k0 >= 0 && k0 < nx && l0 >= 0 && l0 < ny)
	  newim.pixel[i + j*newnx] = pixel[k0 + l0*nx];
      }//for
    }//for
  }//else

  newim.crpix1 = (newim.crpix1 - 0.5)/xscale + 0.5;
  newim.crpix2 = (newim.crpix2 - 0.5)/yscale + 0.5;
  newim.cdelt1 *= scalemax;
  newim.cdelt2 *= scalemax;

  newim.pc001001 *= xscale/scalemax;
  newim.pc001002 *= yscale/scalemax;
  newim.pc002001 *= xscale/scalemax;
  newim.pc002002 *= yscale/scalemax;

  newim /= xscale*yscale;
  newim.reset_detector_keywords();

  *this = newim;

  return success();
}// end 

/*****************************************************************************/
//
// FUNCTION    : zoom2()
// DESCRIPTION : This will be a copy of the input image, but a factor 2 larger
//               in size. 4 pixels will have the same value corresponding
//               to the value of the corresponding pixel of the input image.
//               The rationale to use an inout image is that zoom can be used
//               for displaying programs,  in which case you want to keep
//               the original image.
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
Status Image::zoom2( Image im )
{
  setFunctionID("zoom2");

  if( im.nx < 1 || im.ny < 1)
    return error("image does not contain data");

  if( allocate( 2*im.nx, 2*im.ny) == FAILURE )
    return error("allocation error");


  copy_keywords( im );

  cdelt1 *= 0.5;
  cdelt2 *= 0.5;
  crpix1 = 2.0*(im.crpix1 - 0.5) + 0.5;
  crpix2 = 2.0*(im.crpix2 - 0.5) + 0.5;
  // crval1/2 does not change

  int i;
  for( int j =0 ; j < im.ny ; j++){
    for( i =0 ; i < im.nx ; i++){
      pixel[ 2*i   + 2*j*    nx] = im.pixel[ i + j*im.nx];
      pixel[ 2*i+1 + 2*j*    nx] = im.pixel[ i + j*im.nx];
      pixel[ 2*i   + (2*j+1)*nx] = im.pixel[ i + j*im.nx];
      pixel[ 2*i+1 + (2*j+1)*nx] = im.pixel[ i + j*im.nx];
    }//for i
  }//for j

  return success();
}//zoom2()

Status Image::zoom( Image im, int factor )
{
  setFunctionID("zoom");

  if( im.nx < 1 || im.ny < 1)
    return error("image does not contain data");
  if( factor <= 1 || nx * factor > MAX_SIZE)
    return error("invalid zoom facror ");
  if( allocate( factor*im.nx, factor*im.ny) == FAILURE )
    return error("allocation error");

  copy_keywords( im );

  cdelt1 /= factor;
  cdelt2 /= factor;
  crpix1 = factor*(im.crpix1 - 0.5) + 0.5;
  crpix2 = factor*(im.crpix2 - 0.5) + 0.5;
  // crval1/2 does not change

  int i, i0, j0;
  for( int j =0 ; j < im.ny ; j++){
    for( i =0 ; i < im.nx ; i++){
      i0 = factor*i;
      j0 = factor*j;
      for( int k=0; k < factor; k++){
	for( int l=0; l < factor; l++){
	  pixel[ i0 + k + (j0+l)*nx] = im.pixel[ i + j*im.nx];
	}
      }

    }//for i
  }//for j

  return success();
}//zoom()

Status Image::zoom( int factor )//in place
{
  //  setFunctionID("zoom");

  Image im= *this;

  return  this->zoom( im, factor);

}//zoom



/****************************************************************************/
//
// FUNCTION    : rotate()
// DESCRIPTION : Rotates the image.
//               
//               
//               
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      angle              I                 roatation angle in degrees
//
//  RETURNS    :  SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::rotate( double angle )
{
  setFunctionID("rotate");

  if( nx <= 0 || ny <= 0  )
    return error("no image present");

  setblank(0.0);

  int i, j, di, dj, oldi, oldj;
  double sina = sin(angle * M_PI/180.0);
  double cosa = cos(angle * M_PI/180.0);
  double x, y, xc, yc, dx, dy, pixelval, s=1.0;
  Image newim;

  newim.allocate(nx, ny, blank);
  newim.copy_keywords( *this );

  xc = nx/2;
  yc = ny/2;
  if( cdelt1 != 0.0 && cdelt2 != 0.0){
    s = cdelt2/cdelt1;
  }
  newim.crota2 = crota2 + angle;
  newim.pc001001 = pc001001 * cosa   - pc002001 * sina*s;
  newim.pc001002 = pc001002 * cosa   - pc002002 * sina*s;
  newim.pc002001 = pc001001 * sina/s + pc002001 * cosa;
  newim.pc002002 = pc001002 * sina/s + pc002002 * cosa;
  newim.crpix1 =  (crpix1-0.5-xc) * cosa + (crpix2-0.5-yc) * sina + xc + 0.5;
  newim.crpix2 = -(crpix1-0.5-xc) * sina + (crpix2-0.5-yc) * cosa + yc + 0.5;

  newim.crpix1 =  (crpix1-0.5-xc) * cosa - (crpix2-0.5-yc) * sina + xc + 0.5;
  newim.crpix2 =  (crpix1-0.5-xc) * sina + (crpix2-0.5-yc) * cosa + yc + 0.5;



  // Note the 0.5 comes from the fits definition of the image origin

  for( j = 0; j < newim.ny ; j++ ){
    for( i = 0 ; i < newim.nx; i++){
      x =  (i-xc) * cosa + (j-yc) * sina + xc;   // inverse rotation
      y = -(i-xc) * sina + (j-yc) * cosa + yc;
      oldi = (int)( x + 0.5);
      oldj = (int)( y + 0.5);
      dx = x - oldi;
      dy = y - oldj;
      if( dx > 0 ){
	di= 1;
      }
      else{
	di = -1;
      }
      if( dy > 0 ){
	dj = 1;
      }
      else{
	dj = -1;
      }
      dx=fabs(dx);
      dy=fabs(dy);

      // Interpolate:
      if(  x < 0.0 || y < 0.0 || oldi >= nx || oldj >= ny){
	pixelval=blank;
      }
      else{
	pixelval = (1.0-dx)*(1.0-dy) * pixel[oldi + oldj*nx];
	if( oldi+di >= 0 && oldj+dj < nx )
	{
	  pixelval += (1.0-dy) * dx * pixel[oldi+di + oldj*nx];
	  if( oldj+dj >= 0 && oldj+dj < ny ){
	    pixelval += dx*dy * pixel[oldi+di + (oldj+dj)*nx];
	  }
	}
	if( oldj+dj >= 0 && oldj+dj < ny ){
	  pixelval += (1.0-dx)*dy * pixel[oldi+di + (oldj+dj)*nx];
	}
      }// else
      newim.pixel[i + j*nx] = pixelval;
    }// for i
  }// for j


  *this = newim;
  return success();

}// rotate()

/****************************************************************************/
//
// FUNCTION    : orientate()
// DESCRIPTION : Aligns an image North-South invoking routine rotate
//               
//               
//               
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :  SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::orientate( void )
{
  setFunctionID("orientate");

  if( crota2 == 0.0 ){
    warning("image was already orientated North-South");
    return success();
  }//if

  return rotate( -crota2 );
}// orientate()

/****************************************************************************/
//
// FUNCTION    : rescale()
// DESCRIPTION : Rescales the image into discrete intensities ranging from
//               val low to high
//
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      newxy              I                  new size nx=ny
//
//  RETURNS    :  SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::rescale(int low, int high)
{

  setFunctionID("rescale");

  if( low >= high || nx*ny <= 0 )
    return error("incorrect parameters or no image present");


  float min = minimum();
  float max = maximum();
  float slope;
  long nxy=nx*ny;

  if( max - min <= 0.0 )
    return error("rescaling not possible");

  setblank(min); // so the blank pixels are the lowest possible 

  slope = (high - low)/(max-min);
  for(long i= 0; i < nxy; i++)
  {
    pixel[i] = (int)( slope * (pixel[i] - min) + low + 0.5);
  }//for

  bscale = 1.0/slope;
  bzero = min - low/slope;

  return success();
}//rescale()


/*****************************************************************************/
//
// FUNCTION    : subset()
// DESCRIPTION : Changes the image to a subset of the original.
//               Note that this function can also enlarge an image if
//               e.g. x1 or y1 < 0.
//
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      newxy              I                  new size nx=ny
//
//  RETURNS    :  SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::subset( int x1, int y1, int x2, int y2, float value )
{
  setFunctionID("subset");

  if( nx*ny <= 0 ||
      x2 <= x1 || y2 <= y1 || x2-x1 > MAX_SIZE || y2-y1 > MAX_SIZE  )
    return error("the new image size is not allowed");


  setblank(0.0);
  int newnx, newny, oldi, oldj;
  Image newim;

  x1 -= 1; // because of the fits convention (image starts at 1,1)
  y1 -= 1;
  x2 -= 1;
  y2 -= 1;
  newnx = x2-x1 + 1;
  newny = y2-y1 + 1;

  if( newim.allocate( newnx, newny, value ) == FAILURE){
    return error("allocation error");
  }//


  int i;
  double tot=0.0;
  for(int j = 0; j < newim.ny; j++){
    for(i = 0; i < newim.nx; i++){
      oldi = x1 + i;
      oldj = y1 + j;
      if( oldi >= 0 && oldj >= 0 && oldi < nx && oldj < ny){ 
	tot+= newim.pixel[ i + j * newim.nx] = pixel[ oldi + oldj * nx];
      }//if
    }//for i
  }//for j

  newim.copy_keywords(*this);
  newim.crpix1 -= x1; // actually newim.crpix1 -= x1+1-1;
  newim.crpix2 -= y1; // adding a 1 to x1 because of fits convention and
                      // adding a 1 to crpix1 for the same reason
                      // e.g if (x1+1)=crpix1 then crpix1 is the first pixel=1
  *this = newim;

  message("new pixel integration: ", tot);
  return success();
}//end subset()


/*****************************************************************************/
//
// FUNCTION    : resize()
// DESCRIPTION :
//
//
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      newxy              I                  new size nx=ny
//
//  RETURNS    :  SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::resize(int newxy)
{
  setFunctionID("resize");

  if( newxy > MAX_SIZE || newxy > MAX_SIZE || newxy < 2 )
    return error("the new image size is not allowed");


  int deltax, deltay, x1, x2, y1, y2;

  if( nx > newxy ){
    deltax = (nx - newxy)/2;
  }
  else{
    deltax = -(newxy - nx)/2;
  }
  if( ny > newxy ){
    deltay = (ny - newxy)/2;
  }
  else{
    deltay = -(newxy - ny)/2;
  }
  x1 = deltax + 1;  // if deltax=0 the pixel starts at (1,1): FITS conv.
  x2 = nx - deltax;
  y1 = deltay + 1;
  y2 = ny - deltay;

  deltax =  x2-x1+1 - newxy;
  if( deltax != 0.0 ) 
    x1 += deltax;
  deltay =  y2-y1+1 - newxy;
  if( deltay != 0.0 ) 
    y1 += deltay;

  subset( x1, y1, x2, y2);

  return success();
}//end resize()

/*****************************************************************************/
//
// FUNCTION    : convertNaN()
// DESCRIPTION : Checks for "not a number" pixels and sets them to zero.
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
Status Image::convertNaN(float replaceVal)
{

  setFunctionID("convertNaN");
  if( nx <= 0 || ny <= 0)
    return error("no image present");

  long nnan=0;
  long int n=nx*ny;
  for(long i=0; i < n; i++){
    if( isnan( pixel[i]) ){
      pixel[i] = replaceVal;
      nnan++;
    }
  }

  if( nnan > 0 )
    warning("NaN pixels have been converted to value ", replaceVal);

  return success();
}//



