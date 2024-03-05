#ifndef __JVTOOLS_CFITSIO__H__
#define __JVTOOLS_CFITSIO__H__

/*****************************************************************************/
//
// FILE        :  jvtools_basic.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  X-ray astronomy base class.
//
//
// COPYRIGHT   : Jacco Vink,  1999, 2002
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <string>
//#include <values.h>
#include "fitsio.h"
#include "common.h"
#include "jvtools_messages.h"
//#include "nr.h"


#define N_MAX_COLUMN  128

namespace jvtools{
  class CfitsioClass
  {
  public:
    int status;
    int ncolumns;
    int hdutype, anynull;
    int naxis;
    long naxes[2];
    long naxis1, naxis2, pcount;
    char *ttype[N_MAX_COLUMN], *tunit[N_MAX_COLUMN], *tform[N_MAX_COLUMN];
    char keyWord[FLEN_KEYWORD], keyValue[FLEN_VALUE];
    char comment[FLEN_COMMENT], extname[FLEN_VALUE];
    

    fitsfile *fptr;

    CfitsioClass();
    ~CfitsioClass();
    Status deleteColumns();
    int statusReturn( void );
    int statusReturn( char *keyWord );
    void resetStatus( void );
    int open( const char fitsFileName[], bool forOutput=false);
    int open( const string fitsFileName, bool forOutput=false){
      return open( fitsFileName.c_str(), forOutput);
    };
    int create( const char fitsFileName[]);
    int close( void );
    int moveTo( char hduName[]);
    int moveTo( int hduNr );
    int numberOfHDUs( void );
    int numberOfColumns( void );
    long numberOfRows( void );
    int getNaxes();

    int getDimColumn( char columnName[] );
    const char * readKey(  char keyName[] );
    Status readKey(  char keyName[], char *newkeyValue, int n=FLEN_VALUE);
    double readKeyDbl(  char keyName[], double nullValue=0.0 );
    float  readKeyFlt(  char keyName[], float nullValue=0.0 );
    int    readKeyInt(  char keyName[], int nullValue=0 );
    int writeDate(void){
      fits_write_date(fptr, &status);
      return statusReturn();
    };
    Status addColumn(const char type[], const char form[], 
		     const char unit[]);
    int insertTable( char hduName[], int nrows);

    int writeKey( const char keyName[], 
		  const char value[], const char newComment[]=" " );
    int writeKey( const char keyName[], 
		  long value, const char newComment[]=" " );
    int writeKey( const char keyName[], int value,
    		  const char newComment[]=" " ){
      return 0;
		  return writeKey(keyName, (long)value, newComment);
    };
    int writeKey( const char keyName[], float value,
		  const char newComment[]=" " );
    int writeKey( const char keyName[], 
		  double value, const char newComment[]=" " );
    int writeKey( const char keyName[], bool value,   const char newComment[] );
    int writeColumn( double *column,  long ndata, int colnum, int firstRow =1);
    int writeColumn( float *column,  long ndata, int colnum, int firstRow =1);
    int writeColumn( unsigned long *column,  long ndata, int colnum, 
		     int firstRow =1);
    int writeColumn( long *column,  long ndata, int colnum, int firstRow =1);
    int writeColumn( int *column,  long ndata, int colnum, int firstRow =1);
    int writeColumn( short *column, long ndata, int colnum, int firstRow=1);
    int writeColumn( string *column, long ndata, int colnum, int firstRow=1);
    bool columnExists(char columnName[]);
    int readColumn( char columnName[], short *column, long size, 
    		    long firstRow=1, long firstElement=1);
    int readColumn( char columnName[], unsigned short *column, 
		    long size, long firstRow=1, long firstElement=1);
    int readColumn( char columnName[], long *column, long size, 
		    long firstRow=1, long firstElement=1);
    int readColumn( char columnName[], float *column, long size, 
		    long firstRow=1, long firstElement=1);
    int readColumn( char columnName[], double *column, long size, 
		    long firstRow=1, long firstElement=1);
    int readColumn( char columnName[], char **column, 
		    long size, long firstRow=1, long firstElement=1);

  };
  
};


#endif
