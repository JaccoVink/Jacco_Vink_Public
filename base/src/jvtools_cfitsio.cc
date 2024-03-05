/*****************************************************************************/
//
// FILE        :  jvtools_cfitsio.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  CFITSIO wrapper
//
//
// COPYRIGHT   : Jacco Vink, SRON, 2004
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
// 
//
/*****************************************************************************/
#include "jvtools_cfitsio.h"


using jvtools::CfitsioClass;
using namespace::JVMessages;

/****************************************************************************/
//
// FUNCTION    : CfitsioClass()/~CfitsioClass()
// DESCRIPTION : Constructor/destructor
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
CfitsioClass::CfitsioClass()
{
  fptr    = 0;
  naxes[0]= 0;
  naxes[1]= 0;
  naxis1  = 0;
  naxis2  = 0;
  pcount  = 0;
  status  = 0;
  hdutype = BINARY_TBL;
  anynull = 0;
  ncolumns = 0;

  //  extname = strncpy(extname,"NO_NAME", FLEN_VALUE);
  return;
}
CfitsioClass::~CfitsioClass()
{

  deleteColumns();
  close();
  
  return;
}//destructor


/****************************************************************************/
//
// FUNCTION    : deleteColumns()
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
Status CfitsioClass::deleteColumns()
{
  if( ncolumns > 0 ){
    for(int i=0; i< ncolumns ; i++){
      delete [] ttype[i];
      delete [] tform[i];
      delete [] tunit[i];
      ttype[i] = 0;
      tform[i] = 0;
      tunit[i] = 0;
    }//for
    ncolumns = 0;
  }//if
  return SUCCESS;
}//

/****************************************************************************/
//
// FUNCTION    : statusReturn
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
int CfitsioClass::statusReturn( void )
{
  if( status != 0 )
    warning("Fits I/O status = ", status);

  return status;
}//statusReturn
/****************************************************************************/
//
// FUNCTION    : statusReturn
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
int CfitsioClass::statusReturn( char *keyWord )
{
  if( status != 0 ){
    warning("Fits I/O error for keyword/column ", keyWord);
    warning("Fits I/O status = ", status);
  }

  return status;
}//statusReturn


/****************************************************************************/
//
// FUNCTION    : resetStatus()
// DESCRIPTION : Resets the status value to zero.
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
void CfitsioClass::resetStatus( void )
{
  status = 0;
}//


/****************************************************************************/
//
// FUNCTION    : open/create()
// DESCRIPTION : Opening existing, or creating new fits file
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
int CfitsioClass::open( const char fitsFileName[], bool forOutput )
{
  int iomode=READONLY;

  if( forOutput){
    warning("File will be opened for read and write...");
    iomode=READWRITE;
  }

  fits_open_file( &fptr, fitsFileName , iomode, &status );

  if( status == 0 )
    getNaxes();
  return statusReturn();
}//open
int CfitsioClass::create( const char fitsFileName[] )
{

  fits_create_file( &fptr, fitsFileName , &status );
  naxis=0; 
  naxes[0]=0;
  naxes[1]=0; 
  pcount=0;
  naxis2=0;
  deleteColumns();

  fits_write_imghdr( fptr, 8, naxis, naxes , &status );

  if( status != 0 )
    warning("error creating fits file");
  return statusReturn();
}//create

/****************************************************************************/
//
// FUNCTION    : close()
// DESCRIPTION : Close the fits file.
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
int CfitsioClass::close()
{
  setFunctionID("CfitsioClass::close");
  if( fptr != 0){
    fits_close_file(fptr, &status);
    //    message("fits file closed");
    fptr = 0;
  }//else
  //message("no fitsfile open");
  success();
  return status;
}//close

/****************************************************************************/
//
// FUNCTION    : moveTo()
// DESCRIPTION : Move to HDU with name "hduName" or hdunr
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : fits I/O status
//
/****************************************************************************/
int CfitsioClass::moveTo( char hduName[])
{
  fits_movnam_hdu(fptr, hdutype, hduName, 0, &status);

  if( status == 0 )
    getNaxes();
  else
    warning("unable to move to HDU extension ", hduName);

  return statusReturn();
}//
int CfitsioClass::moveTo( int hduNr )
{
  fits_movabs_hdu(fptr, hduNr, 0, &status);

  if( status == 0 )
    getNaxes();
  else
    warning("unable to move to HDU number ", hduNr);

  return statusReturn();
}//


/****************************************************************************/
//
// FUNCTION    :   numberOfHDUs
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : number of HDUs
//
/****************************************************************************/
int CfitsioClass::numberOfHDUs()
{

  int nrhdus = 0;

  fits_get_num_hdus(fptr, &nrhdus,&status);

  statusReturn();

  return nrhdus;
}//numberOfHDUs()



/****************************************************************************/
//
// FUNCTION    : getNaxes( void )
// DESCRIPTION : reads the naxis1 and naxis2 keywords
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : fitsio status
//
/****************************************************************************/
int CfitsioClass::getNaxes( void )
{

  fits_read_key(fptr, TINT, "NAXIS", &naxis, comment, &status);
  if( status != 0){
    warning("error reading NAXIS keyword, status=", status);
    return statusReturn();
  }

  if( naxis > 0 ){
    fits_read_key_lng(fptr,"NAXIS1", &naxis1, comment, &status);
  }
  else
    naxis1 = 0;

  if( naxis > 1 ){
      fits_read_key_lng(fptr,"NAXIS2", &naxis2, comment, &status);
  }
  else
    naxis2 = 0;
  

  if( status != 0)
    warning("error reading naxis1 & naxis2 keywords, status=", status);

  return statusReturn();
}//getNaxes()

/****************************************************************************/
//
// FUNCTION    : numberOfColumns
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : ncolumns
//
/****************************************************************************/
int CfitsioClass::numberOfColumns( void ){

  int ncols=0;

  fits_get_num_cols(fptr, &ncols, &status);
  if( status != 0){
    JVMessages::error("error status = ", status);
  }
  return ncols;
};
long CfitsioClass::numberOfRows( void ){
  long nrows=0;
  fits_get_num_rows(fptr, &nrows, &status);
  if( status != 0){
    error("error status = ", status);
  }
  return nrows;
};


/****************************************************************************/
//
// FUNCTION    : readKey()
// DESCRIPTION : read keyword, character version
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
const char * CfitsioClass::readKey(  char keyName[])
{
  int tempStatus=0;

  fits_read_key_str( fptr, keyName , keyValue, comment, &tempStatus);
  if( tempStatus != 0 )
    status = tempStatus;

  if( status != 0)
    strncpy(keyValue, "none", FLEN_VALUE);

  statusReturn();
  return keyValue;
}// readKey
Status  CfitsioClass::readKey(  char keyName[], char *newkeyValue, int n)
{
  setFunctionID("readKey");

  if( n < 2 )
    return error("invalid input string length");

  int tempStatus=0;
  fits_read_key_str( fptr, keyName , keyValue, comment, &tempStatus);
  if( tempStatus != 0 )
    status = tempStatus;

  if( status != 0){
    warning("could not obtain value for keyword ", keyName);
    strncpy(keyValue, "none", FLEN_VALUE-1);//leave it untouched
  }
  else
    strncpy( newkeyValue,  keyValue, n-1);

  statusReturn();
  return success();
}//readKey(

/****************************************************************************/
//
// FUNCTION    : readKeyDbl()/readKeyFlt()/readKeyInt()
// DESCRIPTION : read keyword, double/float/in versions
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
double CfitsioClass::readKeyDbl( char keyName[], double nullValue)
{
  double value=nullValue;
  int tempStatus = 0;

  fits_read_key_dbl( fptr, keyName , &value, comment, &tempStatus);
  if( tempStatus != 0 )
    status = tempStatus;

  statusReturn();

  return value;
}// readKey
float CfitsioClass::readKeyFlt( char keyName[], float nullValue)
{
  float value= nullValue;
  int tempStatus = 0;

  fits_read_key_flt( fptr, keyName , &value, comment, &tempStatus);
  if( tempStatus != 0 )
    status = tempStatus;

  statusReturn();

  return value;
}// readKey
 
int CfitsioClass::readKeyInt( char keyName[], int nullValue)
{
  int value = nullValue;
  int tempStatus=0;

  fits_read_key(fptr,TINT, keyName,(void *)&value, comment, &tempStatus);
  if( tempStatus != 0 )
    status = tempStatus;

  statusReturn();

  return value;
}// readKey
 

/****************************************************************************/
//
// FUNCTION    : writeKey()
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
int CfitsioClass::writeKey( const char keyName[], const char value[], 
			    const char newComment[] )
{
  //  setFunctionID("writeKey");

  strncpy( keyWord, keyName, FLEN_KEYWORD);
  strncpy( keyValue, value, FLEN_VALUE);
  strncpy( comment, newComment, FLEN_COMMENT);

  fits_update_key_str(fptr, keyWord, keyValue, comment, &status);

  if( status != 0 )
    warning("Error updating/writing keyword. Status = ", status);

  return statusReturn();
}//writeKey
int CfitsioClass::writeKey( const char keyName[], long value, 
			    const char newComment[] )
{
  strncpy( keyWord, keyName, FLEN_KEYWORD);
  strncpy( comment, newComment, FLEN_COMMENT);
  fits_write_key(fptr, TLONG, keyWord, (long *)&value, comment, &status);
  return statusReturn();
}//writeKey
int CfitsioClass::writeKey( const char keyName[], float value, 
			    const char newComment[] )
{
  strncpy( keyWord, keyName, FLEN_KEYWORD);
  strncpy( comment, newComment, FLEN_COMMENT);

  if( (value == 0.0) || ( (fabs(value) > 1.0e-3) && (fabs(value) < 1.0e6) ))
    //  if( value == 0.0 || fabs(value) > 1.0e-3 && fabs(value) < 1.0e6)
    fits_write_key(fptr, TFLOAT, keyWord, (float *)&value, comment, &status);
  else
    fits_write_key_dbl(fptr, keyWord, value, 6,comment,&status);


  return statusReturn();
}//writeKey
int CfitsioClass::writeKey( const char keyName[], double value, 
			    const char newComment[] )
{
  strncpy( keyWord, keyName, FLEN_KEYWORD);
  strncpy( comment, newComment, FLEN_COMMENT);
  if( value == 0.0 || fabs(value) > 1.0e-3 && fabs(value) < 1.0e6)
    fits_write_key(fptr, TDOUBLE, keyWord, (double *)&value, comment, &status);
  else
    fits_write_key_dbl(fptr, keyWord, value, 7,comment,&status);

  return statusReturn();
}//writeKey

int CfitsioClass::writeKey( const char keyName[], bool value,
			    const char newComment[] )
{
  strncpy( keyWord, keyName, FLEN_KEYWORD);
  strncpy( comment, newComment, FLEN_COMMENT);
  if( value == true)
    fits_write_key(fptr, TLOGICAL, keyWord, (void *)&value, comment, &status);
  else
    fits_write_key(fptr, TLOGICAL, keyWord, (void *)&value, comment, &status);

  return statusReturn();
}//writeKey





/****************************************************************************/
//
// FUNCTION    : addColum()
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
Status CfitsioClass::addColumn(const char type[], 
			       const char form[], 
			       const char unit[])
{
  setFunctionID("addColumn");

  if( ncolumns == N_MAX_COLUMN-1 )
    return error("maximum number of columns used");
  
  int i = ncolumns;
  ttype[i] = new char[FLEN_VALUE];
  tform[i] = new char[FLEN_VALUE];
  tunit[i] = new char[FLEN_VALUE];

  strncpy(ttype[i], type, FLEN_VALUE);
  strncpy(tform[i], form , FLEN_VALUE);
  strncpy(tunit[i], unit, FLEN_VALUE);

  ncolumns++;

  return success();
}//addColumn()


/****************************************************************************/
//
// FUNCTION    : writeColumn()
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
int CfitsioClass::writeColumn( double *column, long ndata, int colnum, 
			       int firstRow)
{
  setFunctionID("writeColumn[double]");


  if(  colnum < 1 || colnum > ncolumns ){
    error("invalid column number");
    return -1;
  }
  if(  firstRow-1+ndata > naxis2 ){
    error("column too long");
    return -1;
  }//if
  if( strncmp(tform[colnum-1], "1D", FLEN_VALUE) != 0 &&
      strncmp(tform[colnum-1], "D", FLEN_VALUE) != 0 ){
    message("colnum = ", colnum);
    error("column does not have the right format (1D=double) instead of ",
	  tform[colnum-1] );
    return -1;
  }//if

  fits_write_col_dbl(fptr, colnum, firstRow, 1, ndata, column, &status );
  
  if( status != 0)
    return statusReturn();

  success();
  return status;
}
int CfitsioClass::writeColumn( float *column, long ndata, int colnum, 
			       int firstRow)
{
  setFunctionID("writeColumn[float]");


  if(  colnum < 1 || colnum > ncolumns ){
    error("invalid column number");
    return -1;
  }
  if(  strncmp(tform[colnum-1], "1PE(", 4) != 0  &&
       firstRow-1+ndata > naxis2 ){
    error("column too long");
    return -1;
  }//if
  if( strncmp(tform[colnum-1], "1E", FLEN_VALUE) != 0 &&
      strncmp(tform[colnum-1], "E", FLEN_VALUE) != 0 &&
      strncmp(tform[colnum-1], "1PE(", 4) != 0 ){
    message("colnum = ", colnum);
    error("column does not have the right format (1E=float) instead of ",
	  tform[colnum-1] );
    return -1;
  }//if

  fits_write_col_flt(fptr, colnum, firstRow, 1, ndata, column, &status );
  
  if( status != 0)
    return statusReturn();

  success();
  return status;
}
int CfitsioClass::writeColumn( unsigned long *column, long ndata, int colnum, 
			       int firstRow)
{
  setFunctionID("writeColumn[unsigned long");


  if(  colnum < 1 || colnum > ncolumns ){
    error("invalid column number");
    return -1;
  }
  if(  firstRow-1+ndata > naxis2 ){
    error("column too long");
    return -1;
  }//if
  if( strncmp(tform[colnum-1], "1J", FLEN_VALUE) != 0 &&
       strncmp(tform[colnum-1], "J", FLEN_VALUE) != 0 ){
    message("colnum = ", colnum);
    error("column does not have the right format (1J=long) instead of ",
	  tform[colnum-1] );
    return -1;
  }//if

  fits_write_col_ulng(fptr, colnum, firstRow, 1, ndata, column, &status );
  
  if( status != 0)
    return statusReturn();

  success();
  return status;
}

int CfitsioClass::writeColumn( long *column, long ndata, int colnum, 
			       int firstRow)
{
  setFunctionID("writeColumn[long]");


  if(  colnum < 1 || colnum > ncolumns ){
    error("invalid column number");
    return -1;
  }
  if(  firstRow-1+ndata > naxis2 ){
    error("column too long");
    return -1;
  }//if
  if( strncmp(tform[colnum-1], "1J", FLEN_VALUE) != 0 &&
       strncmp(tform[colnum-1], "J", FLEN_VALUE) != 0 ){
    message("colnum = ", colnum);
    error("column does not have the right format (1J=long) instead of ",
	  tform[colnum-1] );
    return -1;
  }//if

  fits_write_col_lng(fptr, colnum, firstRow, 1, ndata, column, &status );
  
  if( status != 0)
    return statusReturn();

  success();
  return status;
}
int CfitsioClass::writeColumn( int *column, long ndata, int colnum, 
			       int firstRow)
{
  setFunctionID("writeColumn[int]");


  if(  colnum < 1 || colnum > ncolumns ){
    error("invalid column number");
    return -1;
  }
  if(  firstRow-1+ndata > naxis2 ){
    error("column too long");
    return -1;
  }//if
  if( strncmp(tform[colnum-1], "1J", FLEN_VALUE) != 0 &&
       strncmp(tform[colnum-1], "J", FLEN_VALUE) != 0 ){
    message("colnum = ", colnum);
    error("column does not have the right format (1J=long) instead of ",
	  tform[colnum-1] );
    return -1;
  }//if

  fits_write_col_int(fptr, colnum, firstRow, 1, ndata, column, &status );
  
  if( status != 0)
    return statusReturn();

  success();
  return status;
}
int CfitsioClass::writeColumn( short *column, long ndata, int colnum, 
			       int firstRow)
{
  setFunctionID("writeColumn");


  if(  colnum < 1 || colnum > ncolumns ){
    error("invalid column number");
    return -1;
  }
  if(  firstRow-1+ndata > naxis2 ){
    error("column too long");
    return -1;
  }//if
  if( strncmp(tform[colnum-1], "1I", FLEN_VALUE) != 0 &&
       strncmp(tform[colnum-1], "I", FLEN_VALUE) != 0 ){
    message("colnum = ", colnum);
    error("column does not have the right format (1I=long) instead of ",
	  tform[colnum-1]);
    return -1;
  }//if

  fits_write_col_sht(fptr, colnum, firstRow, 1, ndata, column, &status );
  
  if( status != 0)
    return statusReturn();

  success();
  return status;
}
int CfitsioClass::writeColumn( string *column, long ndata, int colnum, 
			       int firstRow)
{
  setFunctionID("writeColumn[string]");


  if(  colnum < 1 || colnum > ncolumns ){
    error("invalid column number");
    return -1;
  }
  if(  firstRow-1+ndata > naxis2 ){
    error("column too long");
    return -1;
  }//if
#if 0
  if( strncmp(tform[colnum-1], "1A", FLEN_VALUE) != 0 &&
       strncmp(tform[colnum-1], "A", FLEN_VALUE) != 0 ){
    message("colnum = ", colnum);
    error("column does not have the right format (1D=double) instead of ",
	  tform[colnum-1] );
    return -1;
  }//if
#endif

  int strl;
  sscanf( tform[colnum-1],"%iA", &strl);
  //  message("string size = ", strl);
  if( strl <= 0 )
    return error("incorrect string size");
  
  char *str = new char [strl+1];
  //  for( int i = firstRow-1; i < ndata; i++){
  for( int i = 0; i < ndata; i++){
    strncpy(str, column[i].c_str(), strl+1);
    //    puts(str);
    fits_write_col_str(fptr, colnum, firstRow+i, 1, 1, &str, &status );
    //    printf("i=%i %s status = %i\n", i, str, status);
  }
  
  delete [] str;
  if( status != 0)
    return statusReturn();

  success();
  return status;
}



/****************************************************************************/
//
// FUNCTION    : insertTable()
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
int CfitsioClass::insertTable( char hduName[], int nrows)
{

  setFunctionID("insertTable");

  if( ncolumns < 1 )
    return error("no columns to write");

  /*
  message("inserting fits table");
  message("HDU name = ", hduName);
  message("#columns = ",ncolumns);
  message("#rows = ",nrows);
  message("status = ", status);

  for(int i=0; i< ncolumns; i++)
    printf("%i %s  %s\n", i, ttype[i], tunit[i]);

    for(int i=0; i < ncolumns; i++)
    puts(ttype[i]);
  */

  fits_insert_btbl(fptr, nrows, ncolumns, ttype, tform, tunit,
		   hduName, pcount, &status);




  naxis2 = nrows;
  if( status != 0 )
    return statusReturn();

  success();
  return status;
}//insertTable

/****************************************************************************/
//
// FUNCTION    : columnExists
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : true or false
//
/****************************************************************************/
bool CfitsioClass::columnExists(char columnName[])
{
  int colNum= -1, newstatus=0;

  fits_get_colnum(fptr, FALSE, columnName , &colNum, &newstatus);
  if( colNum > 0 && newstatus == 0)
    return true;
  else
    return false;
}// columnExists

/****************************************************************************/
//
// FUNCTION    : readColumn (overloaded)
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
int CfitsioClass::readColumn( char columnName[], long *column, long size, 
			       long firstRow, long firstElement)
{
  int colNum=0;

  if( size < 1 || column == 0){
    warning("invalid column size, or array");
    status = -1;
    return statusReturn();
  }
  
  fits_get_colnum(fptr, FALSE, columnName , &colNum, &status);
  if( status == 0 )
    fits_read_col( fptr, TLONG, colNum, firstRow, firstElement,
		       size, 0 , &column[0], &anynull, &status );
  else
    warning("Could not find column ", columnName);

  return statusReturn();
}// readColumn()
int CfitsioClass::readColumn( char columnName[], double *column, long size, 
			       long firstRow, long firstElement)
{
  int colNum=0;

  if( size < 1 || column == 0){
    warning("invalid column size, or array");
    status = -1;
    return statusReturn();
  }
  
  fits_get_colnum(fptr, FALSE, columnName , &colNum, &status);
  if( status == 0 )
    fits_read_col( fptr, TDOUBLE, colNum, firstRow, firstElement,
		   size, 0 , &column[0], &anynull, &status );
  else
    warning("Could not find column ", columnName);

  return statusReturn();
}// readColumn()
int CfitsioClass::readColumn( char columnName[], float *column, long size, 
			       long firstRow, long firstElement)
{
  int colNum=0;

  if( size < 1 || column == 0){
    warning("invalid column size, or array");
    status = -1;
    return statusReturn();
  }
  
  fits_get_colnum(fptr, FALSE, columnName , &colNum, &status);
  if( status == 0 )
    fits_read_col( fptr, TFLOAT, colNum, firstRow, firstElement,
		   size, 0 , &column[0], &anynull, &status );
  else
    warning("Could not find column ", columnName);

  return statusReturn();
}// readColumn()
int CfitsioClass::readColumn( char columnName[], short *column, long size, 
			       long firstRow, long firstElement)
{
  int colNum=0;

  if( size < 1 || column == 0){
    warning("invalid column size, or array");
    status = -1;
    return statusReturn();
  }
  
  fits_get_colnum(fptr, FALSE, columnName , &colNum, &status);
  if( status == 0 )
    fits_read_col( fptr, TSHORT, colNum, firstRow, firstElement,
		       size, 0 , &column[0], &anynull, &status );
  else
    warning("Could not find column ", columnName);

  return statusReturn();
}// readColumn()
int CfitsioClass::readColumn( char columnName[], unsigned short *column, 
			      long size, long firstRow, long firstElement)
{
  int colNum=0;

  if( size < 1 || column == 0){
    warning("invalid column size, or array");
    status = -1;
    return statusReturn();
  }
  
  fits_get_colnum(fptr, FALSE, columnName , &colNum, &status);
  if( status == 0 )
    fits_read_col( fptr, TUSHORT, colNum, firstRow, firstElement,
		       size, 0 , &column[0], &anynull, &status );
  else
    warning("Could not find column ", columnName);

  return statusReturn();
}// readColumn()
int CfitsioClass::readColumn( char columnName[], char **column, 
			      long size, long firstRow, long firstElement)
{
  int colNum=0;

  if( size < 1 || column == 0){
    warning("invalid column size, or array");
    status = -1;
    return statusReturn();
  }
  
  fits_get_colnum(fptr, FALSE, columnName , &colNum, &status);
  if( status == 0 )
    fits_read_col( fptr, TSTRING, colNum, firstRow, firstElement,
		       size, 0 , column, &anynull, &status );
  else
    warning("Could not find column ", columnName);

  return statusReturn();
}// readColumn()


/****************************************************************************/
//
// FUNCTION    :   getDimColumn
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
int CfitsioClass::getDimColumn( char columnName[] )
{
  int colNum, dim = 0;;

  fits_get_colnum(fptr, FALSE, columnName , &colNum, &status);
  
  if( status == 0 ){
    char tform[FLEN_VALUE], tvalue[FLEN_VALUE];
    sprintf(tform,"TFORM%i",colNum);
    readKey(  tform, tvalue);
    //    message("read keyword  = ", tform);
    //    message("keyword value = ", tvalue);
    if( strlen(tvalue) > 1 ){
      tvalue[strlen(tvalue)-1] = 0;
      dim = atoi( tvalue );
    }
    resetStatus();
    //    message("col = ", colNum);
  }else
    warning("Could not find column ", columnName);
  
  return dim;
}//getDimColumn()
