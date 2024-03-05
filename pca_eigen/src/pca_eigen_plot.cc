/*****************************************************************************/
//
// FILE        :  pca_eigen_plot.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Implementation of the  Image class.
//                Basic functions.
//
// COPYRIGHT   : Jacco Vink, 2023
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_pca_eigen.h"
#include "jvtools_pgplot.h"

using namespace::jvtools;
using namespace::JVMessages;



/****************************************************************************/
//
// FUNCTION    :  plotComponents()
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
Status Pca::plotComponents(const string &device, int nmax)
{
  setFunctionID("plotComponents");


  if( !pcaComputed)
    return error("No PCA done yet");
  
  PGPlot window;


  message("plotting PCA components as function of energy/channel");
  if( useEnergies ){
    //    window.setBoundaries(energyBand[0], energyBand[nn-1], -0.5*singularValues[0], 0.5*singularValues[0]);
    window.setBoundaries(energyBand[0]*0.8, energyBand[nn-1]*1.2, -1.0, 0.75);    
    window.setXlog();
    window.setXlabel("Energy (keV)");
  }else{
    window.setBoundaries(0.0, float(nn), -1.0, 1.0);
    window.setXlabel("spectral element number");
  }

  window.setYlabel("component weight");
  window.setPlotFile( device.c_str() );
  window.open();

  window.setFontSize(0.8);

  string label;
  int n=MIN(nn,nmax);
  int color=PG_COLOR_DEFAULT;
  double yl=-0.5, dy=0.06;
  for(int i=0; i<n; i++){
    window.setColor(color);
    //    window.setYscaling(singularValues[i]);

    window.drawCurve(energyBand, eigenVectors+(i*nn), nn);

    label = "PC " + to_string(i+1);
    window.makeLegend(energyBand[0], energyBand[0]*1.3, yl, label);
    yl -= dy;
    color++;
  }
  return success();
}




/****************************************************************************/
//
// FUNCTION    : plotScores()
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
Status Pca::plotScores(int pc1, int pc2, const string &device,
		       float min1, float max1, float min2, float max2)		       
{
  setFunctionID("plotScores");
  
  if( !pcaComputed || !keepScores)
    return error("No PCA done yet, or data stored");


  if( (pc1 >= pc2) || pc2 >= nn)
    return error("invalid PC scores requested");

  PGPlot window;
  string xlabel = "PC " + to_string(pc1+1);
  string ylabel = "PC " + to_string(pc2+1);  
  int nx, ny;

  if (min1 == -1 && min2 == -1 && max1 == -1 && max2 == -1 ){
    min1 = pcaScores[pc1].minimum();
    max1 = pcaScores[pc1].maximum();
    min2 = pcaScores[pc2].minimum();
    max2 = pcaScores[pc2].maximum();
  }

  
  pcaScores[pc1].getnxny(&nx, &ny);

  window.setPlotFile( device.c_str() );
  window.setBoundaries( min1, max1, min2, max2);
  //window.setBoundaries( -0.1, 0.1, -0.1, 0.1);
  window.setXlabel( xlabel.c_str());
  window.setYlabel( ylabel.c_str());

  //window.open();
  //window.open(false, false, 0.0, 0.0,0,0,true);
  window.open(false, false, 0.0, 0.0,0,0,false);

  window.setSymbolSize(0.7);

  
  window.drawPoints( pcaScores[pc1].pixel, pcaScores[pc2].pixel, pcaScores[pc2].pixel, nx*ny,
		     false);

  if( select ){
    Image impc1=pcaScores[pc1];
    Image impc2=pcaScores[pc2];

    //    message("plotting selection; #pixels = ", selectionMask.total());
    printf("\t*** plotting selection; #pixels = %.0f mm=%i\n", selectionMask.total(),mm);
    impc1 *= selectionMask;
    impc2 *= selectionMask;



    //    printf("\t*** Selected images, totals = %f %f\n", impc1.total(), impc2.total());

    window.setColor(PG_COLOR_RED);
    window.setSymbolNumber(PGPLOT_X);    
    window.drawPoints( impc1.pixel, impc2.pixel, impc2.pixel, nx*ny, false);

  }else{
    //    printf("\t*** No selection\n");
  }
  
  return success();
}


/****************************************************************************/
//
// FUNCTION    : plotEigenvalues
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
Status Pca::plotEigenValues( const string &device)
{
  setFunctionID("plotEigenValues");


  double *eigenValues=new double [nn];
  double *components=new double [nn];


  for(int i=0;i<nn; i++){
    eigenValues[i] = SQR( singularValues[i] );
    components[i] = i+1;
  }



  PGPlot window;

  window.setPlotFile( device.c_str() );
  window.setBoundaries(0.0, nn, 0.1, eigenValues[0]);
  window.setXlabel("Principal component");
  window.setYlabel("Eigen values");
  window.setYlog();
  window.open();

  window.drawCurve( components, eigenValues, nn);
    
  delete [] eigenValues;
  delete [] components;
  
  return success();
}



/****************************************************************************/
//
// FUNCTION    : imageScores
// DESCRIPTION : Stores the histogram of pc1 & pc2 as a fits-image
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  pc1, pc2            I   The two components to be used as X/Y axis (int)
//   filename           I   The filename under which the image is stord
//  useSelectionMask    I   Only plot those points that are in the selection mask (true/false)
//  smoothresults       I   Boolean: smooth image with gaussian or not?
//  applySelection      I   Suppress scores if not in selected image.
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Pca::imageScores(int pc1, int pc2, const string &filename,
			bool useSelectionMask, bool smoothresults,
			float applySelection )
{
  setFunctionID("imageScores");
  
  if( !pcaComputed || !keepScores)
    return error("No PCA done yet, or data stored");


  if( (pc1 >= pc2) || pc2 >= nn)
    return error("invalid PC scores requested");

  int nx2=128;
  int ny2=128;
  Image pcScores(nx2, ny2);
  Image *maskptr = &mask;  
  float min1 = pcaScores[pc1].minimum();
  float max1 = pcaScores[pc1].maximum();
  float min2 = pcaScores[pc2].minimum();
  float max2 = pcaScores[pc2].maximum();   
  float crpix1= 0.5*nx2 + 0.5; //fits convention
  float crpix2= 0.5*ny2 + 0.5; //fits convention
  float crval1= 0.5*(min1 + max1);
  float crval2= 0.5*(min2 + max2);  
  float dx= (max1-min1)/(nx2 - 1);
  float dy= (max2-min2)/(ny2 - 1);
  float sigma=1.0;
  //  message("image pixel scale dx = ",dx);
  //  message("image pixel scale dy = ",dy);


  message("PC1 = ", pc1+1);
  message("PC2 = ", pc2+1);
  message("PC1 min = ",min1);
  message("PC1 max = ",max1);
  message("PC2 min = ",min2);
  message("PC2 max = ",max2);  

  pcScores.setCoordinates( crval1, crval2, crpix1, crpix2, dx, dy,
			   0.0, 2000.0, "LINEAR", "LINEAR","LINEAR");

  if( useSelectionMask && select)
    maskptr= &selectionMask;
  
  int nx, ny, k, l;
  float valpc1, valpc2, val;
  pcaScores[pc1].getnxny(&nx, &ny);


  for(int j=0; j<ny; j++){
    for(int i=0; i<nx; i++){
      if( maskptr->get(i+1,j+1) > 0.5 ){
	valpc1 = pcaScores[pc1].get(i+1,j+1);
	valpc2 = pcaScores[pc2].get(i+1,j+1);      
	k= (int)((valpc1-crval1)/dx   + crpix1 + 0.5);
	l =(int)((valpc2-crval2)/dy   + crpix2 + 0.5);
	val =  pcScores.get(k,l) + 1.0;
	pcScores.set( k, l, val);
      }
    }//for i
  }//for j


  if( smoothresults){
    pcScores.gaussianfilter(sigma);
  }
			    
  pcScores.save(filename);

  if( applySelection > 0.0 && useSelectionMask){
    message("Using selected PC points and apply these to PC scores. Threshold = ",applySelection);

    Image pcascore1=pcaScores[pc1];
    Image pcascore2=pcaScores[pc2];

    //    pcascore1.save("before1.img.gz");
    //    pcascore2.save("before2.img.gz");
      
    for(int j=0; j<ny; j++){
      for(int i=0; i<nx; i++){
	valpc1 = pcascore1.get(i+1,j+1);
	valpc2 = pcascore2.get(i+1,j+1);
	k= (int)((valpc1-crval1)/dx   + crpix1 + 0.5);
	l =(int)((valpc2-crval2)/dy   + crpix2 + 0.5);
	val =  pcScores.get(k,l);
	if( val  < applySelection ){
	  pcascore1.set(i+1,j+1, 0.0);
	  pcascore2.set(i+1,j+1, 0.0);
	}
	
      }//for
    }//for

    //    pcascore1.save("after1.img.gz");
    //    pcascore2.save("after2.img.gz");     
    
    string maskName= outputName + "_pca_score_" + to_string(pc1+1)  + "_w" +  to_string(pc1+1) + "_selected_mask.img.gz";
    //    puts(fileName.c_str());
    message("saving ", maskName);
    pcascore1 /=  pcascore1;
    pcascore1.save(maskName);
    
    
    maskName= outputName + "_pca_score_" + to_string(pc2+1) + "_w" +  to_string(pc1+1) + "_selected_mask.img.gz";
    message("saving ", maskName);    

    pcascore2 /=  pcascore2;
    pcascore2.save(maskName);    
  }//if

  
  return success();
}
