/*****************************************************************************/
//
// FILE        :  rebinim.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  contains main() : application of class image.
//
//
// COPYRIGHT   : Jacco Vink, SRON, 1998, 2002
//
/******************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_image.h"


using jvtools::Image;

int main(int argc, char *argv[] )
{
  if( argc < 3 || argc > 5){
    printf("Usage: %s [-b=N/-block=N] inputfile outputfile factor\n",argv[0]);
    exit( 1);
  }// if

  string inputName = argv[argc-2], outputName = argv[argc-1];
  int factor = 1, xfactor=1, yfactor=1;
  bool block = false;
  Image image;


  for( int i=1 ; i < argc-2; i++){

    if( strncasecmp(argv[i], "-b=",3)==0)
      factor = atoi(argv[i]+3);
    else if( strncasecmp(argv[i], "-bx=",3)==0)
      xfactor = atoi(argv[i]+4);
    else if( strncasecmp(argv[i], "-by=",3)==0)
      yfactor = atoi(argv[i]+4);
    else if( strncasecmp(argv[i], "-block",6)==0)
      block = true;
    else{
      printf("Unknown option: %s\n",argv[i]);
    exit(2);
    }
  }//for

  if( factor == 1 && xfactor == 1 && yfactor == 1 ){
    printf("Rebinning factor has not been set, assume it to be 2\n");
    factor = 2;
  }
  if( (xfactor > 1 || yfactor > 1) && factor > 1){
    printf("It is not allowed to combine -b=N and -bx=N/-by=N\n");
    exit(2);
  }

  printlogo( argv[0]);

  printf(" Input file  : %s\n", inputName.c_str());
  printf(" Output file : %s\n", outputName.c_str() );
  if( factor > 1)
    printf(" Rebinning factor = %d\n",factor);
  if( xfactor > 1)
    printf(" X rebinning factor = %d\n",xfactor);
  if( yfactor > 1)
    printf(" Y rebinning factor = %d\n",yfactor);

  if( block )
    printf(" Blocking image\n\n");

  if( image.read( inputName.c_str() )== FAILURE ){
    puts("Error...");
    exit(2);
  }

  if( !block ){
    if( factor > 1 ){
      if( image.rebin(factor) == FAILURE ){
	puts("Error...");
	exit(3);
      }//if
    }
    else if( xfactor > 1 || yfactor > 1 ){
       if( image.rebinx(xfactor) == FAILURE ){
	puts("Error...");
	exit(3);
       }//if
       if( image.rebiny(yfactor) == FAILURE ){
	puts("Error...");
	exit(3);
       }//if
    }//else

  }
  else
    if( image.block(factor) == FAILURE ){
      puts("Error...");
      exit(3);
    }//if

  image.origin(argc, argv);

  image.testPrint( stdout);
  image.writefits(outputName.c_str());

  exit( 0 );

}// end main()


