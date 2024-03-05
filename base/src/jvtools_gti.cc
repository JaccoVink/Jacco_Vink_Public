/*****************************************************************************/
//
// FILE        :  jvtools_gti.cc
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
#include "jvtools_gti.h"
#include "jvtools_messages.h"

using jvtools::GoodTimeIntervals;
using JVMessages::error;
using JVMessages::warning;
using JVMessages::message;
using JVMessages::setFunctionID;
using JVMessages::success;

/****************************************************************************/
//
// FUNCTION    : GoodTimeIntervals( long n)
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
GoodTimeIntervals::GoodTimeIntervals( long n)
{

  nGTI = 0; 
  startGTI = stopGTI = 0; 
  instrumentID = UNDEF; 
  detectorID = -1;

  if( n <= 0 )
    return;
  allocate(n);
}//


/****************************************************************************/
//
// FUNCTION    : GoodTimeIntervals( )
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
GoodTimeIntervals::GoodTimeIntervals( const GoodTimeIntervals &gti )
{
  setFunctionID("Copy Constructor[GTI]");

  if( allocate( gti.nGTI ) == FAILURE || nGTI == 0){
    error("allocation error");
    return;
  }

  instrumentID = gti.instrumentID;
  detectorID   = gti.detectorID;
  exposure     = gti.exposure;
  if( nGTI == gti.nGTI )
    for( int i = 0 ; i < nGTI; i++){
      startGTI[i] = gti.startGTI[i]; 
      stopGTI[i] = gti.stopGTI[i]; 
    }//for

}//



/****************************************************************************/
//
// FUNCTION    : GoodTimeIntervals( )
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
GoodTimeIntervals & GoodTimeIntervals::operator=( const GoodTimeIntervals &gti )
{
  setFunctionID("operator=[GTI]");

  if( allocate( gti.nGTI ) == FAILURE){
    error("allocation error");
    return *this;
  }

  instrumentID = gti.instrumentID;
  detectorID   = gti.detectorID;
  exposure = gti.exposure;
  if( nGTI == gti.nGTI )
    for( int i = 0 ; i < nGTI; i++){
      startGTI[i] = gti.startGTI[i]; 
      stopGTI[i] = gti.stopGTI[i]; 
    }//for


  return *this;
}//


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
//  RETURNS    :
//
/****************************************************************************/
Status GoodTimeIntervals::allocate( long n)
{

  setFunctionID("allocate[GTI]");

  if( n <= 0 )
    return error("wrong parameter value");

  if( nGTI != 0 && nGTI != n)
    deallocate();

  if( nGTI == 0 ){
    startGTI = new double [n];
    if( startGTI == 0) return FAILURE;
    
    stopGTI  = new double [n];
    if( stopGTI == 0) return FAILURE;
  
    nGTI = n;
  }//if

  for( long i = 0; i < nGTI ; i++) startGTI[i] = stopGTI[i] = 0.0;

  return success();
}//allocate


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
//  RETURNS    :
//
/****************************************************************************/
Status GoodTimeIntervals::deallocate( void )
{

  setFunctionID("deallocate[GTI]");

  if( nGTI != 0 ){
    delete [] startGTI;
    delete [] stopGTI;
    nGTI = 0;
  }//if

  return success();
}//deallocate()

/****************************************************************************/
//
// FUNCTION    : setInstrumentID
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
Status GoodTimeIntervals::setInstrumentID( Instrument instr )
{
  instrumentID = instr;
  return SUCCESS;
}//setInstrumentID


/****************************************************************************/
//
// FUNCTION    : totalExposure()
// DESCRIPTION : Calculates sume of all GTI
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : total exposure time
//
/****************************************************************************/
double GoodTimeIntervals::totalExposure()
{

  exposure = 0.0;
  for( long int i = 0; i < nGTI ; i++)
    exposure += stopGTI[i] - startGTI[i];

  return exposure;
}//totalExposure()


/****************************************************************************/
//
// FUNCTION    : cumulativeExposure()
// DESCRIPTION : Calculates sum of all GTI's up to time = t
// 
//
// REMARKS     : assumes time order GTI
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : total exposure time
//
/****************************************************************************/
double GoodTimeIntervals::cumulativeExposure(double endTime)
{

  exposure = 0.0;
  for( long int i = 0; i < nGTI ; i++){
    if( stopGTI[i] <= endTime )
      exposure += stopGTI[i] - startGTI[i];
    else if( startGTI[i] < endTime )
      exposure += endTime - startGTI[i];
  }

  return exposure;
}//totalExposure()

/****************************************************************************/
//
// FUNCTION    : excludeInterval()
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
Status GoodTimeIntervals::excludeInterval( double tbegin, double tend )
{
  setFunctionID("excludeInterval[GTI]");

  if( nGTI == 0 || tend <= tbegin)
    return error("No GTI present or invalid parameters");

  GoodTimeIntervals newGTI;

  if( newGTI.allocate( nGTI + 2) == FAILURE )
    return error("allocation error");

  newGTI.instrumentID = instrumentID;
  newGTI.detectorID   = detectorID;
  
  // step 1 adapt start and stop times to tbegin and tend
  unsigned long n=0;
  for( long i=0; i < nGTI ; i++){
    if( tbegin > stopGTI[i] || tend < startGTI[i]){
      newGTI.startGTI[n] = startGTI[i];
      newGTI.stopGTI[n]  = stopGTI[i];
      if( newGTI.stopGTI[n] > newGTI.startGTI[n]) n++;
    }
    else if( tbegin > startGTI[i] && tbegin < stopGTI[i] && 
	     tend >= stopGTI[i] ){
      newGTI.startGTI[n] = startGTI[i];
      newGTI.stopGTI[n] = tbegin;
      if( newGTI.stopGTI[n] > newGTI.startGTI[n]) n++;
    }
    else if( tbegin > startGTI[i] && tbegin < stopGTI[i] && 
	     tend <= stopGTI[i] ){
      newGTI.startGTI[n] = startGTI[i];
      newGTI.stopGTI[n] = tbegin;
      if( newGTI.stopGTI[n] > newGTI.startGTI[n]) n++;
      newGTI.startGTI[n] = tend;
      newGTI.stopGTI[n]  = stopGTI[i];
      if( newGTI.stopGTI[n] > newGTI.startGTI[n]) n++;
    }
    else if( tbegin < startGTI[i] &&
	     tend >= startGTI[i] && tend < stopGTI[i] ){
      newGTI.startGTI[n] = tend;
      newGTI.stopGTI[n]  = stopGTI[i];
      if( newGTI.stopGTI[n] > newGTI.startGTI[n]) n++;
    }//else
    //message("skipping GTI");
  }//for

  //message("changing GTI");
  
  if( n<= 0){
    newGTI.startGTI[0] = 0.0;
    newGTI.stopGTI[0]  = 0.0;
    n=1;
  }

  newGTI.nGTI = n;

  *this = newGTI;

  return success();
}//excludeInterval()

/****************************************************************************/
//
// FUNCTION    : read()
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
Status GoodTimeIntervals::read( const char filename[], int detectorNumber )
{
  setFunctionID("read[GTI]");

  char gtiExtension[12] = "STDGTI";

  detectorID = detectorNumber;

  if( detectorNumber >= 0 && 
      (instrumentID == XMM_PN || instrumentID == XMM_MOS1 || 
      instrumentID == XMM_MOS2 || instrumentID == XMM_RGS1 || 
       instrumentID == XMM_RGS2) ){
    if( detectorNumber > 9 )
      sprintf(gtiExtension,"%s%i","STDGTI", detectorNumber);
    else
      sprintf(gtiExtension,"%s0%i","STDGTI", detectorNumber);
  }
  else{
    if( instrumentID == CHANDRA_ACIS ){
      sprintf(gtiExtension,"%s%i","GTI", detectorNumber);
      message("found Chandra ACIS GTI for CCD ", (long)detectorNumber);
      message("HDU Name ", gtiExtension);
    }
    else if( instrumentID == EINSTEIN_HRI )
      strcpy(gtiExtension,"GTI");
    else if( instrumentID == SAX_ME )
      strcpy(gtiExtension,"ALLGTI");
  }//if


  fitsfile *fitsPtr = 0;
  int status=0;

  fits_open_file( &fitsPtr, filename , READONLY, &status );
  if( status != 0 )
    return error("unable to open file");


  // move to the right HDU
  fits_movnam_hdu( fitsPtr, BINARY_TBL, gtiExtension, 0, &status);

  if( status != 0 ){
    warning("unable to move to GTI extension ",gtiExtension);
    warning("try \"GTI\"");
    status = 0;
    fits_movnam_hdu( fitsPtr, BINARY_TBL, "GTI", 0, &status);
    if( status != 0 ){
      warning("unable to move to GTI extension, try \"STDGTI\"");
      status = 0;
      fits_movnam_hdu( fitsPtr, BINARY_TBL, "STDGTI", 0, &status);
    }
    if( status != 0 ){
      warning("unable to move to GTI extension, try \"ALLGTI\"");
      status = 0;
      fits_movnam_hdu( fitsPtr, BINARY_TBL, "ALLGTI", 0, &status);
    }
    if(status != 0 ){
      allocate(1);
      fits_close_file(fitsPtr, &status);
      return  error("unable to move to GTI extension", status);
    }
    message("Successfully read GTI for detector no ", detectorNumber);
  }//if

  //  message(gtiExtension);


  char comment[FLEN_COMMENT];
  long naxis1, naxis2, tfields;
  fits_read_key_lng(fitsPtr, "NAXIS1", &naxis1, comment, &status);
  fits_read_key_lng(fitsPtr, "NAXIS2", &naxis2, comment, &status);
  fits_read_key_lng(fitsPtr, "TFIELDS", &tfields, comment, &status);

  if( tfields < 2 || naxis2 < 1 || status != 0 ){
    fits_close_file(fitsPtr, &status);
    return error("TFIELDS should be >= 2!");
  }//if

  if( allocate( naxis2 ) == FAILURE ){
    fits_close_file(fitsPtr, &status);
    return error("An error occured allocating GTI memory!");
  }//if

  int anynull;
  long firstrow=1, firstelem=1;
  fits_read_col_dbl(fitsPtr, 1, firstrow, firstelem, naxis2 , 0.0, 
		    startGTI, &anynull, &status );
  fits_read_col_dbl(fitsPtr, 2, firstrow, firstelem, naxis2, 0.0, 
		    stopGTI, &anynull, &status );

  if( status != 0 ){
    deallocate();
    fits_close_file( fitsPtr, &status);
    return error("An error occured reading the GTI");
  }//if
  fits_close_file(fitsPtr, &status);


  return success();
}//read()


/****************************************************************************/
//
// FUNCTION    : save()
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
Status GoodTimeIntervals::save( const char fileName[] )
{

  setFunctionID("save[GTI]");


  fitsfile *fitsPtr = 0;
  int status=0;
  
  fits_open_file( &fitsPtr, fileName , READWRITE, &status );
  if( status != 0 ){
    warning("unable to open existing file, creating new file");
    status = 0;
    fits_create_file( &fitsPtr, fileName , &status );
  }//if


  char *ttype[2], *tunit[2], *tform[2];
  int tfields, hdunum,hdutype;
  long pcount=0;

  char gtiExtension[20] = "STDGTI";

  if( detectorID >= 0 ){
    if( detectorID > 9 )
      sprintf(gtiExtension,"%s%i","STDGTI", detectorID);
    else
      sprintf(gtiExtension,"%s0%i","STDGTI", detectorID);
  }//if


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


  fits_get_num_hdus(fitsPtr, &hdunum, &status);
  fits_movabs_hdu(fitsPtr, hdunum, &hdutype,&status);
  status = 0;
  fits_insert_btbl(fitsPtr, nGTI, ncol, ttype, tform, tunit, gtiExtension,
		   pcount, &status);
  //  write_std_keywords( fitsPtr, false);
  fits_write_key( fitsPtr, TSTRING,"HDUCLASS",(void *)"OGIP",
		  "/ format conforms to OGIP standar",&status);
  fits_write_key( fitsPtr, TSTRING,"HDUCLAS1",(void *)"GTI",
		  "/ table contains Good Time Intervals",&status);
  fits_write_key( fitsPtr, TSTRING,"HDUCLAS2",(void *)"STANDARD","",&status);
  fits_write_key( fitsPtr,TDOUBLE, "EXPOSURE",(void *)&exposure," ", &status);
  //  fits_insert_btbl(fitsPtr, BINARY_TBL, n_gti, tfields, ttype, tform, tunit,
  //		   "STDGTI", &status);
  if( status != 0 )
    return error("error creating STDGTI extension.");

  ncol=0;
  fits_write_col_dbl(fitsPtr, ++ncol, 1, 1, nGTI, startGTI, &status );
  fits_write_col_dbl(fitsPtr, ++ncol, 1, 1, nGTI, stopGTI, &status );

  for(int i=0; i< tfields ; i++){
    delete [] ttype[i];
    delete [] tform[i];
    delete [] tunit[i];
  }//for

  fits_close_file( fitsPtr, &status);


  return success();
}//save()


/****************************************************************************/
//
// FUNCTION    : print()
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
Status GoodTimeIntervals::print( FILE *fp )
{
  
  fprintf(fp, "INSTRUMENT ID = %i\n", (int)instrumentID);
  fprintf(fp, "DETECTOR   ID = %i\n", detectorID);
  fprintf(fp, "Number of intervals = %li\n", nGTI);
  fprintf(fp, "\t       START          STOP\n");
  for( int i = 0; i < nGTI; i++)
    fprintf(fp, "\t%4i %e %e\n", i, startGTI[i], stopGTI[i] );

  return SUCCESS;
}//print()






