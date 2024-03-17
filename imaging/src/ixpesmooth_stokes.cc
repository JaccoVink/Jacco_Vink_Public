/*****************************************************************************/
//
// FILE        :  ixpesmooth_stokes.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  
//
//
// COPYRIGHT   : Jacco Vink, UvA, 2023
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
// 
//
/*****************************************************************************/

#include "jvtools_image.h"
#include "jvtools_stokesmaps.h"
#include "jvtools_polarization.h"

#include <iostream>
#include <strstream>

using namespace::std;
using namespace::jvtools;
using namespace::JVMessages;


#define STRL 1014



/****************************************************************************/
//
// FUNCTION    : sum_detectors()
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
Status StokesMaps::sum_detectors()
{
  setFunctionID("sum_detectors");

  message("Summing detector maps...");

  if( stokesI[3].getnx() <= 0 || stokesI[3].getny() <= 0 ||
      varU[3].getnx() <= 0 ||    varU[3].getny() <= 0 )
    return error("No Stokes data present");
  
  counts[0] = counts[1];
  stokesI[0]= stokesI[1];
  stokesQ[0]= stokesQ[1];
  stokesU[0]= stokesU[1];
  varI[0]= varI[1];
  varQ[0]= varQ[1];
  varU[0]= varU[1];  
  for(int det=2; det<=3; det++){
    counts[0] += counts[det];
    stokesI[0] += stokesI[det];
    stokesQ[0] += stokesQ[det];
    stokesU[0] += stokesU[det];
    varI[0] += varI[det];
    varQ[0] += varQ[det];
    varU[0] += varU[det];    
  }


  char detname[]="DU1+DU2+DU3";
  stokesI[0].setDetector(detname);
  stokesI[0].setCreator("ixpesmooth");
  
  stokesQ[0].setDetector(detname);
  stokesQ[0].setCreator("ixpesmooth");
  
  stokesU[0].setDetector(detname);
  stokesU[0].setCreator("ixpesmooth");
  
  varI[0].setDetector(detname);
  varI[0].setCreator("ixpesmooth");
  
  varQ[0].setDetector(detname);
  varQ[0].setCreator("ixpesmooth");
  
  varU[0].setDetector(detname);
  varU[0].setCreator("ixpesmooth");

  counts[0].setDetector(detname);
  counts[0].setCreator("ixpesmooth");
  
  dataSummed = true;
  
  return success();
}//


/****************************************************************************/
//
// FUNCTION    : binMaps
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
Status StokesMaps::rebinMaps(int binf)
{
  setFunctionID("binMaps");

  if( binf <= 1 || binf >32 ){
    return error("invalid bin factor ", binf);
  }

  if( !dataSummed )
    return error("Maps were not summed...");

  for(int det=0; det<4; det++){
    if( counts[det].rebin(binf) == FAILURE ||
	stokesI[det].rebin(binf) == FAILURE ||
	stokesQ[det].rebin(binf) == FAILURE ||
	stokesU[det].rebin(binf) == FAILURE ||
	varI[det].rebin(binf) == FAILURE ||
	varQ[det].rebin(binf) == FAILURE ||  
	varU[det].rebin(binf) == FAILURE ){
      return error("Error binning data");
    }
  }//for
  
  return success();
}



/****************************************************************************/
//
// FUNCTION    : filterMaps
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
//Status StokesMaps::filterMaps(double sm, Filter filter)
Status StokesMaps::filterMaps(int fsize, Filter filter)
{
  setFunctionID("filterMaps");


  message("fsize = ",fsize);
  //  warning("\t You have selected Stokes map smoothing at your own risk!!");

  if( (filter == !gauss) &&  ( (fsize % 2 == 0) || (fsize < 5) ))
    return error("invalid filter size = ", fsize); 

  filtersize = fsize;
  sigma= 0.5*filtersize;

  int nx=stokesI[0].getnx();
  int ny=stokesI[0].getnx();
  
  
  if( filter != gauss ){
    message("filter size = ", filtersize );
  }else{
    message("gaussian sigma = ", sigma );    
  }

  if( filtersize*4 >= MIN(nx,ny) ){
    return error("Filter size is too big for the size of the image (nx/filter<=4), MIN(nx,ny) = ", MIN(nx,ny));
  }

  

  char infotext[STRL];
  
  switch(filter){
  case boxcar:
    stokesI[0].boxcarfilter(filtersize);
    stokesQ[0].boxcarfilter(filtersize);
    stokesU[0].boxcarfilter(filtersize);
    varQ[0].boxcarfilter(filtersize, -1, -1, -1, -1, true);
    varU[0].boxcarfilter(filtersize, -1, -1, -1, -1, true);
    sprintf(infotext,"Filtered: boxcar; filtersize (pix) = ",filtersize);        

    break;
  case hamming:
    stokesI[0].hammingfilter(filtersize);
    stokesQ[0].hammingfilter(filtersize);
    stokesU[0].hammingfilter(filtersize);
    varQ[0].hammingfilter(filtersize, -1, -1, -1, -1, true);
    varU[0].hammingfilter(filtersize, -1, -1, -1, -1, true);
    sprintf(infotext,"Filtered: hamming; filtersize (pix) = %i",filtersize);    
    break;
  case hanning:
    stokesI[0].hanningfilter(filtersize);
    stokesQ[0].hanningfilter(filtersize);
    stokesU[0].hanningfilter(filtersize);
    varQ[0].hanningfilter(filtersize, -1, -1, -1, -1, true);
    varU[0].hanningfilter(filtersize, -1, -1, -1, -1, true);  // smooth with square of kernel            break;
    sprintf(infotext,"Filtered: hanning; filtersize (pix) = %i",filtersize);        
  case welch:
    stokesI[0].welchfilter(filtersize);
    stokesQ[0].welchfilter(filtersize);
    stokesU[0].welchfilter(filtersize);
    varQ[0].welchfilter(filtersize, -1, -1, -1, -1, true);
    varU[0].welchfilter(filtersize, -1, -1, -1, -1, true);  // smooth with square of kernel
    sprintf(infotext,"Filtered: Welch; filtersize (pix) = %i",filtersize);            
    break;
  case gauss:
  default:
    stokesI[0].gaussianfilter(sigma);
    stokesQ[0].gaussianfilter(sigma);
    stokesU[0].gaussianfilter(sigma);
    varQ[0].gaussianfilter(sigma, -1, -1, -1, -1, true);
    varU[0].gaussianfilter(sigma, -1, -1, -1, -1, true); // smooth with gaussian square
    sprintf(infotext,"Filtered: gaussian, sigma (pix) = %.3f",sigma);    
  }


  stokesI[0].puthistorytxt(infotext);
  stokesI[0].puthistorytxt("Stokes I map");
  
  stokesQ[0].puthistorytxt(infotext);
  stokesQ[0].puthistorytxt("Stokes Q map");
  
  stokesU[0].puthistorytxt(infotext);
  stokesU[0].puthistorytxt("Stokes U map");
  
  varQ[0].puthistorytxt(infotext);
  varQ[0].puthistorytxt("Stokes Q variance map");
  
  varU[0].puthistorytxt(infotext);
  varU[0].puthistorytxt("Stokes U variance map");      

  mdp99.puthistorytxt(infotext);
  mdp99.puthistorytxt("MPD99 map");
  
  pol_intensity.puthistorytxt(infotext);
  pol_intensity.puthistorytxt("Polarized intensity = sqrt(Q^2+U^2)");
  
  pol_fraction.puthistorytxt(infotext);
  pol_fraction.puthistorytxt("Polarized fraction = intensity/Stokes I");
  
  chiSqrMap.puthistorytxt(infotext);
  chiSqrMap.puthistorytxt("Chi^2 map (d.o.f 2) = Q^2/varQ + U^2/varU");
  
  
  polAngles.puthistorytxt(infotext);
  polAngles.puthistorytxt("Polarization angles");

  filtered = true;
  
  return success();
}



/****************************************************************************/
//
// FUNCTION    : applyMask
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
Status StokesMaps::applyMask(float clip)
{
  setFunctionID("applyMask");


  if( stokesI[0].getnx() <= 0 || stokesI[0].getny() <= 0 ||
      varU[0].getnx() <= 0 ||    varU[0].getny() <= 0 )
    return error("No Stokes data present");
  if( !dataSummed )
    return error("Maps were not summed...");
  if( !filtered)
    return error("Summed map was not smoothed...");    

  mask = counts[0];
  mask.gaussianfilter(2.0);

  mask.clip( clip, MAX_FLOAT);
  mask /= mask; // normalizes all non zero pixels

  message("Applying mask to summed and filtered Stokes maps. Lower limit = ",clip);

  stokesI[0] *= mask;
  stokesQ[0] *= mask;
  stokesU[0] *= mask;  
  
  varQ[0] *= mask;
  varU[0] *= mask;

  masked = true;
  
  return success();
}

/****************************************************************************/
//
// FUNCTION    : processMaps()

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
Status StokesMaps::processMaps()
{
  setFunctionID("processMaps");
  

  Image q_sqr, u_sqr, tmp;

  q_sqr = stokesQ[0];
  q_sqr *= stokesQ[0];
  pol_intensity = q_sqr;
  
  u_sqr = stokesU[0];
  u_sqr *= stokesU[0];
  pol_intensity += u_sqr;  

  pol_intensity.squareroot();

  
  pol_fraction = pol_intensity;
  pol_fraction /= stokesI[0];


  q_sqr /= varQ[0];
  u_sqr /= varU[0];

  chiSqrMap = q_sqr;
  chiSqrMap += u_sqr;


  mdp99  = varQ[0];
  mdp99 += varU[0];
  mdp99 *= 9.21*0.5;
  mdp99.squareroot();
  mdp99 /= stokesI[0];

  
  processed = true;
  
  return success();
}


/****************************************************************************/
//
// FUNCTION    : read[StokesMaps]
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
Status StokesMaps::read( string det1cubename, bool useDetName)
{
  setFunctionID("read[StokesMaps]");


  //  message("Reading data from data cube. Detector 1 filename: ", det1cubename);

  string cubename=det1cubename;
  
  for(int det=1; det<=3; det++){

    message("detector nr ", det);
    message("reading ", cubename);

    if( counts[det].read( cubename, 1, 3) == FAILURE ){
      return error("error reading counts maps from ", cubename.c_str());      
    }
    if( stokesI[det].read( cubename, 1, 9 ) == FAILURE ||
	stokesQ[det].read( cubename, 1, 11 ) == FAILURE ||
	stokesU[det].read( cubename, 1, 13 ) == FAILURE   ){
      return error("error reading stokes maps from ", cubename.c_str());
    }



    if( varI[det].read( cubename, 1, 10 ) == FAILURE ||
	varQ[det].read( cubename, 1, 12 ) == FAILURE ||
	varU[det].read( cubename, 1, 14 ) == FAILURE   ){
      return error("error reading variance maps from ", cubename.c_str());      

    }    

    message("Converting error maps to variance maps...");
    varI[det] *= varI[det]; 
    varQ[det] *= varQ[det];
    varU[det] *= varU[det];

    if( useDetName){
      message("Use 'det' as detector tag");
      if( det == 1 ){
	if (cubename.find("det1") != string::npos)
	  cubename.replace(cubename.find("det1"), 4, "det2");
      }else if (det == 2){
	if (cubename.find("det2") != string::npos)
	  cubename.replace(cubename.find("det2"), 4, "det3");      
      }//if

      
    }else{
      message("Use 'du' as detector tag");      
      if( det == 1 ){
	if (cubename.find("du1") != string::npos)
	  cubename.replace(cubename.find("du1"), 3, "du2");
      }else if (det == 2){
	if (cubename.find("du2") != string::npos)
	  cubename.replace(cubename.find("du2"), 3, "du3");      
      }//if
    }//if

    
  }//for

  if( sum_detectors() == FAILURE)
    return error("error summing images");
  
  return success();
}


/****************************************************************************/
//
// FUNCTION    : save()
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
Status StokesMaps::save( const string &basename, bool saveDets)
{
  setFunctionID("save[StokesMaps]");


  
  if( stokesI[3].getnx() <= 0 || stokesI[3].getny() <= 0 ||
      varU[3].getnx() <= 0 ||    varU[3].getny() <= 0 )
    return error("No Stokes data present");
  if( !dataSummed )
    return error("Maps were not summed...");

  char infotext[STRL];

  int detmax=0;
  if(saveDets)
    detmax=3;

  for(int i=0; i<= detmax; i++){

    string filename = basename;
    if( filtered )
      filename += "_filtered_";
    else
      filename += "_unfiltered_";
  
    if( i == 0 )
      filename += "sum";
    else
      filename += "det" + to_string(i);      

    //    sscanf(infotext,"Stokes I map, detector = %i ",i);
    //    stokesI[i].putshistorytxt();
    //    stokesI[i].puthistory("Stokes I map");
    
    stokesI[i].save( filename + "_stokesI.img.gz");
    stokesQ[i].save( filename +  "_stokesQ.img.gz");
    stokesU[i].save( filename + "_stokesU.img.gz");
    varQ[i].save( filename + "_stokesQvar.img.gz");
    varU[i].save( filename + "_stokesUvar.img.gz");
  }

  if( processed ){
    string filename = basename;
    if( filtered )
      filename += "_filtered_";
    else
      filename += "_unfiltered_";



    pol_intensity.setDetector(infotext);
    pol_intensity.save( filename + "pol_intens.img.gz");
    pol_fraction.save( filename + "pol_frac.img.gz");
    chiSqrMap.save( filename + "ChiSqrMap.img.gz");
    mdp99.save( filename + "MDP99.img.gz");
  }
  
  return success();
}

/****************************************************************************/
//
// FUNCTION    :   saveVectors
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//    basename          I                       First part of file name
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status StokesMaps::saveVectors( const string &basename, bool magneticvectors,
				int vectorskip, int vector, double minChiSqr)
{
  setFunctionID("saveVectors");

  Polarization polarization;
  string filename=basename + "_pol_vectors.reg";

  message("vector skip = ",vectorskip);
  message("min X^2 = ", minChiSqr);



  if( polAngles.polarizationVectorsX(filename, stokesQ[0], stokesU[0], //switch Q&U
				     chiSqrMap,
				     magneticvectors,
				     vectorskip,vector, minChiSqr)==FAILURE ){
    return error("error writing polarization vectors");
  }  

  
  return success();
}




/****************************************************************************/
//
// FUNCTION    : :polarizationVectors2
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
Status Image::polarizationVectorsX( const string &regionFileName,
				    const Image &stokesQ,
				    const Image &stokesU,
				    const Image &chiSqr,
				    bool magneticvectors,
				    int deltapix,
				    int vector,
				    float minChiSqr1, float minChiSqr2,				    
				    const string &color1, const string &color2
				  )
{
  setFunctionID("polarizationVectors");

  
  if( minChiSqr1 < 0.0 || minChiSqr2 <= minChiSqr1 ){
    return error("invalid input parameter, minimum X^2_2 = ", minChiSqr1);
  }
  if( deltapix > 32 || deltapix < 1 ){
    return error("invalid input parameter, delta pixel = ", deltapix);
  }
  
  if( stokesQ.nx <= 0 ||
      stokesQ.nx != stokesU.nx || stokesQ.nx != chiSqr.nx ){
    return error("incompatible images");
  }

  if( vector > 1 || vector <0){
    warning("invalid value for vector variable, will be set to 1; vector =", vector);
    vector=1;
  }
  
  *this = stokesQ; // copy structure/keywords
  *this *= 0.0; // initialize
  
  // *this will contain polarization angles

  FILE *fp;
  bool colorCode=true;

  if( !(fp=fopen(regionFileName.c_str(),"w"))){
    return error("Could not open region file for writing: %s", regionFileName.c_str());
  }

  //  fprintf(fp,"image\n");
  fprintf(fp,"fk5\n");

  bool isolatedpixel=false;
  string color="magenta";
  int i2, j2, width=2;
  float x1, y1, x2, y2, x0, y0, l=0.5*deltapix;
  double ra1, dec1, ra2, dec2, ra0, dec0;


  fprintf(fp,"# Region file format: DS9 version 4.1\n");
  fprintf(fp,"# Polarization directions\n");
  fprintf(fp,"global color=%s dashlist=8 3 width=1 font=\"helvetica 10 normal\" select=1 highlite=1 dash=0 fixed=0 edit=1 move=1 delete=1 include=1 source=1\n",color1.c_str());


  

  //  l=10;

  message("length of vectors = ", l);
  message("pixel skip = ", deltapix);
  
  float angle, q, u;
  for(int j=0; j< ny; j++){
    for(int i=0; i< nx; i++){


      if( chiSqr.pixel[i + j*nx] >= minChiSqr1 ){ //||
	//	  (colorCode &&  (chiSqr.pixel[i + j*nx] >= 0.001)) ){ //&&	  (i % deltapix == 0) && (j % deltapix == 0) ){
	q = stokesQ.pixel[i + j*nx];
	u = stokesU.pixel[i + j*nx];
	angle= 0.5* atan2(u,q);

	this->set(i+1,j+1, angle);
	
	if( !magneticvectors )
	  angle -= M_PI/2; // PA is wrt north
	
	if( angle < 0)
	  angle += M_PI;
	//	angle *= 0.5;
	//angle += M_PI/2.0; // to go from B to E-vectors

	
	i2=i;
	j2=i;
	// check neigboring pixels
	isolatedpixel=false;
	if( i > 0 && i < nx-1 && j > 0 && j < ny-1 ){
	  if( chiSqr.pixel[(i-1) + j*nx] < minChiSqr1 &&
	      chiSqr.pixel[(i+1) + j*nx] < minChiSqr1){
	    isolatedpixel=true;
	  }
	  if( chiSqr.pixel[i + (j-1)*nx] < minChiSqr1 &&
	      chiSqr.pixel[i + (j+1)*nx] < minChiSqr1 ){
	    isolatedpixel=true;
	  }
	      
	}

	if( true || !isolatedpixel){
	  
	  x0 = i + 1; //fits pixel count convention
	  y0 = j + 1; //fits pixel count convention
	  //	  l = deltapix * polDegree.pixel[i + j*nx]/0.5;
	  x2 = x0 + l * cos( angle);
	  y2 = y0 + l * sin( angle);
	  x1 = x0 - l * cos( angle);
	  y1 = y0 - l * sin( angle);
	  //	  fprintf(fp,"line( %.2f,%.2f,%.2f,%.2f) # width=3\n", x1,y1,x2,y2);

	  chiSqr.getradec(x0, y0, &ra0, &dec0);	  
	  chiSqr.getradec(x1, y1, &ra1, &dec1);
	  chiSqr.getradec(x2, y2, &ra2, &dec2);
	  //	  fprintf(fp,"fk5;line( %.5f,%.5f,%.5f,%.5f) # width=3\n", ra1,dec1,ra2,dec2);
	  ///	  printf("***************  angle = %f %f q=%f u=%f\n", angle * 180.0/M_PI, atan(u/q)*180./M_PI, q,u);
	  if( colorCode){
	    if(  chiSqr.pixel[i + j*nx] >= minChiSqr2 ){
	      color=color1;
	    }else{
	      //	      color="red";
	      color=color2;	      
	    }
	  }
	  if( (i % deltapix == 0) && (j% deltapix == 0) ){
	    fprintf(fp,"# vector(%f,%f,%f\",%f) vector=%i width=%i color=%s\n",
		    ra0,dec0,1.0*l*sqrt(fabs(cdelt1*cdelt1))*3600.0,angle*180/M_PI,vector,width,
		    color.c_str());
	    fprintf(fp,"# vector(%f,%f,%f\",%f) vector=%i width=%i color=%s\n",
		    ra0,dec0,1.0*l*sqrt(fabs(cdelt1*cdelt1))*3600.0,angle*180/M_PI+180.0,vector,width,
		    color.c_str());	  
	  //# vector(23:23:18.802,+58:51:37.296,143.253",46.94022) vector=1 width=4	  
	  }
	}
	
	pixel[i + j*nx] = angle * 180.0/M_PI;
      }//else a zero will be in place
      else{
	pixel[i + j*nx] = 0.0;
      }
    }

  }//for j

  fclose(fp);

  message("length of vectors = ", l);
  
  return success();
}

