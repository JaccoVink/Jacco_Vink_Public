/*****************************************************************************/
//
// FILE        :  clipim.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  contains main() : application of class image.
//
//
// COPYRIGHT   : Jacco Vink, SRON, 1995
//
/******************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_image.h"

#define STRL 1024

using jvtools::Image;

int main(int argc, char *argv[] )
{

  if( argc < 5){
    printf("Usage: %s input-file output-file low-cutoff high-cutoff\n",
	   argv[0]);
    exit( 1);
  }// if


  float min=0.0, max = 1000.0;
  char inputfile[STRL], outputfile[STRL];
  bool relative=false, saturate=false;
  Image image;

  printlogo(argv[0]);

  strncpy(inputfile,argv[argc-4],STRL-1);
  strncpy(outputfile,argv[argc-3],STRL-1);
  min = atof(argv[argc-2]);
  max = atof(argv[argc-1]);

  for(int i=1; i < argc-4; i++){
    if( strncasecmp(argv[i],"-rel",4)==0 )
	relative=true;
    if( strncasecmp(argv[i],"-sat",4)==0 )
      saturate=true;
  }
  

  printf(" Input file  : %s\n", inputfile);
  printf(" Output file : %s\n", outputfile);

  if( image.readfits( inputfile )== FAILURE ){
    puts("Error...");
    exit( 2 );
  }
  if( relative==true){
    float pixmax=image.maximum();
    min *= pixmax;
    max *= pixmax;
    printf(" Maximum pixel value: %.3f\n", pixmax);
  }//

  printf(" min = %f  max = %f   \n",min, max);
  image.setblank(0.0);
  image.clip(min, max, saturate );

  image.origin(argc, argv);


  image.testPrint( stdout);
  image.writefits(outputfile);

  return 0;
}// end main()








