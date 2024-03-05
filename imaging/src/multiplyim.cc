/*****************************************************************************/
//
// FILE        :  multiplyim.cc
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
using namespace JVMessages;


int main(int argc, char *argv[] )
{
    if( argc != 4 && argc != 5){
      printf("Usage: %s inputfile factor/inputfile2 outputfile",argv[0]);
      puts("       inputfile2 should not start with a number");
      exit( 1);
    }// if

    int strl;
    char outputname[STRL], inputname[STRL];
    float factor;
    bool forceImage=false, divide=false, silent=false;
    Image im1, im2;

    if( argc>4){
      for(int i=1; i < argc-3; i++){
	if( strncasecmp(argv[i],"-im",3)==0){
	  forceImage=true;
	  puts("\t Will assume input 2 is a number!");
	}else if( strncasecmp(argv[i],"-div",3)==0){
	  divide=true;
	}else if( strncasecmp(argv[i],"-silent",7)==0){
	  silent=true;
	  setChatLevel(NO_WARNINGS);
	}
      }
    }

    if(!silent)
      printlogo(argv[0]);

    strl = strlen(argv[argc-2]);
    factor = strtod(argv[argc-2], NULL );

    strncpy(inputname, argv[argc-3], STRL-1 );
    strncpy(outputname, argv[argc-1], STRL-1 );

    if(!silent){
      printf(" Input  file    : %s\n",inputname);
      if( (factor == 0 && strl > 1 ) || forceImage ){ 
	forceImage = true;
	printf(" Input  file  2 : %s\n",argv[argc-2]);
      }
      else{
	if( divide ){
	  puts("  Dividing instead of multiplying");
	  factor = 1.0/factor;
	}
	printf(" Factor         : %f\n",factor);
      }
      printf(" Output file    : %s\n\n",outputname);
    }

    if( im1.readfits( inputname ) != SUCCESS ){
      puts("Error..");
      exit( 2 );
    }//if

    if( forceImage ){
      if( im2.readfits( argv[argc-2] ) != SUCCESS ){
	puts("Error...");
	exit( 3 );
      }//if
      im1 *= im2;
    }
    else{
      im1.setblank(0.0);
      im1 *= factor;
    }//else

    im1.origin(argc, argv);
    if(!silent)
      im1.testPrint( stdout);
    
    im1.save(outputname);



    return 0;
}// end main()





