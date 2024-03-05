/*****************************************************************************/
//
// FILE        :  subtractim.cc
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

#define STRL 256

using jvtools::Image;

int main(int argc, char *argv[] )
{
  if( argc < 4  ){
    puts("Usage: subtractim [-norm] inputfile1 inputfile2/value outputfile");
    exit( 1);
  }// if

  char inputfile1[STRL], inputfile2[STRL], outputname[STRL];
  bool subtract_image= true, normalize=false;
  float value=0.0;
  Image im1, im2;

  strncpy(inputfile1, argv[argc-3],STRL);
  //  strncpy(inputfile2, argv[argc-2],STRL);
  strncpy(outputname, argv[argc-1], STRL-1 );

  printlogo( argv[0]);

  if( strncasecmp(argv[1],"-norm",5)== 0 )
    normalize=true;

  if( (argv[argc-2][0] >= '1' && argv[argc-2][0] <= '9') &&  (strncmp(argv[1],"-I",2)!=0)  ){
    value =atof(argv[argc-2]);
    subtract_image=false;
    puts("subtracting number");
  }
  else
    strncpy(inputfile2, argv[argc-2],STRL);

  printf(" Input file 1 : %s\n",inputfile1);
  if( value == 0.0)
    printf(" Input file 2 : %s\n",inputfile2);
  else
    printf(" Value        = %f\n",value);

  printf(" Output file  : %s\n\n",outputname);
  if( normalize )
    printf(" Second image will be normalized to first image\n");
  
  if( im1.readfits( inputfile1 ) == FAILURE){
    puts("Error..");
    exit( 2 );
  }//if
  if( subtract_image == true){
    if( im2.readfits( inputfile2) == FAILURE ){
      puts("Error..");
      exit( 2 );
    }
    if( normalize ){
      float total1=im1.total();
      float total2=im2.total();
      float normfactor= total1/total2;
      im2 *= normfactor;
      printf("total image 1 = %e\n", total1);
      printf("total image 2 = %e\n", total2);
      printf("norm factor = %f\n", normfactor);
    }
  }
  if( value == 0.0)
    im1 -= im2;
  else
    im1 -= value;

  im1.origin(argc, argv);

  im1.testPrint( stdout);
  im1.writefits(outputname);

  return 0;

}// end main()





