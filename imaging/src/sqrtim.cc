/*****************************************************************************/
//
// FILE        :  sqrtim.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  contains main() : application of class image.
//
//
// COPYRIGHT   : Jacco Vink, 2000
//
/******************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_image.h"

#define STRL 1024

using namespace::jvtools;

int main(int argc, char *argv[] )
{

  if( argc != 3  ){
    printf("Usage: %s input-image output-image\n", argv[0]);
    exit( 1);
  }// if

  char inputname[STRL], outputname[STRL];
  Image image;

  printlogo(argv[0]);
  strncpy(inputname, argv[argc-2], STRL );
  strncpy(outputname, argv[argc-1], STRL );

  printf(" Input image        : %s\n", inputname);
  printf(" Output image       : %s\n", outputname);
  printf("\n\n");

  if( image.readfits( inputname ) != SUCCESS || image.squareroot( )!=SUCCESS ){
    puts("Error..");
    exit( 1 );
  }


  image.origin( argc, argv);

  image.testPrint(stdout);
  image.save( outputname);

  return 0;
}// end main()


