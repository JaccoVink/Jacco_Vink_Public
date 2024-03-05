/*****************************************************************************/
//
// FILE        :  image_io.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  In/out functions of the astroimage class.
//
//
// COPYRIGHT   : Jacco Vink,  1999, 2002
//
/*****************************************************************************/
//  MODIFICATIONS :     
//  4/2001              
//
/*****************************************************************************/
#include <strings.h>
#include "jvtools_image.h"
#include "jvtools_messages.h"
#include "jvtools_extraction.h"

using namespace::jvtools;
using namespace::JVMessages;
float mexicanHat(float x, float x0, float sigma);
float gaussian(float x, float x0, float sigma);

/*****************************************************************************/
//
// FUNCTION    :  info()
// DESCRIPTION :  Prints out useful information about the data read.
// 
//
// SIDE EFFECTS:	Assumes testStream is connected to the right writeable
//			file (e.g. stdout).
//
// PARAMETERS  :
//   NAME         I/O         DESCRIPTION
//   testStream   I/O	Assumes testSTream is connected to the right file.
//
//  RETURNS    :	FAILURE if an stream error occured, SUCCESS otherwise.
//
/*****************************************************************************/
//Status Image::info( FILE *testStream ) const
//{
//  return info(testStream);
//}
Status Image::testPrint( FILE *testStream ) const
{
  char topline[LOGOWIDTH], bottomline[LOGOWIDTH];

  if( ferror(testStream) ){
    fclose(testStream);
    return FAILURE;
  }//if

  for( int i = 0; i < LOGOWIDTH-1 ; i++){
    bottomline[i] = BORDERCHAR;
  }//for
  bottomline[LOGOWIDTH-1] =  EOSTR;
  inserttext( bottomline,"IMAGE INFORMATION", topline );

  fprintf(testStream,"%s\n",topline);
  fprintf(testStream,"\n");
  fprintf(testStream,"\tnx         = %6d\n",nx);
  fprintf(testStream,"\tny         = %6d\n",ny);
  //  fprintf(testStream,"\tncomplex   = %li\n",ncomplex);
  fprintf(testStream,"\tblank      = %e\n", blank);
  fprintf(testStream,"\tbscale     = %e\n", bscale);
  fprintf(testStream,"\tbzero      = %e\n", bzero);
  char coordStr[81];
  fprintf(testStream,"\tcrval1     = %s\n",deg2hour_str(coordStr,crval1,true));
  fprintf(testStream,"\tcrval2     = %s\n",deg2deg_str(coordStr,crval2,true));
  fprintf(testStream,"\tcdelt1     = %e\n", cdelt1 );
  fprintf(testStream,"\tcdelt2     = %e\n", cdelt2);
  fprintf(testStream,"\tcrpix1     = %e\n", crpix1 );
  fprintf(testStream,"\tcrpix2     = %e\n", crpix2 );
  fprintf(testStream,"\tcrota2     = %e\n", crota2 );
  fprintf(testStream,"\tpc001001   = %f\n", pc001001 );
  fprintf(testStream,"\tpc001002   = %f\n", pc001002 );
  fprintf(testStream,"\tpc002002   = %f\n", pc002002 );
  fprintf(testStream,"\tpc002001   = %f\n", pc002001 );
  fprintf(testStream,"\tctype1     = %s\n", ctype1 );
  fprintf(testStream,"\tctype2     = %s\n", ctype2 );
  fprintf(testStream,"\tequinox    = %.1f\n", equinox );
  fprintf(testStream,"\tlongpole   = %.1f\n", longpole );
  fprintf(testStream,"\tprojection = %d\n", projectiontype );
  fprintf(testStream,"\timgbin     = %f\n", imgbin );

  if( strncmp(ctype1,"RA",2)== 0 && strncmp(ctype2,"DEC",3)== 0)
    fprintf(testStream,"\tradecsys   = %s\n", radecsys );

  fprintf(testStream,"\tminimum    = %.5e\n", minimum());
  fprintf(testStream,"\tmaximum    = %.5e\n", maximum());
  fprintf(testStream,"\ttotal      = %.6e\n", total());
  //fprintf(testStream,"\t%s\n","The End");


  jvtools::BasicData::print(testStream);

  //  fprintf(testStream,"%s\n",bottomline);

  return SUCCESS;

}// end testPrint()



/****************************************************************************/
//
// FUNCTION    :  readfits()
// DESCRIPTION :  Read image fits file.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//    filename          I
//
//  RETURNS    :  SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::readfits(const char filename[], int imnr, int hdunr )
{
  return read(filename, imnr, hdunr);
}
Status Image::read(const char filename[], int imnr, int hdunr )
{

  setFunctionID("read[image]");
  
  // some variables for using fitsio routines 
  CfitsioClass fitsFile;
  fitsfile *fptr;
  char comment[FLEN_COMMENT], key_string[FLEN_VALUE];
  int status= 0, anynull, hdutype;
  long naxis1=0, naxis2=0, naxis3=0, group, dim1, firstelem, nelements;
  float fimgbin, s = 1.0, todeg = 180.0/M_PI;

  int i=0;

  //  Open the fits file.
  fitsFile.open( filename);
  fptr = fitsFile.fptr;


  if( fitsFile.statusReturn() != 0 || imnr < 1 || hdunr < 1){
    JVMessages::warning("unable to open ", filename);
    return JVMessages::error("incorrect input parameters!", status);
  }//if
  fits_read_key_str(fptr, "CREATOR" , creator, comment, &status);
  status=0;



  
  if( hdunr != 1 ){
    fits_movabs_hdu(fptr, hdunr, &hdutype, &status);
    fits_read_keyword( fptr, "EXTNAME", key_string, comment, &status );
    message("Using HDU number = ",hdunr);
    message("XTENSION name = ",key_string);
    if( status != 0 )
      return error("unable to move to HDU", status);
  }//if


  if( fits_read_key_lng(fptr,"NAXIS3", &naxis3, comment, &status)!= 0){
    status = 0;
    naxis3 = 1; // no data cube
  }//if
  fits_read_key_lng(fptr,"NAXIS1", &naxis1, comment, &status);
  fits_read_key_lng(fptr,"NAXIS2", &naxis2, comment, &status);

  if( status == 0 && !(imnr > 1 && imnr > naxis3 )){
    // allocate mem for image
    if( allocate(naxis1, naxis2,  blank) == FAILURE )
      return error("Memory allocation error!");
  }
  else{
    if( hdunr == 1 ){
      warning("error reading image data, trying 2nd HDU",status);
      return readfits(filename, imnr, 2 );
    }
    return error("error reading fits file!",status);
  }//else



      
  readStdKeywords( &fitsFile);

      
  fits_read_key_flt(fptr,"IMGBIN", &fimgbin, comment, &status);
  if( status == 0)
    imgbin = (int)(fimgbin+0.5);
  status = 0;
  fits_read_key(fptr, TSTRING, "CTYPE1" ,(void *)&ctype1[0], comment, &status);
  fits_read_key(fptr, TSTRING, "CTYPE2" ,(void *)&ctype2[0], comment, &status);

  status=0;
  if( ! (instrumentID == FERMI_LAT) ){
    fits_read_key_dbl(fptr,"CRVAL1", &crval1, comment, &status);
    fits_read_key_dbl(fptr,"CRVAL2", &crval2, comment, &status);
    fits_read_key_dbl(fptr,"CRPIX1", &crpix1, comment, &status);
    fits_read_key_dbl(fptr,"CRPIX2", &crpix2, comment, &status);
    status = 0;
    fits_read_key_dbl(fptr,"CDELT1", &cdelt1, comment, &status);
    fits_read_key_dbl(fptr,"CDELT2", &cdelt2, comment, &status);
    if(status != 0 && instrumentID != INTEGRAL_IBIS ){
      warning("unable to get physical scale information.");
      status = 0; // non-fatal error
    }// if
    fits_read_key(fptr,TINT,"CHANMIN", &chanMin, comment, &status);
    fits_read_key(fptr,TINT,"CHANMAX", &chanMax, comment, &status);

    if( status != 0 ){
      message("no channel information available");
      status=0;
    }
  }else{
    crval1=0.0;
    crval2=0.0;
    crpix1=0.0;
    crpix2=0.0;
    cdelt1=1.0;
    cdelt2=1.0;
  }//else
  //  printf("cdelt1/2 %f %f\n", cdelt1, cdelt2);
  if( 0 && instrumentID == INTEGRAL_IBIS && status !=0 ){
    //    message("IBIS keywords");
    status = 0;
    fits_read_key_dbl(fptr,"CD1_1", &cdelt1, comment, &status);
    fits_read_key_dbl(fptr,"CD2_2", &cdelt2, comment, &status);
    fits_read_key_log(fptr,"CHANMIN", &chanMin, comment, &status);
    fits_read_key_log(fptr,"CHANMAX", &chanMax, comment, &status);
    //    message("crval1 = ", crval1);
    //    message("crval2 = ", crval2);
    //    message("cdelt1 = ", cdelt1);
    //    message("cdelt1 = ", cdelt2);
  }

  status = 0;
  crota2 = 0.0;
  fits_read_key_dbl(fptr,"CD1_1", &pc001001, comment, &status);
  if( status == 0 ){
    //    fits_read_key_dbl(fptr,"CD1_1", &pc001001, comment, &status);
    fits_read_key_dbl(fptr,"CD2_1", &pc002001, comment, &status);
    fits_read_key_dbl(fptr,"CD1_2", &pc001002, comment, &status);
    fits_read_key_dbl(fptr,"CD2_2", &pc002002, comment, &status);
    cdelt1=pc001001;
    cdelt2=pc002002;
    //    crota2=
    //    cdelt1=1.0;
    //    cdelt2=1.0;
    message("Found CDx_x keywords");
    message("Use cdelt1 = ", cdelt1);
    message("Use cdelt2 = ", cdelt2);
    warning("This may cause errors!!!");
  }else{
    status = 0;
    fits_read_key_dbl(fptr,"PC001001", &pc001001, comment, &status);
    fits_read_key_dbl(fptr,"PC002002", &pc002002, comment, &status);
    fits_read_key_dbl(fptr,"PC002001", &pc002001, comment, &status);
    fits_read_key_dbl(fptr,"PC001002", &pc001002, comment, &status);
  }

  if( status != 0){
    status=0;
    warning("No rotation matrix found, trying CROTA2");
    fits_read_key_dbl(fptr,"CROTA2", &crota2, comment, &status);
    if( status == 0 ){
      warning("CROTA2 is used, this is deprecated");
      if( cdelt1 != 0.0 && cdelt2 != 0.0 ){
	s = cdelt2/cdelt1;
      }//if
      pc001001 =  cos(crota2/todeg);
      pc001002 = -sin(crota2/todeg) * s;
      pc002001 = sin(crota2/todeg) / s;
      pc002002 =  cos(crota2/todeg);
    }

  }
#if 1
  if( status != 0 ){
    status=0;
    warning("unable to read cdelt keywords");
    message("cdelt1 = ", cdelt1);
    message("cdelt2 = ", cdelt2);
    message("trying CDx_y keywords");
    fits_read_key_dbl(fptr,"CD1_1", &pc001001, comment, &status);
    fits_read_key_dbl(fptr,"CD2_2", &pc002002, comment, &status);
    fits_read_key_dbl(fptr,"CD2_1", &pc002001, comment, &status);
    fits_read_key_dbl(fptr,"CD1_2", &pc001002, comment, &status);
    //    cdelt1 = cdelt2 = 1.0;
    //    message("pc001001 = ", pc001001);
  }
#endif

  if( instrumentID == INTEGRAL_IBIS ){
    //    message("IBIS keywords");
    status = 0;
    //    fits_read_key_dbl(fptr,"CD1_1", &cdelt1, comment, &status);
    //    fits_read_key_dbl(fptr,"CD2_2", &cdelt2, comment, &status);
    fits_read_key_dbl(fptr,"CD1_1", &pc001001, comment, &status);
    fits_read_key_dbl(fptr,"CD2_2", &pc002002, comment, &status);
    fits_read_key_dbl(fptr,"CD2_1", &pc002001, comment, &status);
    fits_read_key_dbl(fptr,"CD1_2", &pc001002, comment, &status);
    cdelt1 = 1.0;
    cdelt2 = 1.0;
    //    message("crval1 = ", crval1);
    //    message("crval2 = ", crval2);
    //    message("cdelt1 = ", cdelt1);
    //    message("cdelt1 = ", cdelt2);
  }

  fits_read_key_dbl(fptr,"DRPIX1", &drpix1, comment, &status);
  fits_read_key_dbl(fptr,"DRPIX2", &drpix2, comment, &status);
  fits_read_key_dbl(fptr,"DRDELT1", &drdelt1, comment, &status);
  fits_read_key_dbl(fptr,"DRDELT2", &drdelt2, comment, &status);
  status=0;

  status = 0;
  fits_read_key_flt(fptr,"BSCALE", &bscale, comment, &status);
  fits_read_key_flt(fptr,"BZERO",&bzero, comment, &status);
  status = 0;


  fits_read_key_dbl(fptr,"OPTICX", &opticx, comment, &status);
  fits_read_key_dbl(fptr,"OPTICY", &opticy, comment, &status);
  if( status != 0 ){
    status = 0;
    fits_read_key_dbl(fptr,"XCEN", &opticx, comment, &status);
    fits_read_key_dbl(fptr,"YCEN", &opticy, comment, &status);
    status = 0;
  }//if

  if( instrumentID == XMM_RGS1 || instrumentID == XMM_RGS2 ){
    fits_read_key(fptr,TINT,"RFLORDER", &rflorder, comment, &status);
    if( status != 0 )
      fits_read_key(fptr,TINT,"GR_ORDER", &rflorder, comment, &status);
    fits_read_key(fptr,TFLOAT,"ALPHANOM", &alphanom, comment, &status);
    fits_read_key(fptr,TDOUBLE,"BETA_WID", &beta_wid, comment, &status);
    fits_read_key(fptr,TDOUBLE,"BETA_REF", &beta_ref, comment, &status);
    fits_read_key(fptr,TDOUBLE,"XDSP_WID", &xdsp_wid, comment, &status);
    fits_read_key(fptr,TDOUBLE,"XDSP_REF", &xdsp_ref, comment, &status);
    status = 0;
  }//if

  if( instrumentID == INTEGRAL_IBIS ){
    emin = fitsFile.readKeyFlt("E_MIN");
    emax = fitsFile.readKeyFlt("E_MAX");
    exposure = fitsFile.readKeyFlt("ONTIME");
  }else if( instrumentID == SPITZER_IRAC ){
    message("Spitzer: use EXPTIME keyword");
    exposure = fitsFile.readKeyFlt("EXPTIME");
    message("exposure = ", exposure);
  }//if
  group = 1;
  dim1  = naxis1;

  if( imnr == 1 ){
    fits_read_2d_flt( fptr, group, blank, dim1, naxis1, naxis2, &pixel[0], 
		      &anynull,  &status);
  }
  else{
    message("reading image nr. ",imnr);
    nelements = nx*ny;  // =naxis1*naxis2
    firstelem = 1 + (imnr-1) * nelements;
    fits_read_img_flt( fptr, group, firstelem, nelements, blank, &pixel[0],
 		      &anynull,  &status);
  }// if else

#if 0
  // BZERO and BSCALE automatically applied by fitsio!!
  if( bzero != 1.0 && bzero != 0.0 ){
    message("Applying scaling constants (BZERO & BSCALE)");
    nelements = nx*ny;
    /*
    for( int i = 0; i < nelements; i++){
      pixel[i] = bscale * pixel[i] + bzero;
      } */
    if( blank != 0.0 )
      blank = bscale * blank + bzero;
  }
#endif
  bscale = 1.0;
  bzero  = 0.0;
  if( instrumentID == SPITZER_IRAC || instrumentID == SPITZER_MIPS ){
    message("working out some patches for SPITZER word coordinates");
    status = 0;
    fits_read_key_dbl(fptr,"CD1_1", &pc001001, comment, &status);
    fits_read_key_dbl(fptr,"CD2_2", &pc002002, comment, &status);
    fits_read_key_dbl(fptr,"CD2_1", &pc002001, comment, &status);
    fits_read_key_dbl(fptr,"CD1_2", &pc001002, comment, &status);
    cdelt1 = 1.0;
    cdelt2 = 1.0;
  }
  if( exposure<= 0.0 ){
    status=0;
    warning("exposure = ",exposure);
    message("Will try reading primary header");
    fitsFile.moveTo(1);
    exposure=fitsFile.readKeyDbl("EXPTIME");
    warning("exposure = ",exposure);
    if( strncasecmp(telescope,"UNKNOWN",7)==0 ){
      fitsFile.readKey("TELESCOP", telescope);
      fitsFile.readKey("INSTRUME", instrument);
      set_telescope(telescope);
      set_instrument(instrument);
    }
  }
  

  if( status != 0 ){
    deallocate();
    return error("error reading fits file!");
  }

  //  fits_close_file(fptr, &status);


  setCoordinateType();
  setprojection();

  
  return success();
} // readfits()



/*****************************************************************************/
//
// FUNCTION    :  writefits()
// DESCRIPTION :  Writes the image to a FITS-file.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  filename            I                       file name
//
//  RETURNS    :  SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::writefits(const char filename[], bool append )
{
  return save(filename, append);
}
Status Image::save(const char filename[], bool append ) 
{
  setFunctionID("save");

  if( nx == 0 || ny == 0)
    return error("no image to be written");


  // some variables for using fitsio routines 
  fitsfile *fptr;
  int status= 0, bitpix;
  long naxis, naxes[2], naxis1, naxis2, dim1, group;
  char comment[FLEN_COMMENT]=" ";

  bitpix = -32;
  naxis  = 2;
  naxis1 = nx;
  naxis2 = ny;
  naxes[0]= naxis1;
  naxes[1]= naxis2;
  dim1   = nx;
  group  = 1;

  if( append ){
    fits_open_file( &fptr, filename , READWRITE, &status );
    fits_insert_img( fptr,  bitpix, naxis, naxes, &status);
    fits_write_key( fptr, TSTRING,"EXTNAME",(void *)"image"," ", &status);
  }
  else{
    fits_create_file( &fptr, filename , &status );
    fits_write_imghdr( fptr,  bitpix, naxis, naxes, &status);
  }

  if( status != 0 ){
    fits_close_file(fptr, &status);
    message("fits image name=",filename);
    return error("Error creating/appending fits image!",status);
  }//if


  writeStdKeywords( fptr, true);

  fits_write_key( fptr, TSTRING,"CTYPE1",(void *)&ctype1[0],"projection ", &status);
  fits_write_key( fptr,TSTRING,"CTYPE2",(void *)&ctype2[0], "projection ", &status);


  fits_write_key( fptr, TDOUBLE,"CRPIX1", (void *)&crpix1, 
		  "X axis reference pixel", &status);
  fits_write_key( fptr, TDOUBLE,"CRPIX2", (void *)&crpix2, 
		  "Y axis reference pixel", &status);
  fits_write_key( fptr, TDOUBLE, "CRVAL1", (void *)&crval1, 
		  "coord of X ref pixel", &status);
  fits_write_key( fptr, TDOUBLE, "CRVAL2", (void *)&crval2, 
		  "coord of Y ref pixel", &status);
  if( 0 && strncasecmp(ctype1,"DETX",4) == 0 && strncasecmp(ctype2,"DETY",4) == 0){
    fits_write_key( fptr, TINT, "CDELT1", (void *)&imgbin, 
		    "X axis incr., in detxy pixels", &status);
    fits_write_key( fptr, TINT, "CDELT2", (void *)&imgbin, 
    		    "Y axis incr. in detxy pixels", &status);
  }
  else if( cdelt1==1.0 && cdelt2==1.0 ){ // && pc001001 != 1.0 ){
    fits_write_key_flt( fptr, "CD1_1", (float)pc001001,  6,
			"Coord. Descr. Matrix", &status);
    fits_write_key_flt( fptr, "CD1_2", (float)pc001002,  6,
			"Coord. Descr. Matrix", &status);
    fits_write_key_flt( fptr, "CD2_1", (float)pc002001,  6,
			"Coord. Descr. Matrix", &status);
    fits_write_key_flt( fptr, "CD2_2", (float)pc002002,  6,
			"Coord. Descr. Matrix", &status);
  }
  else{
    fits_write_key( fptr, TDOUBLE, "CDELT1", (void *)&cdelt1, 
		    "X axis increment", &status);
    fits_write_key( fptr, TDOUBLE, "CDELT2", (void *)&cdelt2, 
		    "Y axis increment", &status);
    //    fits_write_key( fptr, TDOUBLE, "CROTA2", (void *)&crota2, comment, 
    //		    &status);
    fits_write_key_flt( fptr, "PC001001", (float)pc001001,  6,
			"Coord. Descr. Matrix", &status);
    fits_write_key_flt( fptr, "PC001002", (float)pc001002,  6,
			"Coord. Descr. Matrix", &status);
    fits_write_key_flt( fptr, "PC002001", (float)pc002001,  6,
			"Coord. Descr. Matrix", &status);
    fits_write_key_flt( fptr, "PC002002", (float)pc002002,  6,
			"Coord. Descr. Matrix", &status);
  }


  //  fits_write_key( fptr, TINT, "IMGBIN", (void *)&imgbin, 
  //		  "image rebinning factor", &status);

  fits_write_key( fptr, TFLOAT, "IMGBIN", (void *)&imgbin, 
		  "image rebinning factor", &status);

  if( drpix1 != 0.0 || drpix2 != 0.0 ||  drdelt1 != 0.0 || drdelt2 != 0.0  ){
    fits_write_key( fptr, TDOUBLE, "DRPIX1", (void *)&drpix1, 
		    "X axis detector reference pixel", &status);
    fits_write_key( fptr, TDOUBLE, "DRPIX2", (void *)&drpix2, 
		    "Y axis detector reference pixel", &status);
    fits_write_key( fptr, TDOUBLE, "DRDELT1", (void *)&drdelt1, 
		    "X axis detector increment", &status);
    fits_write_key( fptr, TDOUBLE, "DRDELT2", (void *)&drdelt2, 
		    "Y axis detector increment", &status);
  }//if


  fits_write_key(fptr,TDOUBLE, "OPTICX",(void *)&opticx, comment, &status);
  fits_write_key(fptr,TDOUBLE, "OPTICY",(void *)&opticy, comment, &status);

  if( instrumentID == XMM_RGS1 || instrumentID == XMM_RGS2 ){
    fits_write_key(fptr,TINT,"RFLORDER", &rflorder,comment, &status);
    fits_write_key(fptr,TFLOAT,"ALPHANOM", &alphanom,comment, &status);
    status = 0;
  }//if

#if 0
  if( get_instrument() == XMM_MOS1 ||  get_instrument() == XMM_MOS2){
    char dskeyword[FLEN_KEYWORD],dsvalue[FLEN_VALUE];;
    fits_write_key( fptr, TSTRING,"DSTYP1",(void *)"TIME"," ", &status);  
    fits_write_key( fptr, TSTRING,"DSUNI1",(void *)"s"," ", &status);  
    fits_write_key( fptr, TSTRING,"DSVAL1",(void *)"TABLE"," ", &status); 
    fits_write_key( fptr, TSTRING,"DSREF1",(void *)":STDGTI"," ", &status);  
    fits_write_key( fptr, TSTRING,"DSTYP5",(void *)"CCDNR"," ", &status);  
    fits_write_key( fptr, TSTRING,"DSVAL6",(void *)"TABLE"," ", &status);  
    fits_write_key( fptr, TSTRING,"DSREF6",(void *)":STDGTI01"," ", &status);  
    fits_write_key( fptr, TSTRING,"DSVAL5",(void *)"1"," ", &status);  
    for(int i=2; i <= 7; i++){
      sprintf(dskeyword,"%iDSREF6",i);
      if( i<10){
	sprintf(dsvalue,":STDGTI0%i",i);
	fits_write_key( fptr, TSTRING,dskeyword,(void *)dsvalue," ", &status);
      }
      sprintf(dskeyword,"%iDSVAL5",i);
      sprintf(dsvalue,"%i",i);
      fits_write_key( fptr, TSTRING,dskeyword,(void *)dsvalue," ", &status);
    }//for
  }
#endif


  if( status != 0 ){
    warning("error writing some keywords");
    status=0;
  }

  fits_write_2d_flt( fptr, group, dim1, naxis1, naxis2, &pixel[0], &status);
  fits_close_file(fptr, &status);

  return success();

}// end writefits()

#if 0
/****************************************************************************/
//
// FUNCTION    :  writefits_complex()
// DESCRIPTION :  Writes the complex data to a FITS-file.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  filename            I                       file name
//
//  RETURNS    :  SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::writefits_complex( const char filename[] ){
  return saveComplex( filename);
}
Status Image::saveComplex( const char fileName[] )
{

  setFunctionID("saveComplex");
  warning("not supported");
  
#if 0
  if( ncomplex== 0 || ftransformed==FALSE || 
      ncomplex != (unsigned)((nx/2+1)*ny) )
    return error("no complex data to be written");

  
  // some variables for using fitsio routines 
  fitsfile *fptr;
  int status= 0, bitpix;
  long naxis, naxes[2], naxis1, naxis2, dim1, group;
  char comment[FLEN_COMMENT];

  bitpix = -32;
  naxis  = 2;
  naxis1 = 2*(nx/2+1);
  naxis2 = ny;
  naxes[0]= naxis1;
  naxes[1]= naxis2;
  dim1   = 2*(nx/2+1);
  group  = 1;
  fits_create_file( &fptr, fileName , &status );
  fits_write_imghdr( fptr,  bitpix, naxis, naxes, &status);
  if( status != 0 ){
    warning("writefits_complex(): Error writing fits image!");
    return FAILURE;
  }//if

  strcpy(comment,"Not applicable: FFT image");
  fits_write_key( fptr, TSTRING,"OBJECT",(void *)&object[0], comment, &status);
  fits_write_key( fptr, TSTRING,"INSTRUME",(void *)&instrument[0], 
		  comment, &status);
  fits_write_key( fptr, TSTRING,"TELESCOP",(void *)&telescope[0], 
		  comment, &status);
  fits_write_key( fptr, TFLOAT, "BSCALE",(void *)&bscale, comment, &status);
  fits_write_key( fptr, TFLOAT, "BZERO", (void *)&bzero, comment, &status);
  fits_write_key( fptr, TFLOAT, "RA_NOM", (void *)&ra_nom , comment, &status);
  fits_write_key( fptr, TFLOAT, "DEC_NOM",(void *)&dec_nom, comment, &status);
  fits_write_key( fptr, TSTRING,"CTYPE1",(void *)&ctype1[0],"projection ", &status);
  fits_write_key( fptr,TSTRING,"CTYPE2",(void *)&ctype2[0], "projection ", &status);
  fits_write_key_fixflt(fptr, "EQUINOX", equinox, 1, 
			"Equinox for coordinate system", &status);
  if( strncmp(ctype1,"RA",2)== 0 && strncmp(ctype2,"DEC",3)== 0){
    fits_write_key( fptr, TSTRING,"RADECSYS",(void *)&radecsys[0],"Frame of reference", &status);
  }//if
  fits_write_key( fptr, TDOUBLE, "CRPIX1", (void *)&crpix1, 
		  "X axis reference pixel", &status);
  fits_write_key( fptr, TDOUBLE, "CRPIX2", (void *)&crpix2, 
		  "Y axis reference pixel", &status);
  fits_write_key( fptr, TDOUBLE, "CRVAL1", (void *)&crval1, 
		  "coord of X ref pixel", &status);
  fits_write_key( fptr, TDOUBLE, "CRVAL2", (void *)&crval2, 
		  "coord of Y ref pixel", &status);
  fits_write_key( fptr, TDOUBLE, "CDELT1", (void *)&cdelt1, 
		  "X axis increment", &status);
  fits_write_key( fptr, TDOUBLE, "CDELT2", (void *)&cdelt2, 
		  "Y axis increment", &status);
  fits_write_key( fptr, TDOUBLE, "CROTA2", (void *)&crota2, comment, &status);
  fits_write_key_flt( fptr, "PC001001", (float)pc001001,  6,
		      "Coord. Descr. Matrix", &status);
  fits_write_key_flt( fptr, "PC001002", (float)pc001002,  6,
		      "Coord. Descr. Matrix", &status);
  fits_write_key_flt( fptr, "PC002001", (float)pc002001,  6,
		      "Coord. Descr. Matrix", &status);
  fits_write_key_flt( fptr, "PC002002", (float)pc002002,  6,
		      "Coord. Descr. Matrix", &status);


  if( drpix1 != 0.0 || drpix2 != 0.0 ||  drdelt1 != 0.0 || drdelt2 != 0.0  ){
    fits_write_key( fptr, TINT, "IMGBIN", (void *)&imgbin, 
		    "image rebinning factor", &status);
    fits_write_key( fptr, TDOUBLE, "DRPIX1", (void *)&drpix1, 
		    "X axis detector reference pixel", &status);
    fits_write_key( fptr, TDOUBLE, "DRPIX2", (void *)&drpix2, 
		    "Y axis detector reference pixel", &status);
    fits_write_key( fptr, TDOUBLE, "DRDELT1", (void *)&drdelt1, 
		    "X axis detector increment", &status);
    fits_write_key( fptr, TDOUBLE, "DRDELT2", (void *)&drdelt2, 
		    "Y axis detector increment", &status);
  }//if
  fits_write_key( fptr,TDOUBLE, "EXPOSURE",(void *)&exposure,comment, &status);
  fits_write_key( fptr,TFLOAT, "MJD-OBS",(void *)&mjd_obs, comment, &status);
  fits_write_key( fptr,TDOUBLE, "MJDMEAN",(void *)&mjdmean, comment, &status);
  fits_write_key( fptr,TSTRING,"DATE-OBS",(void *)&obsdate[0],comment,&status);
  fits_write_key( fptr,TSTRING,"DATE-END",(void *)&enddate[0],comment,&status);
  if( blank != 0.0 ){
    fits_write_key( fptr, TFLOAT, "BLANK" , (void *)&blank , comment, &status);
  }//if

  if( status != 0 ){
    warning("writefits_complex(): error writing some keywords.");
    status=0;
  }
  fits_write_key( fptr,TSTRING,"CREATOR",(void *)&creator[0], comment,&status);
  history.add2FitsFile( fptr);

  
  if( status != 0 ){
    warning("writefits_complex(): error writing history", status);
    status=0;
  }

   fits_write_2d_flt( fptr, group, dim1, naxis1, naxis2,(float *)&cpixel[0], &status);
  fits_close_file(fptr, &status);
#endif
  
  return SUCCESS;

}// end writefits_complex()
#endif

/****************************************************************************/
//
// FUNCTION    : extract()
// DESCRIPTION : Calls read events to read the event list, select the 
//               proper photons and puts them in the 2D array.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::extract( const ExtractionParameters &selection, Image *variance)
{

  setFunctionID("extract");

  //  if( selection.imageSize < 2 ||  selection.binFactor < 1  )
  if( selection.imageSize < 2 ||  selection.binFactorF < 0.01  )
    return error("invalid input parameters");
  
  if( selection.chanMin >= selection.chanMax || selection.bufferSize <= 0) 
    return error("invalid input parameters!");

  if( selection.applyPSF ){
    toggleApplyPSF();
  }
    

  setChatLevel(CHATTY);
  if( selection.usePolarisation  ){
    message("polarisation mode =",selection.polarisation);
    warning("Polarisation extraction is requested. This works only with XIPE/IXPE/eXTP data!");

    if( variance == NULL ){
      return error("The variance image cannot be a zero pointer");
    }
  }//if selection.usePolarisation

  
  char process_string[]= "Processed events = ";
  double xbinfactor= selection.binFactorF;
  double ybinfactor= selection.binFactorF;

  int i, j, xsize, ysize, xchipsize, ychipsize, nchip=1;
  int chMin = selection.chanMin, chMax = selection.chanMax;
  int detNumber = selection.detNumber;
  int percentage = 0, oldpercentage=0;
  int *chan_ptr, *pat_ptr, *ccd_ptr;
  long *fl_ptr;
  unsigned long n_rejected=0, n_accepted=0, n_elements=0, k;
  float *x_ptr, *y_ptr, *dx_ptr, *dy_ptr, pixval=1.0;
  float *polQ_ptr=0, *polU_ptr=0, *mod_ptr=0, *polPhi_ptr=0;
  float newcrpix1 = selection.imageSize/2.0 + 0.5;
  float newcrpix2 = selection.imageSize/2.0 + 0.5;
  float rgs_xdspfactor = -RAD2DEG * 3600.0; // set 1 pixel is 1 arcsec
  double *time_ptr, averagetime=0.0;
  double dx=selection.dx, dy=selection.dy;
  double xmask, ymask, alpha , delta, skyX, skyY;
  float polWeight=1.0;
  bool usechipcoordinates = false, makechipmap = false;
  bool recenter = selection.useradec;
  bool rgsUnbinned = false;
  //  Instrument instr = UNDEF;
  Image newcoord;
  Image weightNormalisation;
  //  Polarisation ixpe;
  Coordinate_type maskxytype= UNDEF_XY, ctype = selection.wcsType;
  IO_status status = IO_SUCCESS;


  if( selection.mask.exists() == false )
    message("no mask defined.");
  else
    maskxytype = selection.mask.getxytype();
  message("mask coordinate type = ",(int)maskxytype);
  if( ctype != SKY_XY && recenter)
    recenter = false;

  if( nbadpix > 0 || ctype == RAW_XY || maskxytype == RAW_XY || detNumber >= 0)
    usechipcoordinates = true;

  if(  ctype  == RAW_XY){
    message("ccd number ",detNumber);
    if( dx != 0.0 || dy != 0.0 )
      warning("parameters dx/dy are ignored in CCD mode");
    makechipmap = true;
    ctype = SKY_XY;
    //    xbinfactor = ybinfactor = selection.binFactor = 1;
    xbinfactor = ybinfactor = 1;
    warning("for chip mapping binfactor= 1 and size ignored");
  }
  else if( ctype == XMM_RGS_XDSP_UNBINNED){
    rgsUnbinned = true;
    ctype = XMM_RGS_XDSP;
  }//if
  if( maskxytype == DET_XY && ctype != DET_XY)
    return error("mask and events coordinate types are incompatible!");


  setChatLevel(JVMessages::CHATTY);

  if( nGTI == 0 ){

    message("Obtaining Good Time Intervals");

    if( strncmp(selection.gtiFileName.c_str(), "-",
		strlen(selection.gtiFileName.c_str())) == 0 ){

      if( readGTI( selection.eventFileName.c_str(), selection.detNumber ) 
	  == FAILURE ){
	return error("error reading GTI extensions");
      }
    }else{
      if( readGTI( selection.gtiFileName.c_str(), selection.detNumber ) 
	  == FAILURE)
	return error("error reading GTI file");
    }
    message("Obtained GTIs.");
  }
  else
    warning("using GTIs that are already present");

  
  if( selection.tstart > 0.0 )
    excludeInterval( start_time, start_time+selection.tstart);
  if( start_time + selection.tstop < stop_time )
    excludeInterval( start_time + selection.tstop, stop_time );




  //  mask.setChatty( false );
  for( unsigned long m = 0; status == IO_SUCCESS ; m++ ){

    setChatLevel(JVMessages::CHATTY);


    
    status = readEvents( selection.eventFileName.c_str(), 
			 &n_elements, selection.channelType.c_str(), 
			 ctype, selection.bufferSize, 
			 usechipcoordinates );



    if( status == IO_ERROR)
      return error("I/O error");

    if( m == 0  ){
      message("Number of badpixels = ",nbadpix);
      get_chipsize(instrumentID, &xchipsize, &ychipsize, &nchip);

      newcoord.copyKeywords( *this);
      newcoord.crpix1 = newcrpix1;
      newcoord.crpix2 = newcrpix2;
      newcoord.cdelt1 *= xbinfactor;
      newcoord.cdelt2 *= ybinfactor;
      if( selection.usePolarisation ){
	variance->copyKeywords( newcoord);
      }

      if( !makechipmap ){

	message("Coordinate type = ", ctype);
	xsize = ysize = selection.imageSize;
	nchip = 1;

	if( recenter || instrumentID == FERMI_LAT){
	  newcoord.crval1 = selection.ra0;
	  newcoord.crval2 = selection.dec0;

	  if( instrumentID == FERMI_LAT){
	    newcoord.cdelt1 = -0.2;
	    newcoord.cdelt2 = 0.2;
	    strncpy(newcoord.ctype1,"RA---TAN",8);
	    strncpy(newcoord.ctype2,"DEC--TAN",8);
	    newcoord.setprojection();
	    crval1=0.0;
	    crval2=0.0;
	    crpix1=1;
	    crpix2=1;
	    cdelt1=1.0;
	    cdelt2=1.0;
	  }
	}//

	if( instrumentID != XMM_RGS1 && 
	    instrumentID != XMM_RGS2  && 
	    ctype !=  LETGS_LAMBDA_TGD	    &&
	    cdelt1*cdelt2 != 0.0 ){
	  newcrpix1 -= dx/(cdelt1*3600.0*xbinfactor);
	  newcrpix2 -= dy/(cdelt2*3600.0*ybinfactor);
	}else if(ctype == LETGS_LAMBDA_TGD ){
	  warning("LETGS lambda/cross dispersion: under construction");
	  message("cdelt2 = ", cdelt2);
	  message("crpix2 = ", crpix2);
	  message("crval2 = ", crval2);
	  newcrpix1 = 1;
	  crpix1 = 0;
       	  crpix2 = ny/2;
	  cdelt1 = 1.0;
	  cdelt2 = 1.0;
	  crval1  = 0.0;
	  crval2  = 0.0;
	  xbinfactor = 0.4 * selection.binFactorF;
	  ybinfactor = 0.00015 * selection.binFactorF;
	}else if( xdsp_wid >=0.0 && beta_wid >=0.0){
	  warning("using RGS data settings");
	  if( ctype == XMM_RGS_XDSP && !rgsUnbinned ){
	    ybinfactor = 1;
	    cdelt1 = beta_wid * RAD2DEG;
	    cdelt2 *= RAD2DEG; //cdelt in degrees
	    crval2 *= RAD2DEG;
	    ysize  = (int)(6/60.0/cdelt2/fabs(ybinfactor) + 0.5);
	    //	    newcrpix2  =  ysize/2; 
	    newcrpix2  =  crpix2; 
	    //	    newcrval2  = crval2 + newcrpix2 *;
	    message("RGS: use a XDSP width of 5.3 arcmin, ysize= ", ysize);
	  }else if( ctype == XMM_RGS_XDSP && rgsUnbinned ){
	    //	    ybinfactor = selection.binFactor;
	    ybinfactor = selection.binFactorF;
	    cdelt2  = 1.0/3600.0;
	    cdelt1  = beta_wid*RAD2DEG;
	    xbinfactor = fabs(ybinfactor) * fabs(cdelt2/cdelt1);
	    crpix2 = 0.0;
	    ysize  = (int)(5.3/60.0/cdelt2/fabs(ybinfactor) + 0.5);
	    //	    ysize=xsize;
	    newcrpix2  =  ysize/2; 
	    message("cdelt1      = ", cdelt1);
	    message("cdelt2      = ", cdelt2);
	    message("X binfactor = ", (float)xbinfactor);
	    message("Y binfactor = ", (float)ybinfactor);
	  }
	  else if( ctype == XMM_RGS_PI ){
	    ybinfactor = 5;
	    newcrpix2  = 1;
	    ysize  = (int)((float)selection.chanMax/ybinfactor + 1.0);
	  }else if( dx != 0.0 || dy !=0.0){
	    // WARNING: THIS NEEDS TO BE CHECKED
	    //	    newcrpix1 -= dx;
	    //	    newcrpix2 -= dy;
	    //	    newcoord.crpix1 = newcrpix1;
	    //	    newcoord.crpix2 = newcrpix2;
	  }

	  //	  dx     += BETA_0 - crval1;
	  cdelt1 = beta_wid * RAD2DEG;
	  crval1 = beta_ref * RAD2DEG;
	  message("cdelt1 = ", cdelt1);
	  message("crpix1 = ", crpix1);
	  message("crval1 = ", crval1);
	  message("cdelt2 = ", cdelt2);
	  message("crpix2 = ", crpix2);
	  message("crval2 = ", crval2);
	  newcrpix1 = crpix1 - dx/(cdelt1*3600.0*xbinfactor);
	}//else if (XMM RGS
      }//if makechipmap==FALSE
      else{
	drpix1 = drpix2 = 0;
	xsize = xchipsize;
	ysize = ychipsize;
	if( selection.detNumber >= 0 ) nchip = 1;// map all chips
	message("Number of chips to be used = ",nchip);
      }//else
      message("nx = ", nx);
      message("nx = ", ny);
      if( nx != xsize || ny != ysize || makechipmap == true){

	message("allocating image");

	if( allocate( nchip * xsize, ysize) == FAILURE){
	  read_events( CLOSE_FITS_FILE, &n_elements);
	  return error("allocation error!");
	}//if
      }
      else
	warning("using existing image");
      if( selection.usePolarisation ){
	message("Allocating variance map");
	*variance = *this;
	*variance *= 0.0;

	if( selection.weighting ){
	  weightNormalisation= *this;
	  weightNormalisation *= 0.0;
	}	
	
      }
      if( (maskxytype == DET_XY || maskxytype == RAW_XY) && 
	  strcmp(instrument,selection.mask.instrument)!= 0  )
	warning("instrument keywords events & mask don't match!");
      message("nx = ", nx);
      message("ny = ", ny);
      processtatus(process_string, 0.0, TRUE, FALSE);
    }//if( m== 0 )

    
    setChatLevel(JVMessages::SUPPRESS_ALL_MESSAGES);
    if( status != IO_ERROR ){

      // Use pointers for speed reasons
      x_ptr = &x[0];           
      y_ptr = &y[0];
      chan_ptr = &channel[0];
      pat_ptr = &pattern[0];
      fl_ptr = &flag[0];
      time_ptr = &time[0];
      ccd_ptr = &ccdnr[0];
      if( usechipcoordinates  ){
	dx_ptr = &detx[0];           
	dy_ptr = &dety[0];
      }
	
      //      setChatty( false);
      setChatLevel( JVMessages::SUPPRESS_ALL_MESSAGES );
      for( k = 0 ; k < n_elements ; k++ ){

	if( xy_type == XMM_RGS_XDSP || xy_type == XMM_RGS_PI  ){
	  *x_ptr += (float)rand() /RAND_MAX - 0.5; // randomize
	  if( xy_type == XMM_RGS_XDSP  )
	    *y_ptr *= rgs_xdspfactor;
	}
	if( maskxytype == SKY_XY || recenter )
	  getradec(  *x_ptr, *y_ptr, &alpha, &delta );

	switch( maskxytype){
	case SKY_XY:
	  selection.mask.getxy( alpha, delta, &xmask, &ymask);
	  pixval = selection.mask.get((int)(xmask + 0.5), (int)(ymask + 0.5) );
	  break;
	case DET_XY:
	  xmask = *x_ptr;
	  ymask = *y_ptr;
	  selection.mask.convert_detxy( *this, &xmask, &ymask);
	  pixval = selection.mask.get( (int)(xmask + 0.5), 
				       (int)(ymask + 0.5) );
	  break;
	case RAW_XY:
	  xmask = *dx_ptr;
	  selection.mask.convert_chipxy( *this, *ccd_ptr, xchipsize, &xmask);
	  pixval = selection.mask.get((int)(xmask + 0.5), 
				      (int)(*dy_ptr + 0.5) );
	  break;
	case XMM_RGS_XDSP:
	case XMM_RGS_PI:
	  xmask = *x_ptr;
	  if(  maskxytype == XMM_RGS_PI && getxytype() == XMM_RGS_XDSP) 
	    ymask = *chan_ptr;
	  else
	    ymask = *y_ptr;
	  selection.mask.convertRGS_XY( *this, &xmask, &ymask);
	  pixval = selection.mask.get((int)(xmask + 0.5), (int)(ymask + 0.5) );
	  break;
	default:
	  pixval = 1.0;
	}//switch


	if(  *chan_ptr >= chMin &&  *chan_ptr <= chMax && 
	     checkTime( *time_ptr, *ccd_ptr ) == true && 
	     ( selection.detNumber < 0 || detNumber == *ccd_ptr) &&
	     ( nbadpix== 0 || is_not_badpix(int(*dx_ptr+0.5), int(*dy_ptr+0.5),
	     				    *ccd_ptr) == true) &&
	     check_flags( *pat_ptr, *fl_ptr) == true &&
	     pixval > 0.5 ){
	  // -1 for FITS convention


	  if( !makechipmap  ){ 

	    if( ctype == RA_DEC ){

	      newcoord.getxy( *x_ptr, *y_ptr,  &skyX, &skyY);
	      i = (int)round( skyX + dx) -1;
	      j = (int)round( skyY + dy) -1;
	      //	      printf("%f %f %f %f %i %i\n", *x_ptr, *y_ptr, skyX, skyY, i, j);
	      //	      if(k == 30 )exit(0);
	    }else if( recenter ){
	      //	      getradec(  *x_ptr, *y_ptr, &alpha, &delta );
	      newcoord.getxy(alpha, delta, &skyX, &skyY);
	      i = (int)round( skyX + dx) -1;
	      j = (int)round( skyY + dy) -1;
	    }else{
	      i = (int)((*x_ptr - crpix1)/xbinfactor + newcrpix1 + dx + 0.5) - 1; 
	      j = (int)((*y_ptr - crpix2)/ybinfactor + newcrpix2 + dy + 0.5) - 1;
	    }
	  }
	  else if( *ccd_ptr == selection.detNumber || 
		   selection.detNumber < 0  ){
	    i = (int)( *dx_ptr/xbinfactor + 0.5) - 1; 
	    j = (int)( *dy_ptr/ybinfactor + 0.5) - 1;
	    if( selection.detNumber < 0 ) 
	      i += (int)(((*ccd_ptr) - 1)*(double)xsize/(double)xbinfactor); 
	  }
	  else
	    i = j = -1;
	  
	  if( i >= 0 && i < nx && j >= 0 && j < ny ){
	    //	    if( !selection.weighing && !selection.usePolarisation )
	    if( !selection.usePolarisation ){
	      pixel[i + j*nx] += 1.0;
	    }else{
	      if( selection.mhWeighing ){
		pixel[i + j*nx] += mexicanHat( (float)(*chan_ptr), 
					       selection.chan0, 
					       selection.sigma);
	      }else{
		pixel[i + j*nx] += gaussian( (float)(*chan_ptr), 
					       selection.chan0, 
					       selection.sigma);
	      }
	    }//else
	    averagetime += *time_ptr;
	    n_accepted++;
	  }else{
	    n_rejected++;
	  }
	}//if(  *chan_ptr >= ch_min
	else{
	    n_rejected++;
	}
	chan_ptr++;
	fl_ptr++;
	pat_ptr++;
	time_ptr++;
	x_ptr++;
	y_ptr++;
	ccd_ptr++;
	if( usechipcoordinates){
	  dx_ptr++;
	  dy_ptr++;
	}
	if( selection.usePolarisation ){
	  polPhi_ptr++;
	  polQ_ptr++;
	  polU_ptr++;
	  mod_ptr++;
	}
      }//for
    }//if
    else{
      if( ntot_events > 0 )
	processtatus(process_string,100.0*
		     (n_rejected+n_accepted)/(float)ntot_events, false, true);
      else{
	  processtatus(process_string,0.0, false, true);
      }
      return error("an error occurred reading the event list");
    }//else
    setChatLevel( JVMessages::CHATTY);
    //    if( selection.bufferSize > 1880 || m % 4 == 0)
    percentage =  (int)(100.0*(n_rejected+n_accepted)/(float)ntot_events);
    if( m % 10 == 0 ){
      //	percentage > oldpercentage || m == 0 ){
      oldpercentage = percentage;
      processtatus( process_string,
      	    100.0*(n_rejected+n_accepted)/(float)ntot_events, 
		    false, false);
    }
  }// for

  processtatus(process_string,
	       100.0*(n_rejected+n_accepted)/(float)ntot_events, 
	       FALSE, TRUE);

  message("NBADPIX = ",nbadpix);
  chanMin = chMin;
  chanMax = chMax;
  imgbin = selection.binFactor;
  // calculate optical axis in old world coordinate system
  double alpha_opt, delta_opt; // optical axis
  if( selection.detNumber < 0 && 
      cdelt1*cdelt2 != 0.0 &&opticx == 0.0 && opticy == 0.0 ){
    opticx = newcrpix1 +  dx/(cdelt1*3600.0*imgbin);
    opticy = newcrpix2 +  dy/(cdelt2*3600.0*imgbin);
  }//if

  if( ctype == SKY_XY ) getradec(opticx, opticy, &alpha_opt, &delta_opt );

  if( makechipmap == FALSE ){
    if( selection.useradec || instrumentID == FERMI_LAT ){
      crpix1 = newcoord.crpix1;
      crpix2 = newcoord.crpix2;
      crval1 = newcoord.crval1;
      crval2 = newcoord.crval2;
      cdelt1 = newcoord.cdelt1;
      cdelt2 = newcoord.cdelt2;
      printf("instr = %i\n", instrumentID);
      if( instrumentID == FERMI_LAT){
	strncpy(ctype1, newcoord.ctype1, 8);
	strncpy(ctype2, newcoord.ctype2, 8);
	printf("%s %s\n", ctype1, ctype2);
      }
    }else{
      crpix1 = newcrpix1;
      crpix2 = newcrpix2;
      cdelt1 *= xbinfactor;
      cdelt2 *= ybinfactor;
    }//else
    // crval1/2 does not change 
    if( ctype == DET_XY ){
      //    strncpy(ctype1,"DET----X",8);    strncpy(ctype2,"DET----Y",8);
      strncpy(ctype1,"DETX",8);
      strncpy(ctype2,"DETY",8);
    }//
    else if( instrumentID != XMM_RGS1 && instrumentID != XMM_RGS2 )
      getxy(alpha_opt, delta_opt, &opticx, &opticy );
  }
  else{
    crpix1 =  crpix2 = crval1 = crval2 = 1.0/imgbin;
    cdelt1 = cdelt2 = 1.0/imgbin;
    opticx = opticy = 0.0;
    strncpy(ctype1,"CHIPX",8);
    strncpy(ctype2,"CHIPY",8);
  }//if

  calcExposure();


  if( n_accepted > 0){
    averagetime /= n_accepted;
    mjdmean = averagetime/(24.0*3600.0);
    mjdmean += mjdrefi + mjdreff;
  }//if

  message("Minimum channel = ",chanMin);
  message("Maximum channel = ",chanMax);
  message("Number of accepted photons = ",n_accepted);
  message("Number of rejected photons = ",n_rejected);

  // insert history
  char historyText[HIST_LEN];
  sprintf(historyText,"Number of accepted photons = %li",n_accepted);
  puthistorytxt(historyText );
  sprintf(historyText,"Number of rejected photons = %li",n_rejected);
  puthistorytxt(historyText );
  sprintf(historyText,"Total number of photons = %li",n_events);
  puthistorytxt(historyText );
  sprintf(historyText,"Number of bad pixels       = %li",nbadpix);
  puthistorytxt(historyText );

  if( n_events > n_accepted + n_rejected ){
    n_events = n_accepted;
    warning("buffer size too small");
  }//if

  n_events = n_accepted;
  //  selection.mask.setChatty( true );
  setprojection();


  
  if( status == IO_ERROR )
    return error("an error occurred reading the event list");


  return success();
}//extract()


/*****************************************************************************/
//
// FUNCTION    : phase_image()
// DESCRIPTION : Calls read events. Extracts the image given as certain phase
//               and period.
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
Status Image::phase_image( char filename[],  char gtifile[],
				Image &mask,
				double period, unsigned int phase, 
				unsigned int nbins,
				unsigned int im_size, unsigned int binfactor,
				double tstart, double tend,
				int ch_min, int ch_max,double dx, double dy,
				Coordinate_type ctype, unsigned long b_size )
{

  if( im_size < 2 ||  binfactor < 1 || //binfactor > im_size/2 || 
      allocate( (int)im_size, (int)im_size, 0.0) == FAILURE ){
    warning(
      "phase_image( ): invalid image size/bin factor or allocation error!");
    return FAILURE;
  }// if
  if( ch_min >= ch_max || b_size <= 0 || period < 0.0 || nbins < 1 || 
      phase >= nbins ){ 
    warning("phase_image( ): invalid input parameters!");
    return FAILURE;
  }// if

  int *chan_ptr, *pat_ptr, *ccd_ptr, i, j, masknx, maskny;
  long *fl_ptr;
  unsigned int binnr;
  unsigned long n_rejected=0, n_accepted=0, n_elements=0, k, nperiod;
  int xbinfactor = binfactor, ybinfactor = binfactor;
  float *x_ptr, *y_ptr, *dx_ptr, *dy_ptr, pixval;
  float *polQ_ptr=0, *polU_ptr=0;
  float newcrpix1 = im_size/2.0 + 0.5;
  float newcrpix2 = im_size/2.0 + 0.5;
  double *time_ptr, alpha , delta, xmask, ymask;
  Boolean firstaccess=TRUE, usedetcoordinates=FALSE;
  //  Instrument instr = UNDEF;
  IO_status status = IO_SUCCESS;

  mask.getnxny( &masknx, &maskny );
  if( masknx <= 0 || maskny <= 0 ){
    return error("mask image has wrong size!");
  }//if
  if( nbadpix > 0 )
    usedetcoordinates=TRUE;

  reset_keywords();

  if( read_gti( filename ) == FAILURE ){
    warning("phase_events( ): error reading GTIs");
    return FAILURE;
  }//if
  constrain_time( tstart, tend );


  mask.setChatty( FALSE );
  while( status == IO_SUCCESS ){

    status = read_events( filename, &n_elements, "PI", ctype, b_size,
			  usedetcoordinates);
    if( firstaccess==TRUE && cdelt1 != 0.0 && cdelt2 != 0.0){
      newcrpix1 -= dx/(cdelt1*3600.0*binfactor);
      newcrpix2 -= dy/(cdelt2*3600.0*binfactor);
      firstaccess=FALSE;
    }//if

    if( status != IO_ERROR ){

      // Use pointers for speed reasons
      x_ptr = &x[0];           
      y_ptr = &y[0];
      dx_ptr = &detx[0];
      dy_ptr = &dety[0];
      ccd_ptr = &ccdnr[0];
      chan_ptr = &channel[0];
      pat_ptr = &pattern[0];
      fl_ptr = &flag[0];
      time_ptr = &time[0];
      for( k = 0 ; k < n_elements ; k++ ){

	getradec(  *x_ptr, *y_ptr, &alpha, &delta );
	mask.getxy( alpha, delta, &xmask, &ymask);
	pixval = mask.get( (int)(xmask + 0.5), (int)(ymask + 0.5) );
	nperiod = (unsigned long)( (*time_ptr - start_time)/period );
	binnr =(int)((*time_ptr - start_time - nperiod*period)/period * nbins);

	if( checkTime( *time_ptr, *ccd_ptr ) == true && 
	    //	    check_time( *time_ptr ) == true &&
	    binnr == phase &&
	    check_flags( *pat_ptr, *fl_ptr) == TRUE &&
	     (nbadpix==0 || is_not_badpix(int(*dx_ptr+0.5), int(*dy_ptr+0.5), 
					  *ccd_ptr) == TRUE) &&
      	    *chan_ptr >= ch_min &&  *chan_ptr <= ch_max && pixval > 0.5){
	  // -1 for FITS convention
	  i = (int)( (*x_ptr - crpix1)/binfactor + newcrpix1  + 0.5) - 1; 
	  j = (int)( (*y_ptr - crpix2)/binfactor + newcrpix2  + 0.5) - 1;
	  if( i >= 0 && i < nx && j >= 0 && j < ny ){
	    pixel[i + j*nx] += 1.0;
	    n_accepted++;
	  }
	  else
	    n_rejected++;
	}//if
	else
	    n_rejected++;

	if( usedetcoordinates == TRUE ){
	  ccd_ptr++;
	  dx_ptr++;
	  dy_ptr++;
	}	
	chan_ptr++;
	fl_ptr++;
	pat_ptr++;
	time_ptr++;
	x_ptr++;
	y_ptr++;


      }//for

    }//if
    else
      return errorwarning("an error occurred reading the event list");
  }// while

  imgbin = binfactor;
 // calculate optical axis in old world coordinate system
  double alpha_opt, delta_opt; // optical axis
  if( opticx == 0.0 && opticy == 0.0 ){
    opticx = crpix1;
    opticy = crpix2;
  }//if
  getradec(opticx, opticy, &alpha_opt, &delta_opt );

  crpix1 = newcrpix1;
  crpix2 = newcrpix2;
  cdelt1 *= xbinfactor;
  cdelt2 *= ybinfactor;
  calc_exposure();
  // crval1/2 does not change 

  getxy(alpha_opt, delta_opt, &opticx, &opticy );

  exposure /= nbins; //correct for small periods!!

  mask.setChatty( TRUE );

  //  printf("Accepted : %li events\n", n_accepted);
  //  printf("Rejected : %li events\n", n_rejected);
  //  printf("Read     : %li events\n", n_events);

  if( n_events > n_accepted + n_rejected ){
    n_events = n_accepted;
    warning("extract_image(): Buffer size too small, not all events were loaded!");
  }//if

  setprojection();

  // insert history
  char historyText[HIST_LEN];
  sprintf(historyText,"Number of accepted photons = %li",n_accepted);
  puthistorytxt(historyText );
  sprintf(historyText,"Number of rejected photons = %li",n_rejected);
  puthistorytxt(historyText );

  n_events = n_accepted;

  if( status == IO_ERROR )
    return errorwarning("an error occurred reading the event list!");


  if( exposure <= 0.0 ){
    warning("error in exposure?",exposure);
    calc_exposure();
    warning("recalculated exposure (s) = ",exposure);
  }//if

  return success();

}//phase_image()

/****************************************************************************/
//
// FUNCTION    : extractImage()
// DESCRIPTION : Calls read events to read the event list, select the 
//               proper photons and puts them in the 2D array.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//     filename         I                event list file name
//     gtifile          I                in case GTIs are in a seperate file
//     mask             I                image containing 0 and 1, masking
//                                       the selected region(s)
//     im_size          I                size (ignored if ccdnumber >=0)
//     binfactor        I                rebinning factor
//    ch_min,ch_max     I                minimum/maximum photon channel
//       dx, dy         I                    zero point shift in arcsec
//    ccdnumber         I                CCD number to map (-1 = ignore/all)
//    ctype             I                coordinate type to bin
//    ch_type           I                photon channel type (e.g. PI)
//    tstart, tend      I                additional constraints on time
//    b_size            I                buffer size to use for extracting ev.
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::extractImage( const char filename[], const char gtifile[],
			    Image &mask, 
			    unsigned int im_size, 
			    unsigned int binfactor,
			    int ch_min, int ch_max, 
			    double dx, double dy,
			    int ccdnumber,
			    Coordinate_type ctype, char ch_type[],
			    double tstart, double tend,
			    unsigned long b_size )
{

  setFunctionID("extractImage");
  
  if( im_size < 2 ||  binfactor < 1  )
    return error("invalid input parameters");
  
  if( ch_min >= ch_max || b_size <= 0) 
    return error("invalid input parameters!");

  char process_string[]= "Processed events = ";
  //  unsigned int xbinfactor= binfactor;
  //  unsigned int ybinfactor= binfactor;
  double xbinfactor= binfactor;
  double ybinfactor= binfactor;

  int i, j, xsize, ysize, xchipsize, ychipsize, nchip=1;
  int *chan_ptr, *pat_ptr, *ccd_ptr;
  long *fl_ptr;
  unsigned long n_rejected=0, n_accepted=0, n_elements=0, k;
  float *x_ptr, *y_ptr, *dx_ptr, *dy_ptr, pixval=1.0;
  float newcrpix1 = im_size/2.0 + 0.5;
  float newcrpix2 = im_size/2.0 + 0.5;
  float rgs_xdspfactor = -RAD2DEG * 3600.0; // set 1 pixel is 1 arcsec
  double *time_ptr, averagetime=0.0, xmask, ymask, alpha , delta;
  bool usechipcoordinates = false, makechipmap = false;
  bool rgsUnbinned = false;
  //  Instrument instr = UNDEF;
  Coordinate_type maskxytype= UNDEF_XY;
  IO_status status = IO_SUCCESS;

  if( mask.exists() == FALSE )
    message("no mask defined.");
  else
    maskxytype = mask.getxytype();
  message("mask coordinate type = ",(int)maskxytype);

  if( nbadpix > 0 || ctype == RAW_XY || maskxytype == RAW_XY || ccdnumber >= 0)
    usechipcoordinates = true;
 
  if( ctype == RAW_XY){
    message("ccd number ",ccdnumber);
    if( dx != 0.0 || dy != 0.0 )
      warning("parameters dx/dy are ignored in CCD mode");
    makechipmap = true;
    ctype = SKY_XY;
    xbinfactor = ybinfactor = binfactor = 1;
    warning("for chip mapping binfactor= 1 and size ignored");
  }
  else if(ctype == XMM_RGS_XDSP_UNBINNED){
    rgsUnbinned = true;
    ctype = XMM_RGS_XDSP;
  }//if
  if( maskxytype == DET_XY && ctype != DET_XY)
    return error("mask and events coordinate types are incompatible!");

#if 0
  if( get_gti(filename, gtifile, ccdnumber) == FAILURE )
    return error("error reading GTIs");

  //  set_fname("extract_image");
  if( start_gti[0]+tend < stop_gti[n_gti-1]){
    warning("Changing GTI");
    exclude_time( start_gti[0]+tend,stop_gti[n_gti-1]);
  }
  if( tstart > 0.0 ){
    warning("Changing GTI");
    exclude_time( start_gti[0], start_gti[0]+tstart);
  }
#endif


  //  mask.setChatty( false );
  for( unsigned long m = 0; status == IO_SUCCESS ; m++ ){

    setChatLevel(JVMessages::CHATTY);
    status=readEvents( filename, &n_elements, ch_type, ctype, b_size, 
			usechipcoordinates );

    if( status == IO_ERROR)
      return error("I/O error");

    if( m == 0  ){
      message("Number of badpixels = ",nbadpix);
      get_chipsize(instrumentID, &xchipsize, &ychipsize, &nchip);
      if( nGTI == 0 ){
	if( strncasecmp(gtifile,"-", strlen(gtifile)) == 0 )
	  readGTI( filename, ccdnumber );
	else
	  readGTI( gtifile, ccdnumber );
      }
      else
	warning("using GTIs that are already present");
      if(tstart > 0.0 )
	excludeInterval( start_time, start_time+tstart);
      if( start_time+tend < stop_time )
	excludeInterval( start_time+tend, stop_time );

      if( !makechipmap ){

	message("Coordinate type = ", ctype);
	xsize = ysize = im_size;
	nchip = 1;
	if( instrumentID != XMM_RGS1 && 
	    instrumentID != XMM_RGS2  && 
	    ctype !=  LETGS_LAMBDA_TGD	    &&
	    cdelt1*cdelt2 != 0.0 ){
	  newcrpix1 -= dx/(cdelt1*3600.0*xbinfactor);
	  newcrpix2 -= dy/(cdelt2*3600.0*ybinfactor);
	}else if(ctype == LETGS_LAMBDA_TGD ){
	  warning("LETGS lambda/cross dispersion: under construction");
	  message("cdelt2 = ", cdelt2);
	  message("crpix2 = ", crpix2);
	  message("crval2 = ", crval2);
	  newcrpix1 = 1;
	  crpix1 = 0;
       	  crpix2 = ny/2;
	  cdelt1 = 1.0;
	  cdelt2 = 1.0;
	  crval1  = 0.0;
	  crval2  = 0.0;
	  xbinfactor = 0.4 * binfactor;
	  ybinfactor = 0.00015 * binfactor;
	}else if( xdsp_wid >=0.0 && beta_wid >=0.0){
	  warning("using RGS data settings");
	  if( ctype == XMM_RGS_XDSP && !rgsUnbinned ){
	    ybinfactor = 2;
	    cdelt1 = beta_wid * RAD2DEG;
	    cdelt2 = 1.0/3600.0;
	    crpix2 = 0.0;
	    ysize  = (int)(5.3/60.0/cdelt2/fabs(ybinfactor) + 0.5);
	    newcrpix2  =  ysize/2; 
	    message("RGS: use a XDSP width of 5.3 arcmin, ysize= ", ysize);
	  }else if( ctype == XMM_RGS_XDSP && rgsUnbinned ){
	    ybinfactor = binfactor;
	    cdelt2  = 1.0/3600.0;
	    cdelt1  = beta_wid*RAD2DEG;
	    xbinfactor = fabs(ybinfactor) * fabs(cdelt2/cdelt1);
	    crpix2 = 0.0;
	    ysize  = (int)(5.3/60.0/cdelt2/fabs(ybinfactor) + 0.5);
	    //	    ysize=xsize;
	    newcrpix2  =  ysize/2; 
	    message("cdelt1      = ", cdelt1);
	    message("cdelt2      = ", cdelt2);
	    message("X binfactor = ", (float)xbinfactor);
	    message("Y binfactor = ", (float)ybinfactor);
	  }
	  else if( ctype == XMM_RGS_PI ){
	    ybinfactor = 5;
	    newcrpix2  = 1;
	    ysize  = (int)((float)ch_max/ybinfactor + 1.0);
	  }
	  cdelt1 = beta_wid * RAD2DEG;
	  crval1 = beta_ref * RAD2DEG;
	  //	  dx     += BETA_0 - crval1;
	  newcrpix1 = crpix1 - dx/(cdelt1*3600.0*xbinfactor);
	  message("cdelt1 = ", cdelt1);
	  message("crpix1 = ", crpix1);
	  message("crval1 = ", crval1);
	  message("cdelt2 = ", cdelt2);
	  message("crpix2 = ", crpix2);
	  message("crval2 = ", crval2);
	}//else if (XMM RGS
      }//if makechipmap==FALSE
      else{
	drpix1 = drpix2 = 0;
	xsize = xchipsize;
	ysize = ychipsize;
	if( ccdnumber >= 0 ) nchip = 1;// map all chips
	message("Number of chips to be used = ",nchip);
      }//else
      if( nx != xsize || ny != ysize || makechipmap == true){
	if( allocate( nchip * xsize, ysize) == FAILURE){
	  read_events( CLOSE_FITS_FILE, &n_elements);
	  return error("allocation error!");
	}//if
      }
      else
	warning("using existing image");
      if( (maskxytype == DET_XY || maskxytype == RAW_XY) && 
	  strcmp(instrument,mask.instrument)!= 0  )
	warning("instrument keywords events & mask don't match!");
      processtatus(process_string, 0.0, TRUE, FALSE);
    }//if( m== 0 )
    setChatLevel(JVMessages::SUPPRESS_ALL_MESSAGES);
    if( status != IO_ERROR ){

      // Use pointers for speed reasons
      x_ptr = &x[0];           
      y_ptr = &y[0];
      chan_ptr = &channel[0];
      pat_ptr = &pattern[0];
      fl_ptr = &flag[0];
      time_ptr = &time[0];
      ccd_ptr = &ccdnr[0];
      if( usechipcoordinates  ){
	dx_ptr = &detx[0];           
	dy_ptr = &dety[0];
      }
	
      //      setChatty( false);
      setChatLevel( JVMessages::SUPPRESS_ALL_MESSAGES );
      for( k = 0 ; k < n_elements ; k++ ){

	if( xy_type == XMM_RGS_XDSP || xy_type == XMM_RGS_PI  ){
	  *x_ptr += (float)rand() /RAND_MAX - 0.5; // randomize
	  if( xy_type == XMM_RGS_XDSP  )
	    *y_ptr *= rgs_xdspfactor;
	}

	switch( maskxytype){
	case SKY_XY:
	  getradec(  *x_ptr, *y_ptr, &alpha, &delta );
	  mask.getxy( alpha, delta, &xmask, &ymask);
	  pixval = mask.get( (int)(xmask + 0.5), (int)(ymask + 0.5) );
	  break;
	case DET_XY:
	  xmask = *x_ptr;
	  ymask = *y_ptr;
	  mask.convert_detxy( *this, &xmask, &ymask);
	  pixval = mask.get( (int)(xmask + 0.5), (int)(ymask + 0.5) );
	  break;
	case RAW_XY:
	  xmask = *dx_ptr;
	  mask.convert_chipxy( *this, *ccd_ptr, xchipsize, &xmask);
	  pixval = mask.get( (int)(xmask + 0.5), (int)(*dy_ptr + 0.5) );
	  break;
	case XMM_RGS_XDSP:
	case XMM_RGS_PI:
	  xmask = *x_ptr;
	  if(  maskxytype == XMM_RGS_PI && getxytype() == XMM_RGS_XDSP) 
	    ymask = *chan_ptr;
	  else
	    ymask = *y_ptr;
	  mask.convertRGS_XY( *this, &xmask, &ymask);
	  pixval = mask.get( (int)(xmask + 0.5), (int)(ymask + 0.5) );
	  break;
	default:
	  pixval = 1.0;
	}//switch


	if(  *chan_ptr >= ch_min &&  *chan_ptr <= ch_max && 
	     checkTime( *time_ptr, *ccd_ptr ) == true && 
       	     ( ccdnumber < 0 || ccdnumber == *ccd_ptr) &&
	     ( nbadpix== 0 || is_not_badpix(int(*dx_ptr+0.5), int(*dy_ptr+0.5),
					    *ccd_ptr) == true) &&
	     check_flags( *pat_ptr, *fl_ptr) == true &&
	     pixval > 0.5 ){
	  // -1 for FITS convention
	  if( !makechipmap  ){ 
	    i = (int)( (*x_ptr - crpix1)/xbinfactor + newcrpix1  + 0.5) - 1; 
	    j = (int)( (*y_ptr - crpix2)/ybinfactor + newcrpix2  + 0.5) - 1;
	  }
	  else if( *ccd_ptr == ccdnumber || ccdnumber < 0  ){
	    i = (int)( *dx_ptr/xbinfactor + 0.5) - 1; 
	    j = (int)( *dy_ptr/ybinfactor + 0.5) - 1;
	    if( ccdnumber < 0 ) 
	      i += (int)(((*ccd_ptr) - 1)*(double)xsize/(double)xbinfactor); 
	  }
	  else
	    i = j = -1;
	  if( i >= 0 && i < nx && j >= 0 && j < ny ){
	    pixel[i + j*nx] += 1.0;
	    averagetime += *time_ptr;
	    n_accepted++;
	  }
	  else
	    n_rejected++;

	}//if(  *chan_ptr >= ch_min
	else
	    n_rejected++;
	chan_ptr++;
	fl_ptr++;
	pat_ptr++;
	time_ptr++;
	x_ptr++;
	y_ptr++;
	ccd_ptr++;
	if( usechipcoordinates){
	  dx_ptr++;
	  dy_ptr++;
	}

      }//for

    }//if
    else{
      if( ntot_events > 0 )
	processtatus(process_string,100.0*
		     (n_rejected+n_accepted)/(float)ntot_events, false, true);
      else
	processtatus(process_string,0.0, false, true);
      return error("an error occurred reading the event list");
    }
    setChatLevel( JVMessages::CHATTY);
    processtatus( process_string,
		  100.0*(n_rejected+n_accepted)/(float)ntot_events, 
		  false, false);
  }// for

  processtatus(process_string,
	       100.0*(n_rejected+n_accepted)/(float)ntot_events, 
	       FALSE, TRUE);

  message("NBADPIX = ",nbadpix);
  chanMin = ch_min;
  chanMax = ch_max;
  imgbin = binfactor;
  // calculate optical axis in old world coordinate system
  double alpha_opt, delta_opt; // optical axis
  if( ccdnumber < 0 && cdelt1*cdelt2 != 0.0 &&opticx == 0.0 && opticy == 0.0 ){
    opticx = newcrpix1 +  dx/(cdelt1*3600.0*binfactor);
    opticy = newcrpix2 +  dy/(cdelt2*3600.0*binfactor);
  }//if

  if( ctype == SKY_XY ) getradec(opticx, opticy, &alpha_opt, &delta_opt );

  if( makechipmap == FALSE ){
    crpix1 = newcrpix1;
    crpix2 = newcrpix2;
    cdelt1 *= xbinfactor;
    cdelt2 *= ybinfactor;
    // crval1/2 does not change 
    if( ctype == DET_XY ){
      //    strncpy(ctype1,"DET----X",8);    strncpy(ctype2,"DET----Y",8);
      strncpy(ctype1,"DETX",8);
      strncpy(ctype2,"DETY",8);
    }//
    else if( instrumentID != XMM_RGS1 && instrumentID != XMM_RGS2 )
      getxy(alpha_opt, delta_opt, &opticx, &opticy );
  }
  else{
    crpix1 =  crpix2 = crval1 = crval2 = 1.0/binfactor;
    cdelt1 = cdelt2 = 1.0/binfactor;
    opticx = opticy = 0.0;
    strncpy(ctype1,"CHIPX",8);
    strncpy(ctype2,"CHIPY",8);
  }//if

  calcExposure();


  if( n_accepted > 0){
    averagetime /= n_accepted;
    mjdmean = averagetime/(24.0*3600.0);
    mjdmean += mjdrefi + mjdreff;
  }//if

  message("Minimum channel = ",ch_min);
  message("Maximum channel = ",ch_max);
  message("Number of accepted photons = ",n_accepted);
  message("Number of rejected photons = ",n_rejected);

  // insert history
  char historyText[HIST_LEN];
  sprintf(historyText,"Number of accepted photons = %li",n_accepted);
  puthistorytxt(historyText );
  sprintf(historyText,"Number of rejected photons = %li",n_rejected);
  puthistorytxt(historyText );
  sprintf(historyText,"Total number of photons = %li",n_events);
  puthistorytxt(historyText );
  sprintf(historyText,"Number of bad pixels       = %li",nbadpix);
  puthistorytxt(historyText );

  if( n_events > n_accepted + n_rejected ){
    n_events = n_accepted;
    warning("buffer size too small");
  }//if

  n_events = n_accepted;
  mask.setChatty( true );
  setprojection();

  if( status == IO_ERROR )
    return error("an error occurred reading the event list");

  return success();
}//extract_image()



float mexicanHat(float x, float x0, float sigma)
{
  float t=(x - x0)/sigma;
  float mh = (1.0 - t*t)* exp(- t*t/2.0)/(sqrt(2.0*M_PI)*CUBE(sigma));

  //  printf("%f %f %f -> %f\n",x, x0, sigma,mh);
  return mh;
}
float gaussian(float x, float x0, float sigma)
{
  float t=(x - x0)/sigma;
  float mh = exp(- t*t/2.0)/(sqrt(2.0*M_PI)*SQR(sigma));

  //  printf("%f %f %f -> %f\n",x, x0, sigma,mh);
  return mh;
}
