/*****************************************************************************/
//
// FILE        :  region_io.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Implementation of the  Region class.
//                Basic I/O functions.
//
// COPYRIGHT   : Jacco Vink, 2002
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_region.h"
#include "jvtools_messages.h"
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
Status Region::read(const char fileName[], const char extensionName[] )
{
  setFunctionID("read[Region]");

  fitsfile *fptr;
  char comment[FLEN_COMMENT];
  int status= 0, anynull;
  int hdutype = BINARY_TBL, colnum_x, colnum_y, colnum_shape;
  int maxdim=3, dim1, dim2, dim3, naxis;
  long naxes[3];
  long naxis2=0, firstrow=1, firstelem=1;


  //  Open the fits file.
  //
  fits_open_file( &fptr, fileName , READONLY, &status );


  read_std_keywords( fptr );

  if( status != 0 ){
    fits_close_file(fptr, &status );
    return error("error opening file, status = ",status);
  }

  JVMessages::message("reading fits file ", fileName);


  status = 0;
  int strl= strlen(extensionName)+1;
  char *extName = new char [strl];
  strncpy(extName, extensionName, strl);
  fits_movnam_hdu(fptr, hdutype, extName, 0, &status);
  delete [] extName;
  readStdKeywords( fptr );
  JVMessages::message("fits file extension ", extensionName);
  if( status != 0 ){
    fits_close_file(fptr, &status );
    return error("could not find HDU, status = ",status);
  }//if

  JVMessages::message("Found HDU name " , extensionName);
  if( instrumentID == XMM_RGS1 || instrumentID == XMM_RGS2 ){
    unsigned int np = 0, nreg=0;
    char **shape_str, mform1[FLEN_VALUE];
    RegionSystem sys;

    JVMessages::message("region file for XMM RGS, known region format");
    fits_read_key_lng( fptr, "NAXIS2", &naxis2, comment, &status );
    if( status != 0 || naxis2 <= 0){
      fits_close_file(fptr, &status );
      return error("error reading naxis2 value, status = ",status);
    }//if
    nreg = naxis2;

    JVMessages::message("number of regions = ", (unsigned long)nreg);

    fits_read_key_str( fptr, "MFORM1", mform1, comment,&status);
    JVMessages::message("MFORM1 = ", mform1);
    if( strncasecmp( mform1, "BETA_CORR,PI",FLEN_VALUE) == 0 ){
      sys = BETA_PI; 
      fits_get_colnum( fptr, FALSE, "BETA_CORR", &colnum_x, &status);
      fits_get_colnum( fptr, FALSE, "PI", &colnum_y, &status);
    }
    else if(strncasecmp( mform1, "BETA_CORR,XDSP_CORR", FLEN_VALUE) == 0){
      sys = BETA_XDSP; 
      fits_get_colnum( fptr, FALSE, "BETA_CORR", &colnum_x, &status);
      fits_get_colnum( fptr, FALSE, "XDSP_CORR", &colnum_y, &status);
    }
    else{
      fits_close_file(fptr, &status );
      return error("unknown XMM RGS region system:", mform1);
    }//else
    fits_get_colnum( fptr, FALSE, "SHAPE", &colnum_shape, &status);

    fits_read_tdim(fptr, colnum_x, maxdim, &naxis, naxes, &status);
    dim1 = naxes[0];
    fits_read_tdim(fptr, colnum_y, maxdim, &naxis, naxes, &status);
    dim2 = naxes[0];
    fits_read_tdim(fptr, colnum_shape, maxdim, &naxis, naxes, &status);
    dim3 = naxes[0];
    JVMessages::message("dim3 = ", (unsigned long)dim3);

    shape_str = new char * [naxis2+2];
    for(int i=0; i < naxis2; i++)
      shape_str[i] = new char [dim3+1];

    if( dim1 != dim2 || dim1 <= 0 || dim3 >= FLEN_VALUE){
      fits_close_file(fptr, &status );
      return error("inconsistent format dimensions");
    }

    if( allocate(nreg,  nreg*dim1) == FAILURE ){
      fits_close_file(fptr, &status );
      return error("memory allocation error");
    }

    fits_read_col_str(fptr, colnum_shape, firstrow, 1, naxis2," " ,
		      &shape_str[0], &anynull, &status );

    firstelem=0;
    for( unsigned int firstrow = 0 ; firstrow < nregions; firstrow++){

      fits_read_col_flt(fptr, colnum_x, firstrow+1, 1, dim1, 0 ,
			&x[firstelem], &anynull, &status );
      fits_read_col_flt(fptr, colnum_y, firstrow+1, 1, dim2, 0 ,
			&y[firstelem], &anynull, &status );

      npoints[firstrow] = 0;
      system[firstrow]  = sys;
      do{
	npoints[firstrow] ++;
      }while( npoints[firstrow] < (unsigned int)dim1 && 
	      !(x[firstelem+npoints[firstrow]] == x[firstelem] &&
		y[firstelem+npoints[firstrow]] == y[firstelem]) );
      npoints[firstrow] ++;

      for(unsigned int i = np; i< np+npoints[firstrow]; i++){
	if( system[firstrow] == BETA_PI || system[firstrow] == BETA_XDSP )
	  x[i] *= RAD2DEG;
	if( system[firstrow] == BETA_XDSP )
	  y[i] *= RAD2DEG;
      }//for
      np += npoints[firstrow];

      firstelem += npoints[firstrow];

      if( strncasecmp(shape_str[firstrow],"POLYGON", 7) == 0 ||
	  strncasecmp(shape_str[firstrow],"!POLYGON", 8) == 0 ){
	shape[firstrow] = POLYGON;
	if( strncasecmp(shape_str[firstrow],"!POLYGON", 8) == 0 )
	  exclude[firstrow] = true;
      }//if
      //message("region # ", (unsigned long)firstrow+1);
      //      message("# points ", (unsigned long)npoints[firstrow]);
      //      message("shape  = ",shape_str[firstrow]);
      //      message("exclude = ", (int)exclude[firstrow]);
      //      message("system  = ", (int)system[firstrow]);
    }//for


    for(int i=0; i < naxis2; i++)
      delete [] shape_str[i];
    delete [] shape_str;
    JVMessages::message("nelements = ",(unsigned long)np );
    JVMessages::message("buffersize = ",buffer_size );
  }//if instrumentID == XMM_RGS1/2

  status=0;
  fits_close_file(fptr, &status );

  return JVMessages::success();
}//read()

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
//  RETURNS    :
//
/****************************************************************************/
Status Region::saveDS9(const char fileName[])
{
  setFunctionID("saveDS9");

  message("# regions = ",(int)nregions);
  FILE *fp;

  if(!(fp=fopen(fileName,"w")) )
    return error("error opening file");

  fprintf(fp,"# Region file format: DS9 version 3.0\n");
  fprintf(fp,"global color=green font=\"helvetica 10 normal\"");
  fprintf(fp," select=1 edit=1 move=1 delete=1 include=1 fixed=0\n");

  unsigned long np = 0;
  for(unsigned int i=0 ; i < nregions; i++){

    switch( system[i] ){
    case IMAGE:
      fprintf(fp,"image;");
      break;
    case FK5:
      fprintf(fp,"fk5;");
      break;
    case ICRS:
      fprintf(fp,"icrs;");
      break;
    default:
      fprintf(fp,"unknown;");
    }//if
    if( exclude[i] == true)
      fprintf(fp,"-");

    switch( shape[i] ){
    case CIRCLE:
      fprintf(fp,"circle(");
      if( system[i] == FK5 && npoints[i] == 1 && np < buffer_size )
	fprintf(fp, "%f, %f, %f\"",xpos[np], ypos[np], size1[np]);
      else if( npoints[i] == 1 && np < buffer_size)
	fprintf(fp, "%f, %f, %f",xpos[np], ypos[np], size1[np]);
      break;
    case RECTANGLE:
      fprintf(fp,"rectangle(");
      break;
    case ELLIPSE:
      fprintf(fp,"ellipse(");
      if( system[i] == FK5 && npoints[i] == 1 && np < buffer_size )
	fprintf(fp, "%f, %f, %f\", %f\", %f",
		xpos[np], ypos[np], size1[np], size2[np], angle[np]);
      else if( npoints[i] == 1 && np < buffer_size)
	fprintf(fp, "%f, %f, %f, %f, %f",
		xpos[np], ypos[np], size1[np], size2[np],angle[np]);
      break;
    case POLYGON:
      fprintf(fp,"polygon(");
      for(unsigned int j=np; j < np+npoints[i] && j < buffer_size; j++){
	if( j == np )
	  fprintf(fp,"%f, %f",xpos[j], ypos[j]);
	else if( j <  np+npoints[i]-1 || shape[i] != POLYGON )
	  fprintf(fp,", %f, %f",xpos[j], ypos[j]);
      }//for
      break;
    default:
      fprintf(fp,"unknown(");
    }
    fprintf(fp,")\n");

    np += npoints[i];
  }//for

  fclose(fp);
  return success();
}//saveDS9

/****************************************************************************/
//
// FUNCTION    : readDS9
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
#define MAX_LINE_LENGTH 1024
Status Region::readDS9( const char fileName[] )
{
  setFunctionID("readDS9[region]");

  FILE *fp;

  if( !(fp=fopen(fileName, "r")) )
    return error("error opening file");

  int i;
  int lineNr = 0;
  unsigned int maxNreg=0, nreg=0;
  unsigned long np=0;
  char line[MAX_LINE_LENGTH], identifier[MAX_LINE_LENGTH];

  allocate( 100 );
  maxNreg = nregions;
  nregions = 0;
  while( fgets( line, MAX_LINE_LENGTH-1, fp) != NULL && nreg < maxNreg &&
	 np < buffer_size ){

    // skip leading spaces
    for(i = 0 ; 
	i < MAX_LINE_LENGTH && line[i] != '\n' && line[i] == ' ' /* && 
					    line[i] == '\t' */; i++);

    if( i < MAX_LINE_LENGTH && line[i] != '\0' && line[i] != '#' ){

      int j=i;
      while( j < MAX_LINE_LENGTH && line[j] != '\0' && line[j] != ' ' && 
	     line[j] != ';'){
	identifier[j-i] = line[j++];
      };
      identifier[j-i] = '\0';
      i = j;
      if( strncasecmp(identifier,"global",6) != 0 && 
	  strlen(identifier) > 1){

	if( strncasecmp(identifier,"image",5) == 0)
	  system[nreg] = IMAGE;
	else if( strncasecmp(identifier,"fk5",3) == 0)
	  system[nreg] = FK5;
	else if( strncasecmp(identifier,"icrs",6) == 0)
	  system[nreg] = ICRS;
	else{
	  fclose(fp);
	  return error("unknown region system", identifier);
	}//else
	for( j=i; j < MAX_LINE_LENGTH && line[j] == ' '; j++);
	//	printf("next char found : %c\n",line[j]);
	if( line[j++] != ';' ){
	  fclose(fp);
	  return error("expected \';\', improper format");
	}
	i = j;
	for( j=i; j < MAX_LINE_LENGTH && line[j] == ' '; j++);
	//	printf("next char found (-): %c\n",line[j]);
	if( line[j] == '-'){
	  exclude[nreg] = true;
	  j++;
	}
	else
	  exclude[nreg] = false;
	message("exclude? ", exclude[nreg]);
	if( line[j] == '+' )
	  j++;
	i = j;
	for( j=i; j < MAX_LINE_LENGTH && line[j] == ' '; j++);
	i=j;
	while( j < MAX_LINE_LENGTH && line[j] != '\0' && 
	       line[j] >= 'A' && line[j] <= 'z'){
	  identifier[j-i] = line[j++];
	};
	identifier[j-i] = '\0';
	i = j;
	//	printf("Identifier 2 found: %s\n", identifier);
	if( strncasecmp(identifier,"circle",6) == 0 )
	  shape[nreg] = CIRCLE;
	else if( strncasecmp(identifier,"ellipse",7) == 0 )
	  shape[nreg] = ELLIPSE;
	else if( strncasecmp(identifier,"rectangle",7) == 0 )
	  shape[nreg] = RECTANGLE;
	else if( strncasecmp(identifier,"polygon",7) == 0 )
	  shape[nreg] = POLYGON;
	else{
	  fclose(fp);
	  return error("unknown region shape", identifier);
	}
	message("shape = ", shape[nreg]);
	// skip space
	for( j=i; j < MAX_LINE_LENGTH && line[j] == ' '; j++);
	//	printf("next char found : %c\n",line[j]);
	if( line[j++] != '(' || j >= MAX_LINE_LENGTH){
	  fclose(fp);
	  return error("expected \'(\', improper format");
	}//if
	i=j;
	npoints[nreg] = 0;
	if( shape[nreg] == CIRCLE && np < buffer_size){
	  if( ( (system[nreg] == FK5 || system[nreg] == ICRS) && 
		sscanf(line+i,"%f, %f, %f\" ",
		       &xpos[np], &ypos[np],&size1[np]) != 3) ||
	      (system[nreg] == IMAGE && 
	       sscanf(line+i,"%f, %f, %f",
		      &xpos[np], &ypos[np], &size1[np]) != 3) ){
	    fclose(fp);
	    return error("expected 3 input values");
	  }
	  npoints[nreg] = 1;
	  np += npoints[nreg];
	  nreg++;
	}
	else if( shape[nreg] == ELLIPSE && np < buffer_size){
	  if( ( (system[nreg] == FK5 || system[nreg] == ICRS) && 
		sscanf(line+i,"%f, %f, %f\", %f\", %lf",
		       &xpos[np], &ypos[np],&size1[np], &size2[np], 
		       &angle[np]) != 5) ||
	      (system[nreg] == IMAGE && 
	       sscanf(line+i,"%f, %f, %f, %f, %lf", &xpos[np], &ypos[np],
		      &size1[np], &size2[np], &angle[np]) != 5)  ){
	    fclose(fp);
	    return errorwarning("expected 5 input values");
	  }
	  npoints[nreg] = 1;
	  np += npoints[nreg];
	  nreg++;
	}
	else if( shape[nreg] == RECTANGLE ){
	  ;
	}
	else if( shape[nreg] == POLYGON ){
	  while( np+2 < buffer_size  && i < MAX_LINE_LENGTH-1 &&
		 line[i] != '\0' &&
		 sscanf(line+i,"%f, %f", &xpos[np], &ypos[np] ) == 2 ){
	    for( j=i; j < MAX_LINE_LENGTH-1 && line[j] != '\0'
		   && line[j] != ','; j++);
	    i = j+1;
	    for( j=i; j < MAX_LINE_LENGTH-1 && line[j] != '\0'
		   && line[j] != ',' && line[j] != ')'; j++);
	    i = j+1;
	    np++;
	    npoints[nreg]++;
	  };
	  printf("np = %i\n", npoints[nreg]);
	  if( np+1 < buffer_size ){
	    xpos[np] = xpos[np-npoints[nreg]];
	    ypos[np] = ypos[np-npoints[nreg]];
	    np++;
	    npoints[nreg]++;
	  }
	  i = MAX(j-2, 0);
	  i = MIN(i, MAX_LINE_LENGTH-1);
	  nreg++;
	}
	for( j=i; j < MAX_LINE_LENGTH && line[j] != '\0' && line[j] != ')'; 
	     j++);
	if( line[j] != ')' ){
	  fclose(fp);
	  return error("no closing \')\' found");
	}//if
      }
      else
	message("Ignore: ",identifier);
    }//if

    lineNr++;
  }//while

  fclose(fp);


  nregions = nreg;
  message("number of lines in file = ", lineNr);
  message("number of regions       = ", (unsigned long)nregions);


  return success();
}//readDS9();
#undef MAX_LINE_LENGTH

/****************************************************************************/
//
// FUNCTION    : saveSAS()
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  fileName              I                       file name
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Region::saveSAS(const char fileName[])
{
  setFunctionID("saveSAS");
  
  return success();
}//






