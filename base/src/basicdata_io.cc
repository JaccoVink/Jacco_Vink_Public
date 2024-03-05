/****************************************************************************/
//
// FILE        :  basicdata_io.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  BasicData in/out functions
//
//
// COPYRIGHT   : Jacco Vink,  1999, 2002
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/****************************************************************************/
#include "jvtools_basic.h" // includes all other necessary files
#include "jvtools_cfitsio.h"
//#include "jvtools_polarization.h"


using namespace::jvtools;
using namespace::JVMessages;

/****************************************************************************/
//
// FUNCTION    : read_events()
// DESCRIPTION : Reads the event list (in FITS format).
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//    filename[]        I               filename, or command "close_fits_file"
//
//  RETURNS    : return SUCCESS
//
/****************************************************************************/
IO_status BasicData::read_events(const char filename[],
				 unsigned long *n_elements,
				 const char ch_type[], 
				 Coordinate_type coord_type,
				 unsigned long b_size, bool usechipxy)
{
  return readEvents(filename, n_elements,ch_type, coord_type, 
		    b_size, usechipxy);
}

IO_status BasicData::readEvents(const char filename[],
				 unsigned long *n_elements,
				 const char ch_type[], 
				 Coordinate_type coord_type,
				 unsigned long b_size, bool usechipxy)
{
  // some variables for using fitsio routines 
  setFunctionID("readEvents");

  fitsfile *fptr=0;
  static CfitsioClass fitsFile;
  static int x_col=0, y_col=0, chan_col=0, time_col=0, flag_col=0;
  static int dx_col=0, dy_col=0, pat_col = 0, ccd_col = 0, node_col=0;
  static int polphi_col=0, polU_col=0, polQ_col=0;
  static int energy_col=0;
  //  static long naxis1= 0, naxis2=0, naxis3=0;
  static long n_read = 0, ntot=0;
  fptr = fitsFile.fptr;

  int anynul, status = 0;
  long firstelem=1, firstrow;


  if( fitsFile.fptr != 0 && strncasecmp(CLOSE_FITS_FILE, filename,15) == 0){
    fitsFile.close();
    fptr = 0;
    n_read = 0;
    ntot = 0;
    x_col= y_col= chan_col = time_col = flag_col = pat_col = 0;
    ccd_col = node_col = dx_col= dy_col=0;
    warning("fits file closed prematurely on request.");
    success();
    return IO_FINISHED;
  }//if


  *n_elements = 0;

  // The first time this file is encountered the file is opened
  // and keywords are read
  if( fitsFile.fptr == 0 ){

    char keyw[FLEN_KEYWORD];
    //    char comment[FLEN_COMMENT];
    double  s, todeg = 180.0/M_PI;

    resetKeywords();
    n_events = 0;

    //  Open the fits file.
    fitsFile.open( filename);
    fptr = fitsFile.fptr;

    if( fitsFile.statusReturn() != 0 ){
      error("unable to open event list!", fitsFile.status);
      fitsFile.close();
      return IO_ERROR;
    }//if

    readStdKeywords( &fitsFile);
#if 0
    if( instrumentID == IXPE_GPD || instrumentID == XIPE_GPD ){
      polarizationColumns=true;
      message("polarization columns will be read");
    }
#endif
    
    if( allocate( b_size) == FAILURE ){
      error("allocation error");
      return IO_ERROR;
    }//if
    message("Using a buffer size of ", b_size);


    
    fitsFile.moveTo("EVENTS");
    if( fitsFile.statusReturn() != 0 ){
      fitsFile.resetStatus();
      fitsFile.moveTo("STDEVT");
      if( fitsFile.statusReturn() != 0 ){
	error("could not find EVENTS extension");
	return IO_ERROR;
      }
    }//if
    if( fitsFile.naxis1 == 0 || fitsFile.naxis2 == 0 ){
      error("empty event list  found");
      return IO_ERROR;
    }//if


    readStdKeywords( &fitsFile);

    ntot =  ntot_events = fitsFile.naxis2;
    message("#events = ", ntot_events);


    fits_get_colnum( fptr, FALSE, "TIME", &time_col, &status);
    status = 0;
    fits_get_colnum( fptr, FALSE, "FLAG", &flag_col, &status);
    status = 0;

    if( strncasecmp(telescope,"XMM",3)==0){
      if(  strncasecmp(instrument,"RGS",3)==0)
	fits_get_colnum( fptr, FALSE, "GRADE", &pat_col, &status);
      else
	fits_get_colnum( fptr, FALSE, "PATTERN", &pat_col, &status);
      status = 0;
      fits_get_colnum( fptr, FALSE, "CCDNR", &ccd_col, &status);
      status = 0;
    }
    else if( strncasecmp(telescope,"CHANDRA",7)==0){
      fits_get_colnum( fptr, FALSE, "GRADE", &pat_col, &status);
      status = 0;
      fits_get_colnum( fptr, FALSE, "CCD_ID", &ccd_col, &status);
      status = 0;
      fits_get_colnum( fptr, FALSE, "NODE_ID", &node_col, &status);
      status = 0;
    }
    else if( strncasecmp(telescope,"SUZAKU",7)==0){
      fits_get_colnum( fptr, FALSE, "GRADE", &pat_col, &status);
      status = 0;
    }
    if( usechipxy == true ){
      
      if( instrumentID == XMM_RGS1 || instrumentID == XMM_RGS2 ){
	fits_get_colnum( fptr, FALSE, "RAWX", &dx_col, &status);
	status = 0;
	fits_get_colnum( fptr, FALSE, "RAWY", &dy_col, &status);
	status = 0;
      }
      else if( strncasecmp(telescope,"CHANDRA",7)== 0 || 
	       (strncasecmp(telescope,"XMM",3)== 0 &&
		strncasecmp(instrument,"RGS",3) == 0)){
	fits_get_colnum( fptr, FALSE, "chipx", &dx_col, &status);
	status = 0;
	fits_get_colnum( fptr, FALSE, "chipy", &dy_col, &status);
	status = 0;
      }
      else{
	fits_get_colnum( fptr, FALSE, "RAWX", &dx_col, &status);
	status = 0;
	fits_get_colnum( fptr, FALSE, "RAWY", &dy_col, &status);
	status = 0;
      }//if
    }
    if( polarizationColumns ){
      //      *polAngle, *polx, *poly;
      	fits_get_colnum( fptr, FALSE, "PHI", &polphi_col, &status);
	status = 0;
	fits_get_colnum( fptr, FALSE, "PHE_U", &polU_col, &status);
	status = 0;
	fits_get_colnum( fptr, FALSE, "PHE_Q", &polQ_col, &status);
	status = 0;
    }

    message("Coordinate Type = ", coord_type);
    switch( coord_type ){
    case SKY_XY :	
      fits_get_colnum( fptr, FALSE, "X", &x_col, &status);
      fits_get_colnum( fptr, FALSE, "Y", &y_col, &status);
      break;
    case DET_XY :
      fits_get_colnum( fptr, FALSE, "DETX", &x_col, &status);
      fits_get_colnum( fptr, FALSE, "DETY", &y_col, &status);
      break;
    case RAW_XY:
      fits_get_colnum( fptr, FALSE, "RAWX", &x_col, &status);
      fits_get_colnum( fptr, FALSE, "RAWY", &y_col, &status);
      break;
    case AXAF_XY:
      fits_get_colnum( fptr, FALSE, "detx", &x_col, &status);
      fits_get_colnum( fptr, FALSE, "dety", &y_col, &status);
      break;
    case XMM_RGS_XDSP:
      fits_get_colnum( fptr, FALSE, "BETA_CHANNEL", &x_col, &status);
      //      fits_get_colnum( fptr, FALSE, "XDSP_CORR", &y_col, &status);
      fits_get_colnum( fptr, FALSE, "XDSP_CHANNEL", &y_col, &status);
      break;
    case XMM_RGS_PI:
      fits_get_colnum( fptr, FALSE, "BETA_CHANNEL", &x_col, &status);
      fits_get_colnum( fptr, FALSE, "PI", &y_col, &status);
      break;
    case LETGS_LAMBDA_TGD:
      fits_get_colnum( fptr, FALSE, "tg_lam", &x_col, &status);
      fits_get_colnum( fptr, FALSE, "tg_d", &y_col, &status);
      break;
    case RA_DEC:
      fits_get_colnum( fptr, FALSE, "RA", &x_col, &status);
      fits_get_colnum( fptr, FALSE, "DEC", &y_col, &status);
      break;
    case L_B  :
    default:
      warning("XY type not supported!");
    }// switch


    if( status == 0 ){
      fitsFile.resetStatus();

      sprintf(keyw,"%s%i","TCTYP", x_col );
      fitsFile.readKey(  keyw, &ctype1[0] );
      sprintf(keyw,"%s%i","TCTYP", y_col );
      fitsFile.readKey(  keyw, &ctype2[0] );

      if( fitsFile.statusReturn() != 0 ){
	warning("could not obtain ctype1/2 keywords, trying CTYPE");
	fitsFile.resetStatus();
	fitsFile.readKey( "CTYPE1", ctype1);
	fitsFile.readKey( "CTYPE2", ctype2);

	if( fitsFile.statusReturn() != 0 && 
	    strncasecmp(instrument,"RGS",3) != 0 )
	  warning("unable to get projection type.");
	fitsFile.resetStatus();
      }//if
      setCoordinateType();


      sprintf(keyw,"%s%i","TCROT", y_col );
      crota2 = fitsFile.readKeyDbl(keyw);
      pc001001 = pc002002 = 1.0;
      pc002001 = pc001002 = 0.0;
      fitsFile.resetStatus();

      sprintf(keyw,"%s%i","TCRVL", x_col );
      crval1 = fitsFile.readKeyDbl(keyw);
      sprintf(keyw,"%s%i","TCRVL", y_col );
      crval2 = fitsFile.readKeyDbl(keyw);
      sprintf(keyw,"%s%i","TCRPX", x_col );
      crpix1 = fitsFile.readKeyDbl(keyw);
      sprintf(keyw,"%s%i","TCRPX", y_col );
      crpix2 = fitsFile.readKeyDbl(keyw);
      sprintf(keyw,"%s%i","TCDLT", x_col );
      cdelt1 = fitsFile.readKeyDbl(keyw);
      sprintf(keyw,"%s%i","TCDLT", y_col );
      cdelt2 = fitsFile.readKeyDbl(keyw);

      
      if( coord_type == XMM_RGS_XDSP || coord_type == XMM_RGS_PI ){
	beta_ref = fitsFile.readKeyDbl("BETA_REF");
	beta_wid = fitsFile.readKeyDbl("BETA_WID");
	if( coord_type == XMM_RGS_XDSP ){
	  xdsp_ref = fitsFile.readKeyDbl("XDSP_REF");
	  xdsp_wid = fitsFile.readKeyDbl("XDSP_WID");
	}
	if( fitsFile.statusReturn() != 0 ){
	  warning("could not find beta_ref/beta_wid keywords");
	  warning("using crval/cdelt keywords");
	  fitsFile.resetStatus();
	  beta_ref = crval1;
	  beta_wid = cdelt1;
	  strncpy(ctype1,"RGS--DSP", FLEN_VALUE);
	  if( coord_type == XMM_RGS_XDSP ){
	    xdsp_ref = crval2;
	    xdsp_wid = cdelt2;
	    strncpy(ctype2,"RGS-XDSP", FLEN_VALUE);
	  }
	  else
	    strncpy(ctype2,"RGS---PI", FLEN_VALUE);
	}//if
	if( status != 0 ){//???????????????
	  error("error reading RGS keywords!");
	  //	  fits_close_file(fptr, &status);
	  fitsFile.close();
	  fptr = 0;
	  n_read = ntot = 0;
	  x_col=y_col=chan_col = time_col =flag_col =pat_col = dx_col=dy_col=0;
	  return IO_FINISHED;
	}//if

      }else if( fitsFile.statusReturn() != 0 ){
	warning("could not obtain all cdelt/crpix keywords"); 
	fitsFile.resetStatus();
	crval1 = fitsFile.readKeyDbl("CRVAL1");
	crval2 = fitsFile.readKeyDbl("CRVAL2");
	crpix1 = fitsFile.readKeyDbl("CRPIX1");
	crpix2 = fitsFile.readKeyDbl("CRPIX2");
	cdelt1 = fitsFile.readKeyDbl("CDELT1");
	cdelt2 = fitsFile.readKeyDbl("CDELT2");
	crota2 = fitsFile.readKeyDbl("CROTA2");
	if( crota2  != 0.0 && cdelt1 != 0.0){
	  s = cdelt2/cdelt1;
	  pc001001 =  cos(crota2/todeg);
	  pc001002 = -sin(crota2/todeg) * s;
	  pc002001 =  sin(crota2/todeg) / s; // CHECK signs!!!!
	  pc002002 =  cos(crota2/todeg);
	}
	fitsFile.resetStatus();
      }//else


      if( usechipxy == TRUE  ){
	sprintf(keyw,"%s%i","TCRVL", dx_col );
	drval1 = fitsFile.readKeyDbl(keyw);
	sprintf(keyw,"%s%i","TCRVL", dy_col );
	drval2 = fitsFile.readKeyDbl(keyw);
	sprintf(keyw,"%s%i","TCRPX", dx_col );
	drpix1 = fitsFile.readKeyDbl(keyw);
	sprintf(keyw,"%s%i","TCRPX", dy_col );
	drpix2 = fitsFile.readKeyDbl(keyw);
	sprintf(keyw,"%s%i","TCDLT", dx_col );
	drdelt1 = fitsFile.readKeyDbl(keyw);
	sprintf(keyw,"%s%i","TCDLT", dy_col );
	drdelt2 = fitsFile.readKeyDbl(keyw);
	if( fitsFile.statusReturn() != 0 && 
	    strncasecmp(instrument,"EPN",3) == 0 ){
	  drdelt1 = -0.00113888;
	  drdelt2 = 0.00113888;
	  drpix1 = 1;
	  drpix2 = 1;
	}//if
	fitsFile.resetStatus();
      }//

      // optional optical axes:
      sprintf(keyw,"%s%i","OPTIC", x_col );
      opticx = fitsFile.readKeyDbl("keyw");
      sprintf(keyw,"%s%i","OPTIC", y_col );
      opticy = fitsFile.readKeyDbl("keyw");

      if( fitsFile.statusReturn() != 0){
	opticx = crpix1;
	opticy = crpix2;
      }//if

    }
    else
      warning("error finding XY column!",status);


    fitsFile.resetStatus();
    if( strncasecmp(ch_type,"PI",2) == 0 ){
      fits_get_colnum( fptr, FALSE, "PI", &chan_col, &status);
      strncpy( chan_type, ch_type,FLEN_VALUE);
    }
    else if( strncasecmp(ch_type,"PHA",3) == 0 ){
      fits_get_colnum( fptr, FALSE, "PHA", &chan_col, &status);
      strncpy( chan_type, ch_type,FLEN_VALUE);
    }
    else if( strncasecmp(ch_type,"ENG",3) == 0 ){
      fits_get_colnum( fptr, FALSE, "energy", &chan_col, &status);
      strncpy( chan_type, ch_type,FLEN_VALUE);
    }
    else if( strncasecmp(ch_type,"energy",6) == 0 ){
      fits_get_colnum( fptr, FALSE, "energy", &energy_col, &status);
      chan_col = energy_col;
      strncpy( chan_type, ch_type,FLEN_VALUE);
    }
    else
      warning("channel type not supported: ", ch_type);


    if( status != 0 ){
      warning("Error finding channel column!",status);
      status = 0;
    }//if
    sprintf(keyw,"%s%i","TLMIN", chan_col );
    chanMin = fitsFile.readKeyInt(keyw);
    sprintf(keyw,"%s%i","TLMAX", chan_col );
    chanMax = fitsFile.readKeyInt(keyw);
    fitsFile.resetStatus();
    status = 0;
  }//if( fptr==0 : first time event list is entered

  /*

    Only this piece of the function is always used

  */

  // Always buffer_size elements are read,, unless we're near end of list
  *n_elements = MIN( (long)buffer_size, ntot - n_read); 

  firstrow  = n_read + 1;
  firstelem = 1; // allways!

  if( x_col != 0 && y_col != 0 ){
    fits_read_col_flt(fptr, x_col, firstrow, firstelem, *n_elements, 0.0, 
		      &x[0], &anynul, &status );
    fits_read_col_flt(fptr, y_col, firstrow, firstelem, *n_elements, 0.0, 
		      &y[0], &anynul, &status );
  }//if

  if( time_col != 0 )
    fits_read_col_dbl(fptr, time_col, firstrow, firstelem, *n_elements, 0.0, 
		      &time[0], &anynul, &status );

  if( chan_col != 0 )
    fits_read_col_int(fptr, chan_col, firstrow, firstelem, *n_elements, 0, 
		      &channel[0], &anynul, &status );

  if( energy_col != 0 )
    fits_read_col_flt(fptr, chan_col, firstrow, firstelem, *n_elements, 0, 
		      &chanEnergy[0], &anynul, &status );

  if( flag_col != 0 )
    fits_read_col_lng(fptr, flag_col, firstrow, firstelem, *n_elements, 0, 
		      &flag[0], &anynul, &status );

  if( pat_col != 0 )
    fits_read_col_int(fptr, pat_col, firstrow, firstelem, *n_elements, 0, 
		      &pattern[0], &anynul, &status );

  if( dx_col != 0 )
    fits_read_col_flt(fptr, dx_col, firstrow, firstelem, *n_elements, 0, 
		      &detx[0], &anynul, &status );
  if( dy_col != 0 )
    fits_read_col_flt(fptr, dy_col, firstrow, firstelem, *n_elements, 0, 
		      &dety[0], &anynul, &status );
  if( ccd_col != 0 )
    fits_read_col_int(fptr, ccd_col, firstrow, firstelem, *n_elements, 0, 
		      &ccdnr[0], &anynul, &status );

  if( node_col != 0 )
    fits_read_col_int(fptr, node_col, firstrow, firstelem, *n_elements, 0, 
		      &node_id[0], &anynul, &status );

  if( polphi_col != 0 )
    fits_read_col_flt(fptr, polphi_col, firstrow, firstelem, *n_elements, 0, 
		      &polAngle[0], &anynul, &status );
  if( polQ_col != 0 )
    fits_read_col_flt(fptr, polQ_col, firstrow, firstelem, *n_elements, 0, 
		      &polx[0], &anynul, &status );
  if( polU_col != 0 )
    fits_read_col_flt(fptr, polU_col, firstrow, firstelem, *n_elements, 0, 
		      &poly[0], &anynul, &status );
  

  if( status != 0 ){
    error("Error reading events!",status);
    //    fits_close_file(fptr, &status);
    fitsFile.close();
    return IO_ERROR;
  }
  n_read += *n_elements; 
  n_events += *n_elements;

  if( n_read > ntot )// should never occur!!
    warning("A bug : n_read > naxis1!!! nread=", n_read);

  // When the number of read events equals naxis1, all events have been read
  // the file should be closed and variable reinitialized

  if( applyPSF){
    warning("A PSF will be applied to x & y");
    warning("THIS IS A SMALLER VERSION OF THE CODE, PSF BROADENING TO EVENTS CANNOT BE APPLIED IN THIS VERSION");
    //    applyPSFevents(*n_elements);
  }
  
  if( n_read >= ntot ){
    fitsFile.close();
    fptr = 0;
    n_read = 0;
    ntot = 0;
    x_col = y_col = chan_col = time_col = 0;

    success();
    return IO_FINISHED;
  }//if

  success();
  return IO_SUCCESS;

}//  read_events( )


/****************************************************************************/
//
// FUNCTION    : readStdKeywords()
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
/****************************************************************************/
Status BasicData::read_std_keywords( fitsfile *fptr )
{
  return readStdKeywords(fptr);
}
Status BasicData::readStdKeywords( const char gtiFileName[] )
{
  setFunctionID("readStdKeywords");
  
  
  CfitsioClass fitsFile;
  int status=0;
  if(   (status =fitsFile.open(gtiFileName)) != 0 )
    return error("error opening file, status = ", status);


  if( readStdKeywords( &fitsFile) == FAILURE )
    return error("error reading file ", gtiFileName);

  return success();
}//
Status BasicData::readStdKeywords( fitsfile *fptr )
{

  setFunctionID("readStdKeywords");

  int status=0;
  char comment[FLEN_COMMENT];


  fits_read_key_str(fptr,"TELESCOP", telescope, comment, &status);
  status = 0;
  fits_read_key_str(fptr,"INSTRUME", instrument, comment, &status);
  status = 0;
  fits_read_key_str(fptr,"DETNAM", detector, comment, &status);
  status = 0;
  fits_read_key_str(fptr,"FILTER", filter, comment, &status);
  status = 0;
  fits_read_key_str(fptr,"GRATING", grating, comment, &status);
  status = 0;
  fits_read_key_str(fptr, "OBJECT" , object, comment, &status);
  status = 0;
  fits_read_key_str(fptr,"DATE-OBS",obsdate, comment, &status);
  status=0;
  fits_read_key_str(fptr,"DATE-END",enddate, comment, &status);
  status=0;
  fits_read_key_flt(fptr,"MJD-OBS",&mjd_obs, comment, &status);
  status=0;
  fits_read_key_dbl(fptr,"MJDMEAN",&mjdmean, comment, &status);
  status=0;
  fits_read_key_dbl(fptr,"MJDREF",&mjdref, comment, &status);
  status=0;
  fits_read_key(fptr,TSTRING,"CREATOR",(void *)&creator[0], comment, &status);

  getInstrument();


 // RA_NOM and DEC_NOM are not realy important
  if( strncasecmp(telescope,"XMM",3)==0 ){
    fits_read_key_flt(fptr,"RA_PNT",&ra_nom, comment, &status);
    fits_read_key_flt(fptr,"DEC_PNT",&dec_nom, comment, &status);
    fits_read_key_dbl(fptr,"PA_PNT",&roll_nom, comment, &status);
    status = 0;
    if( instrumentID == XMM_RGS1 || instrumentID == XMM_RGS2 ){
      fits_read_key_dbl(fptr,"RGSALPHA", &rgsalpha, comment, &status);
      fits_read_key_dbl(fptr,"LINE_SEP", &line_sep, comment, &status);
      if( status == 0 ){
	message("obtained RGSALPHA = ",rgsalpha);
	message("obtained LINE_SEP = ",line_sep);
      }else{
	warning("could not obtain RGS keywords");
	message("obtained RGSALPHA = ",rgsalpha);
	message("obtained LINE_SEP = ",line_sep);
	status=0;
      }
    }//if
    
  }
  else{
    fits_read_key_flt(fptr,"RA_NOM", &ra_nom, comment, &status);
    fits_read_key_flt(fptr,"DEC_NOM",&dec_nom, comment, &status);
    fits_read_key_dbl(fptr,"ROLL_NOM",&roll_nom, comment, &status);
  }//
  status=0;

  fits_read_key_flt(fptr,"EQUINOX", &equinox, comment, &status);
  if( status!= 0 || equinox == 0.0 ){
    status = 0;
    fits_read_key_flt(fptr,"EPOCH", &equinox, comment, &status);
    if( status!= 0 || equinox == 0.0 ){
      status = 0;
      warning("no EQUINOX key word found, assuming 2000.");
      equinox=2000;
    }
  }
  fits_read_key(fptr, TSTRING, "RADECSYS" ,(void *)&radecsys[0], 
		comment, &status);
  if( status !=0 ){
    status = 0;
    if( equinox >= 1984){
      strncpy(radecsys,"FK5",FLEN_VALUE);
    }
    else{
      strncpy(radecsys,"FK4",FLEN_VALUE);
    }
  }//if
  status=0;
  fits_read_key_dbl(fptr,"EXPOSURE", &exposure, comment, &status);
  if( status!=0 || exposure == 0.0 ){
    status=0;
    fits_read_key_dbl(fptr,"ONTIME", &exposure, comment, &status);
    if( status!=0 || exposure == 0.0 ){
      status=0;
      fits_read_key_dbl(fptr,"XS-LIVTI", &exposure, comment, &status);
      if( status!=0 || exposure == 0.0 ){
	warning("No exposure keyword found. Trying EMSCE001 (XMM), otherwise setting exposure=1");
	status = 0;
	fits_read_key_dbl(fptr,"EMSEC002", &exposure, comment, &status);
	if( status != 0 ){
	  exposure = 1.0;
	  status = 0;
	}//if
      }
    }//if
  }//if
  status = 0;
  fits_read_key_dbl(fptr,"XS-DTCOR", &deadtime_corr, comment, &status);
  status = 0;

  fits_read_key_dbl(fptr,"TSTART", &start_time, comment, &status);
  fits_read_key_dbl(fptr,"TSTOP", &stop_time, comment, &status);
  status=0;

  fits_read_key_flt(fptr,"MJD-OBS", &mjd_obs, comment, &status);
  status = 0;

  mjdrefi=0.0;
  mjdreff=0.0;
  fits_read_key_dbl(fptr,"MJDREFI", &mjdrefi, comment, &status);
  fits_read_key_dbl(fptr,"MJDREFF", &mjdreff, comment, &status);
  if( status != 0)
    fits_read_key_dbl(fptr,"MJDREF", &mjdrefi, comment, &status);
  status=0;
  
  return JVMessages::success();
}//read_std_keywords( )
Status BasicData::readStdKeywords( CfitsioClass *fitsFile )
{

  setFunctionID("readStdKeywords");

   
  int oldchat = (int)JVMessages::getChatLevel();

   setChatLevel( JVMessages::SUPPRESS_ALL_MESSAGES);

  fitsFile->readKey( "TELESCOP", telescope);
  fitsFile->readKey( "INSTRUME", instrument );
  fitsFile->resetStatus();
  fitsFile->readKey( "DETNAM", detector );
  fitsFile->resetStatus();
  fitsFile->readKey( "FILTER", filter );
  fitsFile->resetStatus();
  fitsFile->readKey( "GRATING", grating );
  fitsFile->resetStatus();
  fitsFile->readKey( "OBJECT", object );
  fitsFile->resetStatus();
  fitsFile->readKey( "DATE-OBS", obsdate );
  fitsFile->readKey( "DATE-END", enddate );
  fitsFile->resetStatus();
  fitsFile->readKey( "CREATOR", creator );
  fitsFile->resetStatus();

  getInstrument();


  // RA_NOM and DEC_NOM are not realy important
  if( strncasecmp(telescope,"XMM",3)==0 ){
    ra_nom   = fitsFile->readKeyFlt("RA_PNT");
    dec_nom  = fitsFile->readKeyFlt("DEC_PNT");
    roll_nom = fitsFile->readKeyDbl("PA_PNT");
    fitsFile->resetStatus();

    if( instrumentID == XMM_RGS1 || instrumentID == XMM_RGS2 ){
      rgsalpha = fitsFile->readKeyDbl("RGSALPHA");
      line_sep = fitsFile->readKeyDbl("LINE_SEP");
      if( fitsFile->statusReturn() == 0 ){
	message("obtained RGSALPHA = ",rgsalpha);
	message("obtained LINE_SEP = ",line_sep);
      }else{
	warning("could not obtain RGS keywords");
	message("obtained RGSALPHA = ",rgsalpha);
	message("obtained LINE_SEP = ",line_sep);
      }
      fitsFile->resetStatus();
    }//if
    
  }
  else{
    ra_nom   = fitsFile->readKeyFlt("RA_NOM");
    dec_nom  = fitsFile->readKeyFlt("DEC_NOM");
    roll_nom = fitsFile->readKeyDbl("ROLL_NOM");
    fitsFile->resetStatus();
  }//
  equinox =  fitsFile->readKeyFlt("EQUINOX");
  if( fitsFile->statusReturn() != 0 || equinox == 0.0 ){
    fitsFile->resetStatus();
    equinox =  fitsFile->readKeyFlt("EPOCH");
    if( fitsFile->statusReturn() != 0 || equinox == 0.0 ){
    fitsFile->resetStatus();
    warning("no EQUINOX key word found, assuming 2000.");
    equinox = 2000.0;
    }
  }

  fitsFile->readKey( "RADECSYS", radecsys );

  
  if( fitsFile->statusReturn() != 0 ){
    fitsFile->resetStatus();
    if( equinox >= 1984){
      strncpy(radecsys,"FK5",FLEN_VALUE);
    }
    else{
      strncpy(radecsys,"FK4",FLEN_VALUE);
    }
  }//if
  exposure = fitsFile->readKeyDbl("EXPOSURE");

  if( fitsFile->statusReturn() != 0 || exposure == 0.0 ){
    fitsFile->resetStatus();
    exposure = fitsFile->readKeyDbl("ONTIME");
    if( fitsFile->statusReturn() != 0 || exposure == 0.0 ){
      fitsFile->resetStatus();
      exposure = fitsFile->readKeyDbl("XS_LIVTI");
      if( fitsFile->statusReturn() != 0 || exposure == 0.0 ){
	fitsFile->resetStatus();
	warning("No exposure keyword found. Trying EMSCE002 (XMM), otherwise setting exposure=1");
	exposure = fitsFile->readKeyDbl("EMSCE002");
	fitsFile->resetStatus();
	if( fitsFile->statusReturn() != 0 ){
	  exposure=1.0;
	  fitsFile->resetStatus();
	}
      }
    }//if
  }//if
  deadtime_corr = fitsFile->readKeyDbl("XS_DTCOR");
  fitsFile->resetStatus();

  start_time = fitsFile->readKeyDbl("TSTART");
  stop_time = fitsFile->readKeyDbl("TSTOP");
  fitsFile->resetStatus();

  mjd_obs = fitsFile->readKeyFlt("mjd_obs");
  fitsFile->resetStatus();

  mjdmean = fitsFile->readKeyDbl("MJDMEAN");
  fitsFile->resetStatus();  


  
  mjdrefi=0.0;
  mjdreff=0.0;
  mjdrefi = fitsFile->readKeyDbl("MJDREFI");
  mjdreff = fitsFile->readKeyDbl("MJDREFF");
  if( fitsFile->statusReturn() != 0)
    mjdrefi = fitsFile->readKeyDbl("MJDREF");

  
  fitsFile->resetStatus();

  chanMin =  fitsFile->readKeyInt("CHANMIN");
  chanMax =  fitsFile->readKeyInt("CHANMAX");
  if( fitsFile->statusReturn() != 0){
    chanMin = 0;
    chanMax = 2047;
  }//if
  
  fitsFile->resetStatus();
  JVMessages::setChatLevel( (JVMessages::ChatLevel)oldchat );



  return JVMessages::success();
}//read_std_keywords( )



/****************************************************************************/
//
// FUNCTION    : write_std_keywords(
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
/****************************************************************************/
Status BasicData::write_std_keywords( fitsfile *fptr, bool writehistory ) const
{
  return writeStdKeywords(fptr,writehistory);
}
Status BasicData::writeStdKeywords( fitsfile *fptr, bool writehistory ) const
{

  setFunctionID("writeStdKeywords");


  int status=0;
  char comment[FLEN_COMMENT]=" ";

  if( strncasecmp(telescope,"XMM",3)== 0 && instrumentID != XMM_OM ){
    fits_write_key(fptr, TFLOAT, "RA_PNT", (void *)&ra_nom , comment, &status);
    fits_write_key(fptr, TFLOAT, "DEC_PNT",(void *)&dec_nom, comment, &status);
    fits_write_key(fptr, TDOUBLE, "PA_PNT",(void *)&roll_nom,comment,&status);
    //fits_write_key(fptr,TFLOAT, "RAREF", (void *)&ra_nom , comment, &status);
    //fits_write_key(fptr,TFLOAT, "DECREF",(void *)&dec_nom, comment, &status);
  }
  else{
    fits_write_key( fptr, TFLOAT, "RA_NOM", (void *)&ra_nom ,comment, &status);
    fits_write_key( fptr, TFLOAT, "DEC_NOM",(void *)&dec_nom,comment, &status);
    fits_write_key( fptr, TDOUBLE,"ROLL_NOM",(void *)&roll_nom,comment,&status);
  }//if else

   if( status != 0)
     warning("error writing nominal pointing keywords",status);
   status = 0;

  fits_write_key( fptr, TSTRING,"OBJECT",(void *)&object[0], 
		  "Observation target", &status);
  fits_write_key( fptr, TSTRING,"TELESCOP",(void *)&telescope[0], 
		  comment, &status);
  fits_write_key( fptr, TSTRING,"INSTRUME",(void *)&instrument[0], 
		  comment, &status);
  fits_write_key( fptr, TSTRING,"DETNAM",(void *)&detector[0], 
		  comment, &status);

  if( strncasecmp(filter,"UNKNOWN",FLEN_VALUE) != 0 )
    fits_write_key( fptr, TSTRING,"FILTER",(void *)&filter, comment, &status);

  if( strncasecmp(telescope,"XMM",3)== 0 && strncasecmp(telescope,"RGS",3)!= 0)
    fits_write_key_str( fptr, "DATAMODE","IMAGING","XMM specific keyword", 
			&status);  

  if( status != 0)
    warning("error writing observatory keywords",status);
  status = 0;

  fits_write_key_fixflt(fptr, "EQUINOX", equinox, 1, 
			"Equinox for coordinate system", &status);
  fits_write_key(fptr, TSTRING,"RADECSYS",(void *)&radecsys[0],
		  "Frame of reference", &status);
  fits_write_key( fptr,TDOUBLE, "EXPOSURE",(void *)&exposure,
		  "[s] exposure time", &status);

  fits_write_key(fptr,TFLOAT, "MJD-OBS",(void *)&mjd_obs, 
		 "Modified Julian date", &status);

  if( mjdmean > 0.0)
    fits_write_key(fptr,TDOUBLE, "MJDMEAN",(void *)&mjdmean,"Average MJD", 
		   &status);
  if( strncasecmp(obsdate,"UNKNOWN", FLEN_VALUE) != 0)
    fits_write_key(fptr,TSTRING,"DATE-OBS",(void *)&obsdate[0],
		   comment,&status);
  if( strncasecmp(enddate,"UNKNOWN", FLEN_VALUE) != 0)
    fits_write_key(fptr,TSTRING,"DATE-END",(void *)&enddate[0],comment,
		   &status);
  if( chanMin != 0 || chanMax != 2047 ){
    status = 0;
    fits_write_key_fixflt( fptr, "CHANMIN",(float)chanMin, 1,
			   "Minimum energy channel", &status);
    fits_write_key_fixflt( fptr, "CHANMAX", (float)chanMax, 1,
			   "Maximum energy channel", &status);
  }

  if( status != 0)
     warning("error writing observation keywords",status);
   status = 0;

  fits_write_date(fptr, &status);
  fits_write_key(fptr,TSTRING,"CREATOR",(void *)&creator[0], 
		 "Data sets origin", &status);

  if( status != 0)
    return error("error writing keywords",status);

  if( writehistory == true )
    history.add2FitsFile( fptr);
    //    for( int i=0 ; i < nhistory ; i++ )
    //      fits_write_history(fptr, history[i], &status );

  return success();
}//writekeywords( )


/****************************************************************************/
//
// FUNCTION    : writeWCSkeywords
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
Status BasicData::writeWCSkeywords( fitsfile *fptr)
{
  setFunctionID("writeWCSkeywords");

  int status=0;
  
  fits_write_key( fptr,TSTRING,"CTYPE1",(void *)&ctype1[0],"projection type",
		  &status);
  fits_write_key( fptr,TSTRING,"CTYPE2",(void *)&ctype2[0], "projection type",
		  &status);

  fits_write_key( fptr, TDOUBLE,"CRPIX1", (void *)&crpix1, 
		  "X axis reference pixel", &status);
  fits_write_key( fptr, TDOUBLE,"CRPIX2", (void *)&crpix2, 
		  "Y axis reference pixel", &status);
  fits_write_key( fptr, TDOUBLE, "CRVAL1", (void *)&crval1, 
		  "coord of X ref pixel", &status);
  fits_write_key( fptr, TDOUBLE, "CRVAL2", (void *)&crval2, 
		  "coord of Y ref pixel", &status);
  

  if( status!=0 )
    return error("error writing WCS keywords");
  
  return success();
}


/****************************************************************************/
//
// FUNCTION    : getNrHDUs 
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
int BasicData::getNrHDUs(const char fileName[])
{
  int nrhdus = 0;
  
  CfitsioClass fitsFile;

  fitsFile.open(fileName);

  nrhdus = fitsFile.numberOfHDUs();

  return nrhdus;
}



/*****************************************************************************/
//
// FUNCTION    : read_gti()
// DESCRIPTION : Reads the good time intervals from a fits file
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
// filename             I                   Name of input fits file.
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status BasicData::read_gti( const char filename[], int ccdnumber )
{
  setFunctionID("read_gti");

  fitsfile *fgtiptr=0;
  char comment[FLEN_COMMENT], extname[FLEN_VALUE]=" ";
  char gti_extension[FLEN_COMMENT] = "STDGTI";
  int status=0, anynul, hdutype, ixt;
  long firstrow=1, firstelem=1;
  long naxis1, naxis2, tfields;
  double nulval = 0.0;

  fits_open_file( &fgtiptr, filename , READONLY, &status );
  if( status != 0 )
    return errorwarning("error opening GTI file",status);


  fits_read_key_str(fgtiptr, "TELESCOP", telescope , comment, &status);
  fits_read_key_str(fgtiptr, "INSTRUME", instrument , comment, &status);
  fits_read_key_str(fgtiptr, "OBJECT", object , comment, &status);
  status = 0;



  Instrument instr=get_instrument();
  if( ccdnumber >= 1  && 
      (instrumentID == XMM_PN || 
       instrumentID == XMM_MOS1 || instrumentID == XMM_MOS2 ||
       instrumentID == XMM_RGS1 || instrumentID == XMM_RGS2 )
      ){
    if( ccdnumber >= 10 )
      sprintf(gti_extension,"%s%i","STDGTI", ccdnumber);
    else
      sprintf(gti_extension,"%s0%i","STDGTI", ccdnumber);
    message("using ccd specific GTI for chip ",ccdnumber);
  }
  else if( instr == XMM_PN )
    sprintf(gti_extension,"%s%i","STDGTI", 12);
  else if( instr == XMM_MOS1 || instr == XMM_MOS2 )
    sprintf(gti_extension,"%s0%i","STDGTI", 1);
  else if ( instr == SAX_ME )
    sprintf(gti_extension,"%s","ALLGTI");
  message("GTI extension name",gti_extension);
  // move to second header unit

  ixt = 2;
  do{
    status = 0;
    fits_movabs_hdu(fgtiptr, ixt , &hdutype, &status);
    fits_read_key_str(fgtiptr, "EXTNAME",extname , comment, &status);
  }while( ixt++ < 100 && strncmp(extname,gti_extension, 8)!= 0 && 
	  strncmp(extname,"GTI",3)!= 0 &&  strncmp(extname,"ALLGTI",6)!= 0);

  if( status != 0 || 
      (strncmp(extname,"STDGTI",6)!= 0 && strncmp(extname,"GTI",3)!= 0 &&
       strncmp(extname,"ALLGTI",8)!= 0) ){
    warning("Could not find GTI extension!", status);
    fits_close_file(fgtiptr, &status);
    warning("Improvising and using make_gti()");
    return make_gti(filename);
  }//if

  fits_read_key_lng(fgtiptr, "NAXIS1", &naxis1, comment, &status);
  fits_read_key_lng(fgtiptr, "NAXIS2", &naxis2, comment, &status);
  fits_read_key_lng(fgtiptr, "TFIELDS", &tfields, comment, &status);

  if( tfields < 2 || naxis2 < 1 || status != 0 ){
    fits_close_file(fgtiptr, &status);
    return errorwarning("TFIELDS should be >= 2!", status);
  }//if

  if( allocate_gti( (unsigned long)naxis2 ) == FAILURE ){
    fits_close_file(fgtiptr, &status);
    return errorwarning("An error occured allocating GTI memory!");
  }

  fits_read_key_str(fgtiptr, "TELESCOP", telescope , comment, &status);
  fits_read_key_str(fgtiptr, "INSTRUME", instrument , comment, &status);
  fits_read_key_str(fgtiptr, "OBJECT", object , comment, &status);
  status = 0;

  fits_read_col_dbl(fgtiptr, 1, firstrow, firstelem, naxis2 , nulval, 
		    start_gti, &anynul, &status );
  fits_read_col_dbl(fgtiptr, 2, firstrow, firstelem, naxis2, nulval, 
		    stop_gti, &anynul, &status );

  if( status != 0 ){
    deallocate_gti();
    fits_close_file(fgtiptr, &status);
    return errorwarning("An error occured reading the GTIs!", status);
  }//if
  fits_close_file(fgtiptr, &status);

  if( n_gti <= 0 || start_gti[0] >= stop_gti[0] )
    return errorwarning("No or incorrect GTI!");

  message("number of GTI intervals = ",n_gti);
  if( instrumentID == XMM_RGS1 || instrumentID == XMM_RGS2 )
    mergeGTI();

  calc_exposure();
  if( exposure > 3.15e7 ){
    warning("exposure > 1.0e37, using START/STOP Keywords!");
    return make_gti(filename);
  }//if

  return success();
}// read_gti()


/****************************************************************************/
//
// FUNCTION    : get_gti()
// DESCRIPTION : Layer between extraction functions and read_gti.
//               It checks whether GTIs are present (from n_gti) and
//               if not (or if gtifile==IGNORESTRING) it reads the GTI files
//               contained in file "gtifile".
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  evenstfile          I                      string with filename
//  gtifile             I                      string with filename
//
//  RETURNS    :  SUCCESS or FAILURE
//
/****************************************************************************/
Status BasicData::get_gti(char eventsfile[], char gtifile[], int ccdnumber )
{
  setFunctionID("get_gti");
  if( strcmp( gtifile, IGNORE_STRING) == 0 && n_gti > 0 )
    message("GTIs presumed to be loaded, n_gti =",n_gti);
  else{
    if( strncmp(gtifile,"-",5) == 0 ){
      if( read_gti( eventsfile, ccdnumber ) == FAILURE )
	return errorwarning("error reading GTIs");
    }
    else{
      if( read_gti( gtifile, ccdnumber ) == FAILURE )
	return errorwarning("error reading GTIs");
    }
  }// else

  return success();
}//get_gti()

/****************************************************************************/
//
// FUNCTION    : readGTI()
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
Status BasicData::readGTI( const char gtifile[] , int ccdnumber)
{
  setFunctionID("readGTI");

  //  JVMessages::warning("experimental");
  message("gti file   = ", gtifile);
  message("instrument id = ", instrumentID);
  message("ccd number = ", ccdnumber);


  if( readStdKeywords( gtifile) ==  FAILURE )
    return error("error reading standrad keywords");

  int nccd = 1;
  switch( instrumentID ){
  case XMM_MOS1:
  case XMM_MOS2:
    nccd = 7;
    break;
  case XMM_PN:
    nccd = 12;
    break;
  case XMM_RGS1:
  case XMM_RGS2:
    nccd = 9;
    break;
  case CHANDRA_ACIS:
    nccd = 9;
    break;
  default:
    nccd = 1;
  }//switch


  allocateGTI( nccd );
  message("nccd = ", nccd);
  message("nGTI = ", nGTI);

  //  if( ccdnumber == -1 && nGTI > 0){
  if( nGTI > 0){
    if( nGTI > 1){
      for( int i = 0; i < nGTI; i++){
	gti[i].setInstrumentID( instrumentID);
	gti[i].read(gtifile,i+1);
      }
    }else{
      gti->setInstrumentID( instrumentID);
      gti->read(gtifile, ccdnumber);
    }
  }

#if 0
  else if( ccdnumber == 1 && nGTI == 1 )
    gti->read(gtifile, ccdnumber);
  else if( ccdnumber >= 1 && nGTI >= ccdnumber )
    gti[ccdnumber-1].read(gtifile, ccdnumber);
#endif

  return success();
}//


/*****************************************************************************/
//
// FUNCTION    : make_gti()
// DESCRIPTION : Use this in case read_gti failed!
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
// filename             I                   Name of input fits file.
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status BasicData::make_gti( const char filename[] )
{

  setFunctionID("make_gti");

  fitsfile *fgtiptr;
  char comment[FLEN_COMMENT], extname[FLEN_VALUE];
  int status=0,hdutype;

  warning("A GTI will be made from START and STOP keywords!");

  fits_open_file( &fgtiptr, filename , READONLY, &status );
  if( status != 0 ){
    fits_close_file(fgtiptr, &status);
    return error("Error opening fits file!");
  }//if


  // move to second header unit
  fits_movabs_hdu(fgtiptr, 2, &hdutype, &status);
  // move to the right
  int ixt = 1;
  do{
    status = 0;
    fits_movabs_hdu(fgtiptr, ixt , &hdutype, &status);
    fits_read_key_str(fgtiptr, "EXTNAME",extname , comment, &status);
  }while( ixt++ < 5 && strncmp(extname,"STDEVT",6)!= 0 && 
	  strncmp(extname,"EVENTS",6)!= 0 );
  
  double tstart, tstop;
  fits_read_key_dbl(fgtiptr,"TSTART", &tstart, comment, &status);
  fits_read_key_dbl(fgtiptr,"TSTOP", &tstop, comment, &status);

  if( instrumentID == XMM_RGS1 || instrumentID == XMM_RGS2 ){
    warning("XMM RGS event list do not (always?) contain GTI intervals");
    warning("Improvising by allowing a broad time range");
    tstart = 0.0;
    tstop  = 3.15e12;
    message("using tstart = ", tstart);
    message("using tstop  = ", tstop);
    status = 0;
  }
  if( status != 0 )
    return error("Error reading TSTART/TSTOP keywords!");

  if( allocate_gti( 1 ) == FAILURE ){
    fits_close_file(fgtiptr, &status);
    return error("an error occured allocating GTIs");
  }
  
  start_gti[0] = tstart;
  stop_gti[0]  = tstop;

  calc_exposure();
  if( tstart > tstop || 
      (tstop - tstart > 3.15e7 &&
       instrumentID != XMM_RGS1 && instrumentID != XMM_RGS2   ))
    return error("error in values TSTART/TSTOP keywords");

  return success();

}//make_gti()


/****************************************************************************/
//
// FUNCTION    : writeGTI
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
/****************************************************************************/
Status BasicData::writeGTI(const char fileName[] )
{
  setFunctionID("writeGTI");
  if( nGTI < 1 )
    return error("no GTI present");

  if( nGTI == 1 )
    gti->save(fileName);
  else
    for( int i= 0; i < nGTI; i++)
      gti[i].save( fileName );

  return success();
}//writeGTI()

/****************************************************************************/
//
// FUNCTION    : write_gti
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
/****************************************************************************/
Status BasicData::write_gti(char filename[], int ccdnr, bool createfile )
{
  setFunctionID("write_gti");

  if( n_gti <= 0 )
    return error("no GTI data present");

  fitsfile *fptr;
  char *ttype[2], *tunit[2], *tform[2], extname[FLEN_COMMENT]="STDGTI";
  int status= 0, tfields, hdunum,hdutype;
  long pcount=0;
  //  char comment[FLEN_COMMENT]="  ";

  if( ccdnr >= 0 ){
    if( ccdnr >= 10 )
      sprintf(extname,"%s%i","STDGTI", ccdnr);
    else
      sprintf(extname,"%s0%i","STDGTI", ccdnr);
    message("GTI extension name = ", extname);
  }//
  if( createfile == false )
    fits_open_file( &fptr, filename , READWRITE, &status );
  else
    fits_create_file( &fptr, filename , &status);

  tfields = 2;
  for(int i=0; i< tfields ; i++){
    ttype[i] = new char[40];
    tform[i] = new char[40];
    tunit[i] = new char[40];
  }//for
  int ncol=0;
  strcpy(ttype[ncol],"START");
  strcpy(tform[ncol],"1D");
  strcpy(tunit[ncol++],"s");
  strcpy(ttype[ncol],"STOP");
  strcpy(tform[ncol],"1D");
  strcpy(tunit[ncol++],"s");


  fits_get_num_hdus(fptr, &hdunum, &status);
  fits_movabs_hdu(fptr, hdunum, &hdutype,&status);
  status = 0;
  fits_insert_btbl(fptr, n_gti, ncol, ttype, tform, tunit, extname,
		   pcount, &status);
  //  write_std_keywords( fptr, false);
  fits_write_key( fptr, TSTRING,"HDUCLASS",(void *)"OGIP",
		  "/ format conforms to OGIP standar",&status);
  fits_write_key( fptr, TSTRING,"HDUCLAS1",(void *)"GTI",
		  "/ table contains Good Time Intervals",&status);
  fits_write_key( fptr, TSTRING,"HDUCLAS2",(void *)"STANDARD","",&status);
  fits_write_key( fptr,TDOUBLE, "EXPOSURE",(void *)&exposure," ", &status);
  //  fits_insert_btbl(fptr, BINARY_TBL, n_gti, tfields, ttype, tform, tunit,
  //		   "STDGTI", &status);
  if( status != 0 )
    return errorwarning("Error creating STDGTI extension.");

  ncol=0;
  fits_write_col_dbl(fptr, ++ncol, 1, 1, n_gti, start_gti, &status );
  fits_write_col_dbl(fptr, ++ncol, 1, 1, n_gti, stop_gti, &status );

  for(int i=0; i< tfields ; i++){
    delete [] ttype[i];
    delete [] tform[i];
    delete [] tunit[i];
  }//for

  fits_close_file( fptr, &status);

  return success();

}//write_GTI


/****************************************************************************/
//
// FUNCTION    : origin()
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
/****************************************************************************/
Status BasicData::origin(int argc, char *argv[])
{
  setFunctionID("origin()");

  setCreator( argv[0] );

  string history = "PARAMETERS:";
  for( int i = 0 ; i < argc ; i++)
    history += string(" ") + string(argv[i]);
  puthistorytxt( history.c_str());
  if( argc == 1 )
    setCreator( argv[0]);

  return success();
}//origin()


#if 1
/****************************************************************************/
//
// FUNCTION    : puthistorytxt()
// DESCRIPTION : BasicData allows a number of NHIST history lines to be 
//               stored. The incoming string may be HIST_LEN long and
//               will, indeed needed, spread over to history lines.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//    histtxt		I		History text
//
//  RETURNS    : void
//
/****************************************************************************/
Status BasicData::puthistorytxt(const char histtxt[] )
{

  setFunctionID("puthistorytxt");

  if( strlen(histtxt) >= HIST_LEN )
    warning("the string length is too large");

  //  strncpy( history[nhistory],histtxt, HIST_LEN );
  history.insertText(histtxt);

  nhistory++;

  return success();
}// puthistorytxt()
#endif

/****************************************************************************/
//
// FUNCTION    : print()
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  testStream          I/O                file pointer
//
//  RETURNS    : SUCCESS
//
/****************************************************************************/
Status BasicData::print( FILE *testStream ) const
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
  inserttext( bottomline," OBSERVATIONAL INFORMATION", topline );

  fprintf(testStream,"%s\n",topline);
  fprintf(testStream,"\n");
  fprintf(testStream,"\tobject       = %s\n", object);
  fprintf(testStream,"\ttelescope    = %s\n", telescope);
  fprintf(testStream,"\tinstrument   = %s\n", instrument);
  if( instrumentID >= CHANDRA_ACIS && instrumentID <= CHANDRA_LETG_HRC)
    fprintf(testStream,"\tgrating       = %s\n", grating);
  else
    fprintf(testStream,"\tfilter       = %s\n", filter);
  fprintf(testStream,"\texposure     = %.1f\n", exposure );
  fprintf(testStream,"\tstart_time   = %15f\n", start_time );
  fprintf(testStream,"\tstop_time    = %15f\n", stop_time );
  fprintf(testStream,"\tra_nom       = %e\n", ra_nom  );
  fprintf(testStream,"\tdec_nom      = %e\n", dec_nom );
  fprintf(testStream,"\troll_nom     = %e\n", roll_nom );
  fprintf(testStream,"\tchan_min     = %i\n", chanMin);
  fprintf(testStream,"\tchan_max     = %i\n", chanMax);
  fprintf(testStream,"\tobsdate      = %s\n", obsdate);
  fprintf(testStream,"\tmjd obs      = %.2f\n", mjd_obs);
  fprintf(testStream,"\n");
  fprintf(testStream,"\tbuffer size  = %li\n", buffer_size);
  fprintf(testStream,"\t#events      = %li\n", n_events );
  fprintf(testStream,"\t#GTI         = %i\n", nGTI);
  fprintf(testStream,"\tcreator      = %s\n", creator);
  
#if 1
  if( buffer_size > 0 ){
    fprintf(testStream,"\tFirst 20 events in buffer:\n");
    fprintf(testStream,
	    "\t\t     time              X          Y        Channel\n");
    for(unsigned long i = 0 ; i < MIN(20,buffer_size) ; i++){
      fprintf(testStream,"\t\t %14e  %10.2f %10.2f %6i\n", 
	      time[i],x[i], y[i], channel[i]);
    }//for
  }//if
#endif


  if( nbadpix > 0 && nbadpix<10){
    fprintf(testStream,"\tBadpixels (rawx, rawy, ccdnr)\n");
    for( unsigned long i= 0; i < MIN(nbadpix,200); i++)
      fprintf(testStream,"\t%5i %5i %3i\n", badpix[i].rawx, badpix[i].rawy,
	      badpix[i].ccdnr);
  }
  if( n_gti > 0 && n_gti <= 4){
    fprintf(testStream,"\tGood Time Intervals:\n");
    for(unsigned long i = 0 ; i < n_gti ; i++){
      fprintf(testStream,"\t\t %15f %15f\n", start_gti[i], stop_gti[i]);
    }//for
  }//if

#if 0
  for(int i = 0; i < nGTI; i++){
    fprintf(testStream,"%s\n",bottomline);
    gti[i].print(testStream);
  }
#endif
  fprintf(testStream,"%s\n",bottomline);


  history.print( testStream );
  fprintf(testStream,"%s\n",bottomline);

  return SUCCESS;
}// print()

/****************************************************************************/
//
// FUNCTION    : processtatus()
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
/****************************************************************************/
#define INFO_STRING "INFO"
void BasicData::processtatus(char message[], float percentage, 
			       Boolean start, Boolean finish)
{
  if( chatty == TRUE ){
#if 0
    if( start == FALSE )
      printf("\r");
    printf("\t%-8s - %-20s %5.1f%%",INFO_STRING, message, percentage);
#endif
#if 1
    if( start == TRUE )
      printf("\t%-8s - %-20s - %s %5.1f%%",INFO_STRING, fname, message,
	     percentage);
    else
      printf("\b\b\b\b\b\b%5.1f%%",percentage);
#endif
    if( finish == TRUE)
      printf("\n");
  }

  return;
}//processtatus()








/****************************************************************************/
//
// FUNCTION    : saveEvents
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
Status BasicData::saveEvents(const string &outputFile )
{
  setFunctionID("saveEvents");


  // some variables for using fitsio routines
  //  int status=0;
  CfitsioClass fitsFile;

  message("creating file ", outputFile );
  if( fitsFile.create( outputFile.c_str() ) != 0 ){

    warning("error creating fits file, trying to append  hdu at end of file");
    return error("event list already exists");
    //
    fitsFile.resetStatus();
    if( fitsFile.open( outputFile.c_str(), true) !=0 )
      return error("unable to append to existing file");
    int nhdu = fitsFile.numberOfHDUs();
    fitsFile.moveTo(nhdu);
    fitsFile.resetStatus();
    //
  }
  writeStdKeywords( fitsFile.fptr);
  fitsFile.writeDate();
  
  //  if(   writeFitsHeader( fitsFile.fptr) == FAILURE )
  //    return error("Error writing primary header");


  fitsFile.addColumn("time", "1D"," ");
  fitsFile.addColumn("x", "1E","pixel");
  fitsFile.addColumn("y", "1E","pixel");
  fitsFile.addColumn("PHA", "1J"," ");
  fitsFile.addColumn("ENERGY", "1E"," ");
  if( polarizationColumns ){
    fitsFile.addColumn("PE_ANGLE", "1E","radians");
    //    fitsFile.addColumn("cosa", "1E"," ");
    //    fitsFile.addColumn("sina", "1E"," ");
  }
  
  fitsFile.insertTable("EVENTS",n_events);
  fitsFile.writeDate();
  writeStdKeywords( fitsFile.fptr);
  //  writeWCSkeywords(fitsFile.fptr);
     
  int ncol=0;
  fitsFile.writeColumn(time, n_events, ++ncol);
  fitsFile.writeColumn(x, n_events, ++ncol);
  fitsFile.writeColumn(y, n_events, ++ncol);
  fitsFile.writeColumn(channel, n_events, ++ncol);
  fitsFile.writeColumn(chanEnergy, n_events, ++ncol);
  if( polarizationColumns ){
    fitsFile.writeColumn(polAngle, n_events, ++ncol);
    //    fitsFile.writeColumn(polx, n_events, ++ncol);
    //    fitsFile.writeColumn(poly, n_events, ++ncol);
  }
  
  return success();
}

