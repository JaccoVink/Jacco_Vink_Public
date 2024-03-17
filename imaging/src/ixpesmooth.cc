/**************************************************************************/
//
// FILE        :  ixpesmooth.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  
//
//
// COPYRIGHT   : Jacco Vink, 2022/24
//
/**************************************************************************/
//  MODIFICATIONS :                                                         
//
//
/**************************************************************************/
#include "jvtools_image.h"
#include "jvtools_stokesmaps.h"

#include <iostream>
#include <strstream>

using namespace::std;
using namespace::jvtools;
using namespace::JVMessages;


int main(int argc, char *argv[])
{

  if( argc < 3 ){
    puts("expecting at least 3 arguments");
    //    puts("-sm=[filtersize]");
    puts("-width=[filtersize]");    
    //    puts("-filter={hamm,box,hann,welch,gauss");
    puts("-filter={hamm,box,welch,gauss");        
    puts("-b=[int binfactor]");
    puts("-det=");        
    return 1;
  }


  
  string cubename=argv[argc-2], outputName=argv[argc-1];
  int binf=1, filtersize=7;
  float sigma=1.0, x0=0.0, y0=0.0, minChiSqr=6.8; 
  float maskclip=50.0;
  StokesMaps stokesMaps;
  int vector=1, vectorskip=3;
  bool radial=false, useMask=false, saveall=false, savesum=false;
  bool magneticvectors=false;
  bool useDetName=false;
  Filter filter=gauss;  

  if( argc > 3 ){
    for(int i=1; i< argc-2; i++){
      if( strncasecmp(argv[i],"-sm=", 4)== 0 ){
	sigma = atof(argv[i]+4);
	printf("-sm is deprececated, please use width; four gaussian sigma = 0.5*width\n");
      }else if( strncasecmp(argv[i],"-width=", 7)== 0 ){
	filtersize = atof(argv[i]+7);
	sigma = 0.5*filtersize;
      }else if( strncasecmp(argv[i],"-filter=box",11)==0){
	filter=boxcar;
      }else if( strncasecmp(argv[i],"-filter=hamm",12)==0){
	filter=hamming;
	//      }else if( strncasecmp(argv[i],"-filter=hann",12)==0){ // there seems to be a problem with the Hann filter
	//	filter=hanning;
      }else if( strncasecmp(argv[i],"-filter=welch",12)==0){
	filter=welch;
      }else if( strncasecmp(argv[i],"-filter=gauss",13)==0){
	filter=gauss;					
      }if( strncasecmp(argv[i],"-b=", 3)== 0 ){
	binf = atoi(argv[i]+3);
	if( binf < 1 ){
	  printf("invalid rebin factor %i\n",binf);
	  return 1;
	}
      }if( strncasecmp(argv[i],"-det", 4)== 0 ){
	useDetName=true;
      }else if( strncasecmp(argv[i],"-novector",9)== 0){
	vector=0;
      }else if( strncasecmp(argv[i],"-vskip=",7)== 0){
	vectorskip=atoi(argv[i]+7);
      }else if( strncasecmp(argv[i],"-mask", 5)== 0 ){
	useMask = true;
	if( strncasecmp(argv[i],"-mask=", 6)== 0 ){
	  maskclip=atof(argv[i]+6);
	}
      }else if( strncasecmp(argv[i],"-magnetic", 9)== 0 ){
	magneticvectors=true;
      }else if( strncasecmp(argv[i],"-novector", 9)== 0 ){
	vector=0;
      }else if( strncasecmp(argv[i],"-minchisqr=", 11)== 0 ){
	minChiSqr= atof(argv[i]+11);
      }else if( strncasecmp(argv[i],"-saveall", 8)== 0 ){
	saveall =true;
      }else if( strncasecmp(argv[i],"-savesum", 8)== 0 ){
	savesum =true;	
      }
#if 0
      else if( strncasecmp(argv[i],"-radial=",8)==0){
	if( sscanf(argv[i]+8,"%f %f", &x0, &y0) != 2){
	  puts("\t radial symmetry assumed, but could not read coordinates of central axis....");
	  return 1;
	}else{
	  radial=true;
	}//else
      }
#endif
      }//for
  }  //if


  switch(filter){
  case boxcar:
    printf("\t*** Smoothing with a boxcarfilter\n");
    break;
  case hamming:
    printf("\t*** Smoothing with a Hamming filter\n");
    break;
  case hanning:
    printf("\t*** Smoothing with a Hanning filter\n");
    break;
  case welch:
    printf("\t*** Smoothing with a Welch filter\n");
    break;   
  default:
    printf("\t*** Smoothing with gaussian filter\n");
  }

  if( (filter != gauss) && (filtersize % 2 == 0 || filtersize < 5) ){

    filtersize=MAX(filtersize,5);
    if( filtersize % 2 == 0 )
      filtersize++;
    
    printf("\t For filters other than the gaussian filter, the filter size must be an odd number.");
    printf("\t Filtersize set to %i\n", filtersize);
  }
      
#if 0  
  if( binf <= 1 )
    rebin=false;
  if( filter != 0 && ( (filtersize < 5) || (filtersize % 2 != 1) ) ){
    printf("\t*** ERROR: filters other than gaussian need a smoothing kernel >=5 !\n");
    return 1;
  }  

  if( radial ){
    printf("\t*** Radial symmetry assumed and rotating...\n");
    printf("\t*** rotation axis: x0 = %.2f y0 = %.2f\n", x0, y0);
    outputName += "_rot";
  }
#endif

  printf("\t*** Input file (du1) = %s\n", cubename.c_str());
  printf("\t*** Output name      = %s\n", outputName.c_str());
  if( binf > 1)
    printf("\t*** Binning factor   = %i\n", binf);
  if( filter != gauss )
    printf("\t*** Filtersize       = %i pixels\n", filtersize);
  else
    printf("\t*** sigma            = %.1f pixels\n", sigma);
  if( useMask ){
    printf("\t*** Mask clip = %.2f\n",maskclip);
  }
  
  if( useDetName)
    printf("\t*** Using 'det' and not 'du' as unit tag\n");
  if( stokesMaps.read(cubename, useDetName) == FAILURE )
    return 2;

#if 0
  if( radial ){
    printf("\t Start rotating azimuthally around %.2f %.2f\n", x0,y0);
    for( int det=1; det<=3; det++){
      printf("\t*** Rotating for det = %i\n",det);
      dummy.radiallyRotQU( &stokesMaps.stokesQ[det], &stokesMaps.stokesU[det], x0, y0);      
    }
  }
#endif

  if( (binf > 1)  && (stokesMaps.rebinMaps(binf) == FAILURE) )
    return 3;

  if( stokesMaps.processMaps() == FAILURE )
    return 4;

  if( saveall )
    stokesMaps.save( outputName, saveall);

  if(  stokesMaps.filterMaps( filtersize, filter) == FAILURE )
    return 5;
  
  if( stokesMaps.applyMask( maskclip ) == FAILURE)
    return 6;
  
  if( stokesMaps.processMaps() == FAILURE )
    return 7;

  if( stokesMaps.save( outputName) == FAILURE )
    return 8;

  if( stokesMaps.saveVectors( outputName, magneticvectors, vectorskip, vector, minChiSqr) == FAILURE )
    return 9;

  
  return 0;
}

