/*****************************************************************************/
//
// FILE        :  markim.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  contains main() : application of class image.
//
//
// COPYRIGHT   : Jacco Vink, SRON, 2001
//
/******************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_image.h"

#define STRL 81

using jvtools::Image;

int main(int argc, char *argv[] )
{
  if( argc < 3 || argc > 7){
    printf("Usage: %s -threshold=N/-t=N -dilate=N -erode=N inputfile outputfile\n",argv[0]);
    exit(  _NPARAM_ERROR_);
  }// if

  char inputname[STRL], outputname[STRL];
  int ndil=0, ner=0;
  float threshold=0.5, max;
  bool erode= false, dilate=false;
  Image image;

  strncpy(inputname, argv[argc-2],STRL-1);
  strncpy(outputname, argv[argc-1],STRL-1);

  for( int i=1 ; i < argc-2; i++){

    if( strncasecmp(argv[i], "-threshold=",7)==0)
      threshold = atof(argv[i]+7);
    else if( strncasecmp(argv[i], "-t=",3)==0)
      threshold = atof(argv[i]+3);
    else if( strncasecmp(argv[i], "-dilate",4)==0){
      dilate = TRUE;
      if( strlen(argv[i]) > 8)
	ndil=atoi(argv[i]+8);
      else
	ndil =1;
    }
    else if( strncasecmp(argv[i], "-erode",4)==0){
      erode = TRUE;
      if( strlen(argv[i]) > 7)
	ner = atoi(argv[i]+7);
      else
	ner =1;
    }
    else{
      printf("Unknown option: %s\n",argv[i]);
    exit( _UNKNOWN_PARAM_ERROR_ );
    }
  }//for

  printlogo( argv[0]);

  printf(" Input file  : %s\n", inputname);
  printf(" Output file : %s\n", outputname );
  printf(" Threshold   : %.3f\n",threshold);
  printf(" Dilate      : %i\n", ndil);
  printf(" Erode       : %i\n", ner);

  if( image.readfits( inputname )== FAILURE ){
    puts("Error...");
    exit(_IO_ERROR_);
  }//if
  max = 2.0 * image.maximum();
  image.bilevel( threshold, max);


  if(dilate == TRUE){
    printf("Dilating image...\n");
    for( int i=0; i < ndil; i++){
      image.dilate( 1.0 );
      printf("%i ",i+1);
    }
    printf("\n");
  }

  if( erode == TRUE ){
    printf("Eroding image...\n");
    for( int i=0; i < ner; i++){
      image.erode( 1.0 );
      printf("%i ",i+1);
    }
    printf("\n");
  }

  printf("\tNumber of indepent regions found = %i\n",
	 image.startMark8( 0.5, 1.5 ) );

  image.origin(argc, argv);

  image.testPrint( stdout);
  image.writefits(outputname);
  
  return  _NO_ERROR_;
  
}// end main()




