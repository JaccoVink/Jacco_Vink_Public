/*****************************************************************************/
//
// FILE        :  addim.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  contains main() : application of class image.
//
//
// COPYRIGHT   : Jacco Vink, SRON, 1995, 2002
//
/******************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_image.h"

#define STRL 256

using jvtools::Image;

int main(int argc, char *argv[] )
{
  if( argc < 4  ){
    printf("Usage: %s inputfile1 inputfile2 [inputfile3 ... inputfileN] outputfile\n",argv[0]);
    exit( 1);
  }// if

  char outputname[STRL];
  Image im1;
  Image im2;
  
  printlogo(argv[0]);
  
  strncpy(outputname, argv[argc-1], STRL );
  
  printf(" Image 1     : %s\n",argv[1]);
  printf(" Image 2/value     : %s\n",argv[2]);
  printf(" Output file : %s\n\n",outputname);
  printf(" Read image nr %d: %s\n",1,argv[1]);
  if( im1.read( argv[1] ) != SUCCESS ){
    puts("Error..");
    exit( 2 );
  }//if


  double value=0.0;
  for(int i = 2; i < argc-1 ; i++ ){
    if( (value=atof(argv[i]))== 0){
      if( im2.read(argv[i])==SUCCESS ){
	printf(" Read image nr %d: %s\n",i , argv[i]);
	im1 += im2;
      }
      else{
	puts("Error...");
	exit(3);
      }
    }else{
      printf(" Adding a value %e to every pixel.",value);
      im1 += value;
    }
  }//end for
  printf("\n");

  im1.origin(argc,argv);
  

  im1.testPrint( stdout);
  im1.writefits(outputname);



}// end main()


