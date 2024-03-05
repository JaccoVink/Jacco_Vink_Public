/*****************************************************************************/
//
// FILE        :  pca_eigen.cc
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

/*****************************************************************************/
//
// FUNCTION    : Pca()
// DESCRIPTION : constructor
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/*****************************************************************************/
Pca::Pca( const string& outputname )
{
  nx=ny=0;
  nn=mm=0;
  
  clipMin=0.0;
  minCounts=500.0;
  binfactor=2;

  normalize = true;
  pcaComputed = false;
  useEnergies = false;
  keepScores = false;
  select = false;
  useMesh=false;
	      
  levels=3;
  
  outputName=outputname;
  index_to_pix = 0;
  
  return;
}//Pca

Pca::~Pca( void  )
{
  deallocate();

  if(   index_to_pix != 0 && mm != 0){
    delete [] index_to_pix;
    mm =0;
  }
  
  return;
}//Pca


#define STRL 1024




/****************************************************************************/
//
// FUNCTION    : allocate
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
Status Pca::allocate(int n)
{

  setFunctionID("allocate");


  if( n < NMIN ){
    return error("n < ",NMIN);
  }

  inputImageNames = new string [n];
  spectralMean = new double [n];
  spectralVariance = new double [n];
  eigenVectors = new double [n*n];
  energyBand = new double [n];
  singularValues = new double [n];
  

  pcaScores = new Image [n]; // note that this only makes the objects with zero length arrays
  
  nn = n;
  
  return success();
}

Status Pca::deallocate( )
{
  setFunctionID("deallocate");
  if( nn > 0){
    delete [] inputImageNames;
    delete [] spectralMean;
    delete [] spectralVariance;
    delete [] eigenVectors;
    delete [] energyBand;
    delete [] singularValues;
    delete [] pcaScores;
    nn=0;
  }

    
  return success();
}




/****************************************************************************/
//
// FUNCTION    :   processInputFile(const string& inputfile);
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
Status Pca::readInputFile(const string& inputfile)
{
  setFunctionID("readInputFile");

  FILE *fp;
  if( !(fp=fopen(inputfile.c_str(), "r")) ){
    return error("could not open file ", inputfile.c_str());
  }

  message("opened input file ", inputfile.c_str());
  int nlines=0;
  char line[STRL], filename[STRL];
  while( fgets(line,STRL, fp) && (strncasecmp(line,"END",3)!= 0) ){
    nlines++;
  }

  message("#entries (input images)  = ",nlines);
  rewind(fp);

  if( allocate(nlines) == FAILURE ){
    return error("invalid number of images, minimum = ",NMIN);
  }

  
  for(int i=0; i <nn; i++){
    fgets(line,STRL,fp);
    sscanf(line,"%s",filename);
    inputImageNames[i] = filename;
    message("image file ", inputImageNames[i]);
  }//for

  fclose(fp);
  
  return success();
}







/****************************************************************************/
//
// FUNCTION    :   createMask()
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
Status Pca::createMask()
{
  setFunctionID("createMask()");

  if( nn < NMIN)
    return error("not enough input images for PCA, m = ",nn);
  
  message("Creating mask from total counts of the summed images");
  message("Rebin factor          = ",binfactor);
  message("Minimum #counts/pixel = ",minCounts);

  
  Image bandimage;

  
  
  for(int i=0; i < nn; i++){
    //    message("reading image file ", inputImageNames[i]);

    if( bandimage.read( inputImageNames[i] ) == FAILURE ){
      return error("could not read image ",inputImageNames[i].c_str());
    }//i

    if( binfactor > 1 ){
      //      message("rebinning by factor ", binfactor);
      bandimage.rebin(binfactor);
    }
    
    if( i == 0 )
      total = bandimage;
    else
      total += bandimage;

    
  }//for


  mask=total;
  mask.clip(minCounts, 1.0e37);
  mask /= mask;

  int nerode=3;
  message("dilating/eroding mask in order to remove isolate mask pixels");
  
  for(int i=0;i<nerode; i++)
    mask.erode(1.0);  
  for(int i=0;i<nerode; i++)
    mask.dilate(1.0);  

  mask.save( outputName + string("_mask.img.gz"));


  mm = mask.total();  // mm is the number of spectra/pixels to be investigated
                      // nn is the number of spectral bins
  mask.getnxny( &nx, &ny);
  message("nx = ",nx);
  message("ny = ",ny);  
  message("Principal Component Analysis: #rows    m =",mm);
  message("Principal Component Analysis: #columns n =",nn);


  if( mm < NMIN+1)
    return error("number of nonzero mask pixels (m) is insufficient");

  dataMatrix.resize(mm,nn);

  //  message("saving index map...");
  //  indexmap *= mask;
  //  indexmap.save( outputName + string("_index.img.gz"));
  
  total *= mask;
  total.save(outputName + string("_total_binned_map.img.gz") );


    
  
  return success();
}





/****************************************************************************/
//
// FUNCTION    :   processInputFile(const string& inputfile);
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
Status Pca::processInputFile(const string& inputfile)
{
  setFunctionID("processInputFile");

  if( useMesh ){
    if( processInputFile_mesh(inputfile) == FAILURE){
      return error("error reading ",inputfile.c_str());      
    }
    return success();
  }else{
    if( readInputFile(inputfile) == FAILURE){
      return error("error reading ",inputfile.c_str());
    }
    if( createMask() == FAILURE )
      return error("error creating mask image");    
  }//else


  
  if( mm < NMIN+1 || nn < NMIN){
    return error("number of data entries appear insufficient");
  }
  
  Image bandimage, meanimage, variance;

  //  message("Start reading and processing image files...");
  message("#rows = ", dataMatrix.rows() );
  message("#columns = ", dataMatrix.cols());


  message("Reading image data and storing it in a matrix... ");
  message("Number of pixels = ",mm);

  //  setChatLevel(NO_MESSAGES);
  if( !useMesh )
    indexmap = mask;

  
  int chanmin,chanmax;
  double mean;

  for(int k=0; k <nn; k++){ // nn is number of spectral bins

    message("processing ",  inputImageNames[k] );
    if( bandimage.read( inputImageNames[k] ) == FAILURE ){
      return error("could not read image ",inputImageNames[k].c_str());      
    }//if

    if( binfactor > 1 ){
      message("rebinning by factor ", binfactor);
      bandimage.rebin(binfactor);
    }

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

    
    bandimage *= mask; // set all pixels outside mask to zero
      //    message("Spectral band's total  = ", bandimage.total());    
    spectralMean[k] = bandimage.total()/mm;    
    meanimage = mask; // contains zeroes and ones
    meanimage *= spectralMean[k]; // contains zeroes and mean values;
    
    bandimage -= meanimage; // now the image is centered
      
      
    setChatLevel (CHATTY);
      //    message("Spectral band's total after centering = ", bandimage.total());
      
      
    if( normalize ){
      variance = bandimage;
      variance *= bandimage;
      
      spectralVariance[k] = variance.total()/(mm-1);
      message("Spectral band's mean = ",  spectralMean[k]);
      message("Spectral band's std deviation = ", sqrt(spectralVariance[k]));
      
      bandimage /= sqrt( spectralVariance[k] );
      
    }
      
      
      //bandimage.save("firstimage_centered.img.gz");
    long l=0;
    double value;
    for(int j=0; j<ny; j++){
      for(int i=0; i<nx; i++){
	if( mask.get(i+1,j+1) >= 1.0 ){ //nb fits convention!
	  if( k == nn-1) // k refers to spectral vin
	    indexmap.set(i+1,j+1,float(l) );
	  //	  mean = total.get(i+1, j+1)/mm;
	  value = bandimage.get(i+1,j+1);
	  dataMatrix(l,k) = value; //bandimage.get(i+1,j+1); // l: pixel number, k = spectral bin
#if 0
	  value *=  sqrt( spectralVariance[k] );
	  value += spectralMean[k];
	  if( value < 10.0 ){
	    printf("\t\t*** Low pixel value detected for (%3i,%3i): %.1f\n", i+1, j+1, value);
	  }
#endif
	    
	  l++;
	}
	
      }
    }//for

  


    
  }//for k

  
  //  indexmap.save( outputName + string("_index.img.gz"));

  //  if( index != 0 ){
  //    delete [] index_to_pix;
  //  }


  
  //  return error("xxx");
  return success();
}



/****************************************************************************/
//
// FUNCTION    : SVD()
// DESCRIPTION : 
//
//     D=dataMatrix
//     decomposition D=
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
Status Pca::PCA()
{

  setFunctionID("PCA");

  message("Performing SVD");
  //  cout << "Here is the data matrix:" << endl << dataMatrix << endl;

  //  svd1.compute(dataMatrix, Eigen::ComputeFullU | Eigen::ComputeFullV);
  svd2.compute(dataMatrix, Eigen::ComputeThinU | Eigen::ComputeThinV);
  pcaComputed=true;

  //https://stats.stackexchange.com/questions/134282/relationship-between-svd-and-pca-how-to-use-svd-to-perform-pca
  // the columns of V  are principal directions/axes (eigenvectors).

  //cout << "Its right singular vectors are the columns of the thin V matrix:" << endl << svd2.matrixV() << endl;
  cout << "V size is " <<  svd2.matrixV().rows() << " x " <<  svd2.matrixV().cols() << endl;
  cout << "U size is " <<  svd2.matrixU().rows() << " x " <<  svd2.matrixU().cols() << endl;  
  cout << "n = " << nn << endl;
  
  for(int j=0; j<nn; j++){// eigen vector number=column


    for(int i=0; i<nn; i++){ // element
      eigenVectors[j*nn + i] =  svd2.matrixV()(i,j);
      //      printf(" %6.3f ", eigenVectors[j*nn + i]);
    }
    //    printf("\n");
  }//for


  printf("\t**singular values:\n");
  printf("\t  PC  \t   SV   \t   SV normalized\n");
  for(int i=0; i<nn;i++){
    singularValues[i]= svd2.singularValues()(i,0);
    printf("\t %3i \t %6.3f \t %6.4f\n", i+1, singularValues[i], singularValues[i]/singularValues[0]);
  }
  

  scoreMatrix = svd2.matrixU();/// * svd2.matrixS(); //singularValues();

  cout << "score matrix size is " <<  scoreMatrix.rows() << " x " <<  scoreMatrix.cols() << endl;  
  
#if 0
  //  cout << "Its singular values are:" << endl << svd1.singularValues() << endl;
  //  cout << "Its singular values are:" << endl << svd2.singularValues() << endl;  
  //  cout << "Its left singular vectors are the columns of the thin U matrix:" << endl << svd.matrixU() << endl;

  //  Vector3f rhs(1, 0, 0);
  //  cout << "Now consider this rhs vector:" << endl << rhs << endl;
  //  cout << "A least-squares solution of m*x = rhs is:" << endl << svd.solve(rhs) << endl;  
#endif
  
  return success();
}

/****************************************************************************/
//
// FUNCTION    : savePCAscores()
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
Status Pca::storePCAscores( bool savemasks, bool hardcopy, int pcmax)
{
  setFunctionID("savePCAscores");

  if( pcaComputed == false ){
    return error("No PCA computed");
  }

  if( useMesh ){
    return storePCAscores_mesh(savemasks, hardcopy,pcmax);
  }


  string fileName;
  Image pca_score;
  pca_score = mask; //ensures proper keyword values
  for(int k=0; k<nn;k++){

    

    int index=0;
    for( int j=0; j<= ny; j++){
          for( int i=1; i<= ny; i++){
	    if( mask.get(i+1, j+1) > 0.5 ){
	      //	      pca_score.set(i, j, singularValues[k]*scoreMatrix(index, k) );
	      pca_score.set(i+1, j+1, scoreMatrix(index, k) );	      
	      index++;
	    }
	  }
    }
    //    printf("k=%2i mm = %i\n", k,index);

    if( hardcopy && k<pcmax){
      fileName= outputName + "_pca_score_" + to_string(k+1) + ".img.gz";
      //    puts(fileName.c_str());
      message("saving ", fileName);
      pca_score.save(fileName);
    }

    if( savemasks ){
      Image minmask, maxmask;
      double min=pca_score.minimum(), max=pca_score.maximum();
      //      double median = 0.5*( min + max), frac= 0.90;
      double frac= 0.90;

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



/****************************************************************************/
//
// FUNCTION    : reconstruct
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
Status Pca::reconstruct(int iband, int pcmax, int pcskip)
{
  setFunctionID("reconstruct");

  
  if( !pcaComputed )
    return error("No PCA computed");
  if( pcmax < 1 || pcmax > nn )
    return error("invalid dimension: ",pcmax);
  if( keepScores == false ){
    return error("Scores were not saved.");
  }
  if( iband <= 0 || iband > nn ){
    return error("invalid band request: ", iband);
  }



  message("Creating and saving reconstructed image, #", iband);
  
  iband -= 1;
  pcmax -= 1; //c++ convention: arrays start at zero
  pcskip -= 1;

  Image reconstruction, pcascore;//=mask;



  reconstruction = mask;
  reconstruction *= 0.0;
  
  int i=0;
  do{

    if( i != pcskip ){
      pcascore = pcaScores[i];
      pcascore *= singularValues[i];
      pcascore *= eigenVectors[iband + i*nn];
      reconstruction += pcascore;    
    }else
      message("skipping PC ", pcskip);
    i++;
  }while( i < pcmax);
  


  if( normalize){
    message("multiplying by normalization");
    reconstruction *= sqrt(spectralVariance[iband]);
  }

  
  reconstruction += spectralMean[iband];

  if( useMesh )
    reconstruction /= binMap;
  else
    reconstruction *= mask;


  
  
  string fileName=
    outputName + "_pca_reconstruction_" + "band_" + to_string(iband+1) + "_max_pc_" + to_string(pcmax+1)+ ".img.gz";

  if( pcskip >= 0 ){
    fileName = outputName + "_pca_reconstruction_" + "band_" + to_string(iband+1) + "_max_pc_" + to_string(pcmax+1)+ "_skip_" + to_string(pcskip+1) + ".img.gz";    
  }

  message("Saving ",fileName);
  reconstruction.save(fileName);


  
  return success();
}

/****************************************************************************/
//
// FUNCTION    : readSelectionMask
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
Status Pca::readSelectionMask( const string& inputfile)
{
  setFunctionID("readSelectionMask");


  message("reading mask file ", inputfile);
  if( selectionMask.read(inputfile) == FAILURE){
    select=false;
    return error("error reading mask: ",inputfile.c_str());
  }

  if( binfactor > 1 ){
    message("It is assumed mask has similar WCS coordinates as input images. Rebinning it with binfactor ",binfactor);
    selectionMask.rebin(binfactor);
    selectionMask /= selectionMask; // ensures that fractional values are rounded to 1
  }//if
  
  select=true;
  
  //selectionMask.save("testmask.img.gz");

  return success();
}

