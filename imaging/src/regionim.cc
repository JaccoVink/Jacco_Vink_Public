/*****************************************************************************/
//
// FILE        :  regionim.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  contains main() : application of class image.
//
//
// COPYRIGHT   : Jacco Vink, SRON, 1998
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
  if( argc < 4 ){
    printf("Usage: %s ",argv[0]);
    puts("[-mask] [-old] inputimage outputimage regionfile1 [regionfile2...]");
    puts("\t-old : old method using less memory, but not handling composed regions");
    exit( 1);
  }// if

  char inputname[STRL], outputname[STRL], regionfile[STRL];
  int noptions=0;
  bool makeMask=false, oldMethod = false;
  Image image, mask;
  
  for(int i=1; i < argc; i++){

    if( strncasecmp(argv[i],"-",1) == 0){
      noptions++;
      if(  strncasecmp(argv[i],"-mask",5) == 0)
	makeMask = true;
      else if(  strncasecmp(argv[i],"-old",5) == 0)
	oldMethod = true;
      else
	printf("Unknown option: %s\n",argv[i]);
    }//if

  } //for

  if( noptions+3 >= argc ){
    puts("Error: No input/output image(s) and/or region file(s) specified!");
    exit(2);
  }
    
  strncpy(inputname, argv[noptions+1],STRL);
  strncpy(outputname,argv[noptions+2],STRL);

  
  printlogo(argv[0]);

  printf(" Input image  : %s\n", inputname);
  printf(" Output image : %s\n", outputname);


  if( image.readfits( inputname ) == FAILURE ){
    puts("Error..");
    exit(2);
  }//if

  if( !oldMethod ){
    mask = image;
    mask *= 0.0;
    mask -= 1.0;
  }

  if( oldMethod && makeMask == true ){
    printf("\tA mask image (containing only 0 and 1) will be made.\n");
    image *= 0.0;
    image += 1.0;
  }//if

  image.setblank( 0.0);
  for( int i=noptions+3 ; i < argc; i++){

    strncpy(regionfile,argv[i],STRL);
    printf(" Region file %2i: %s\n\n",i-2, regionfile);
    if( oldMethod ){
      if( image.selectregion(regionfile, true)== FAILURE ){
	puts("Failure reading region file.");
	exit(3);
      }
    }
    else{
      if( mask.selectRegion(regionfile, true)== FAILURE ){
	puts("Failure reading region file.");
	exit(3);
      }//if
    }//else
  }//for

  
  if( !oldMethod ){
    mask.clip(0.0,1e6);
    mask /= mask;
    if( makeMask )
      image = mask;
    else
      image *= mask;
  }

  image.origin(argc, argv);
  
  image.testPrint( stdout);
  image.save(outputname);
  
  return 0;
    
}// end main()


