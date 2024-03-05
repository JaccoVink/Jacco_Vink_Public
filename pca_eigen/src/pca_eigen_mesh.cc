/*****************************************************************************/
//
// FILE        :  pca_eigen_mesh.cc
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
// FUNCTION    : createMeshIndex()
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
Status Pca::createMeshIndex( )
{
  setFunctionID("createMeshIndex");

  if( nn < NMIN)
    return error("not enough input images for PCA, m = ",nn);

#if 0
  if( nlevels <= 1 || nlevels > 10 ){
    return error("invalid request for mesh refinement levels: ", nlevels);
  }
  levels=nlevels;
#endif
  
  message("Creating mesh from total counts of the summed images");
  message("Mesh levels      = ",levels);
  message("Minimum #counts/pixel for mesh refinement = ",minCounts);
  
  Image bandimage, meshedimage;

  warning("Besides mesh refinement the basic bin factor is set to ", binfactor);

  
  for(int i=0; i < nn; i++){
    //    message("reading image file ", inputImageNames[i]);

    if( bandimage.read( inputImageNames[i] ) == FAILURE ){
      return error("could not read image ",inputImageNames[i].c_str());
    }//i

    if( binfactor > 1 ){
      message("rebinning image by factor ", binfactor);
      if( bandimage.rebin(binfactor) == FAILURE)
	return error("rebinning error");
    }//if
    

    if( i == 0 )
      total = bandimage;
    else
      total += bandimage;

    
  }//for

  if( clipMin > 0.0){
    warning("Clipping away pixels with values below ",clipMin);
    total.clip( clipMin, 1.0e36);
  }

  message("Performing mesh level refinement");
  if( total.meshrefinement( &meshedimage, &indexmap, &binMap, minCounts, levels) == FAILURE){
    return error("Mesh refinement problem");
  }


  mm=(int)indexmap.maximum();

    
  if( determine_index_to_pixel(  )==0 )
    return error("index to pixel mapping error");


  if( mm < NMIN+1)
    return error("number of nonzero mask pixels (m) is insufficient");

  dataMatrix.resize(mm,nn);
  
  
  message("nx = ",nx);
  message("ny = ",ny);  
  message("Principal Component Analysis: #rows    m =",mm);
  message("Principal Component Analysis: #columns n =",nn);



  mask = indexmap;
  mask /= indexmap;
  
  indexmap.save( outputName + string("_index_map.img.gz"));
  meshedimage.save(outputName + string("_total_meshed_map.img.gz"));
  binMap.save(outputName + string("_total_bin_map.img.gz"));

    
  //  return error("xxx");
  return success();
}


/****************************************************************************/
//
// FUNCTION    : determine_index_to_pixel( )
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
Status Pca::determine_index_to_pixel( )//const Image &indexmap)
{
  setFunctionID("determine_index_to_pixel");

  
  if( mm <= 1){
    return error("invalid array length, ",mm);
  }

  indexmap.getnxny( &nx, &ny);
  if( nx <= 0 || ny <= 0 )
    return error("invalid index map");
  
  message("allocate array");
  if( index_to_pix != 0)
    delete [] index_to_pix;
  
  index_to_pix = new int [mm]();

  
  message("Making index-mapping array...");

  

  
  int npix=nx*ny;
  
  for( int i=1; i<= mm; i++){ // for each index number
    int pixnr = 0;
    int index=-1;
    int j=0;
    for(j=0; j<npix && index != i;j++){
      //    index=int(indexmap.get(j));
      index=int(indexmap.pixel[j]);	    
    }//for
    
    if( index == i ){
      index_to_pix[i-1] = j-1;
    }


    
  }//for i



  message("Mapping completed...");
  
  return success();
}


/****************************************************************************/
//
// FUNCTION    :   processInputFile_mesh(const string& inputfile);
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
Status Pca::processInputFile_mesh(const string& inputfile)
{
  setFunctionID("processInputFile_mesh");

  if( readInputFile(inputfile) == FAILURE)
    return error("error reading ",inputfile.c_str());
  

  if(  createMeshIndex()==FAILURE ){
    return error("Error creating index map");
  }
  
  if( mm < NMIN+1 || nn < NMIN){
    return error("number of data entries appear insufficient");
  }


  message("Matrix allocation:");
  message("#rows = ", dataMatrix.rows() );
  message("#columns = ", dataMatrix.cols());


  int chanmin=0, chanmax=0;
  double mean=0.0;
  Image bandimage;


  for(int k=0; k <nn; k++){ // nn is number of spectral bins

    message("Processing ",  inputImageNames[k] );
    if( bandimage.read( inputImageNames[k] ) == FAILURE ){
      return error("could not read image ",inputImageNames[k].c_str());      
    }//if

    if( binfactor > 1 ){
      message("rebinning image by factor ", binfactor);
      if( bandimage.rebin(binfactor) == FAILURE)
	return error("rebinning error");
    }
    
    if( bandimage.meshedIndexBinning(indexmap, binMap, &mean ) == FAILURE){
      return error("error with meshed index binning");
    }
    spectralMean[k]=mean;
    //    message("Meshed image mean = ",mean);

    bandimage.getChanMinMax(&chanmin, &chanmax);
    if( ( (chanmin > 0) || (chanmax > 0)) && (chanmax > chanmin) ){
      energyBand[k] = 0.5*(chanmin + chanmax)/1000.0; // assuming channel = E in eV
      if( k==0 )
	useEnergies=true;
      message("<E> = ", energyBand[k]);                
    }else{
      energyBand[k]=double(k+1);

      if( k==0 )
	useEnergies=false;
    }

    message("Transferring data to data matrix for level = ",k);
    message("# data entries = ",mm);
    message("Mean value per pixel = ",spectralMean[k]);

    double pixvalue=0.0, id=0.0, binf=0.;
    double variance=0.0;
    for(int index=1; index < mm; index++){

      pixvalue = bandimage.get( index_to_pix[index-1]  );
      binf = binMap.get( index_to_pix[index-1]  );
      //      if( index % 2000 == 0 ){
      //	id = indexmap.get(  index_to_pix[index-1]  );
      //	printf("%i %.0f %.0f \n",index, id, binf);
      //      }
      dataMatrix(index-1, k) = pixvalue*binf - spectralMean[k];	 // index -1 as datamrtix starts counting at 0
      variance += SQR(  pixvalue*binf - spectralMean[k]);
    }


    variance /= mm-1.0;
    spectralVariance[k] = variance;
    if( normalize ){
      message("Normalizing matrix...");
      for(int i=0; i<mm; i++){
	dataMatrix(i, k) = dataMatrix(i, k)/sqrt(variance);
      }
    }

    
  }//for k
  message("Finished data gathering...");

  
  
  
  return success();
}

/****************************************************************************/
//
// FUNCTION    : savePCAscores_mesh()
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
Status Pca::storePCAscores_mesh( bool savemasks, bool hardcopy, int pcmax)
{
  setFunctionID("savePCAscores_mesh");




  int npix=nx*ny, index;
  double value, binf;
  string fileName;
  Image pca_score;
  pca_score = indexmap; //ensures proper keyword values


  message("Number of image pixels = ", npix);
  //  return error("no code yet");
  
  for(int k=0; k<nn;k++){

    message("Score #",k+1);
    pca_score *= 0.0;

    
    for(int j=0; j<npix; j++){
      index = (int)indexmap.get( j);
      if( index > 0){
	//	  binf = binMap.get( j );	  
	value = scoreMatrix( index-1, k);///binf;
	if( pca_score.set( j, value) == FAILURE){
	  return error("should not occur");
	}
      }
    }//for

    if( hardcopy && k<pcmax){
      fileName= outputName + "_mesh_pca_score_" + to_string(k+1) + ".img.gz";
      //    puts(fileName.c_str());
      message("saving ", fileName);
      pca_score.save(fileName);
    }

    if( 0 && savemasks ){
      Image minmask, maxmask;
      double min=pca_score.minimum(), max=pca_score.maximum();
      double median = 0.5*( min + max), frac= 0.90;

      message("saving min/max masks");
      minmask = pca_score;
      //      minmask -= median;
      //
      minmask.clip( 1.1*min,(1.0-frac)*min );
      //      minmask *= mask;
      minmask /= minmask;
      
      fileName= outputName + "_pca_score_" + to_string(k+1) + "_min_mask.img.gz";
      minmask.save(fileName);
      
      maxmask = pca_score;
      //      maxmask -= median;
      //      maxmask.clip( (1.0-frac)*(max-median), 1.1*(max-median));
      maxmask.clip( (1.0-frac)*max, 1.1*max);      
      //      maxmask *= mask;
      maxmask/= maxmask;
      
      fileName= outputName + "_pca_score_" + to_string(k+1) + "_max_mask.img.gz";
      maxmask.save(fileName);
    }else{
      message("No masks are saved...");
    }
    
    if( keepScores ){
      message("Storing pca score in memory.");
      pcaScores[k] = pca_score;
    }    
  }
  
  return success();
}
