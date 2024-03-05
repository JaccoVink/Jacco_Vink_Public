/*****************************************************************************/
//
// FILE        :  pca_im.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  A new implementation of PCA of images.
//               Image are assumed to be on a common grid.
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
#include <iostream>
#include "Eigen/Dense"
#include "jvtools_pca_eigen.h"
 
using Eigen::MatrixXd;


int main(int argc, char *argv[] )
{

  if( argc < 3){
    printf("%s expects at least 2 input names: inputfilename outputname\n", argv[0]);
    return 1;
  }

  printlogo( argv[0]);



  int binfactor=1, levels=3;
  double minCnts=500.0, clipmin=0.0, pcselect=1.0;
  bool chatty=false, savemasks=false, savescores=false, reconstruct=false;
  bool plotScorePoints=false, greenmonster=false;
  const string inputFileName=argv[argc-2];
  const string outputName=argv[argc-1];
  string inputMaskName="none";
  Pca pca(outputName);

  if( argc > 3){
    for(int i=1; i<argc-2; i++){
      if( strncasecmp("-minc=",argv[i],6) == 0){
	minCnts=atof(argv[i]+6);
	pca.setMinCounts(minCnts);
      }else if( strncasecmp("-clipmin=",argv[i],9) == 0){
	clipmin = atof( argv[i]+9);
	pca.setClipMin(clipmin);
      }else if( strncasecmp("-pcsel=",argv[i],7) == 0){
        pcselect = atof( argv[i]+7);
      }else if( strncasecmp("-bf=",argv[i],4) == 0){
	binfactor=atoi(argv[i]+4);
	pca.setBinFactor(binfactor);
      }else if( strncasecmp("-chatty",argv[i],6) == 0){
	chatty=true;
      }else if( strncasecmp("-mask=",argv[i],6) == 0){
	inputMaskName = argv[i]+6;
      }else if( strncasecmp("-savemasks",argv[i],10) == 0){
	savemasks=true;
      }else if( strncasecmp("-savescores",argv[i],11) == 0){
	savescores =true;
      }else if( strncasecmp("-plotscores",argv[i],11) == 0){
	plotScorePoints=true;
      }else if( strncasecmp("-reconstruct",argv[i],6) == 0){
	reconstruct=true;
      }else if( strncasecmp("-mesh",argv[i],5)==0 ){
	pca.toggleUseMesh();
	if( strncasecmp("-mesh=",argv[i],6)==0 ){
	  levels=atof(argv[i]+6);
	  pca.setMeshLevels(levels);
	}
      }else if( strncasecmp("-greenmonster", argv[i],10) == 0){
	greenmonster=true;
      }
    }//for
  }//if

  

  if( pca.processInputFile(inputFileName )== FAILURE ){
    puts("*** Program Ended ***");
    return 2;
  }

  if( inputMaskName != "none" ){
    pca.readSelectionMask(inputMaskName );
  }
  
  pca.PCA();
  if( chatty )
    setChatLevel(JVMessages::CHATTY);
  pca.toggleKeepScores();
  pca.storePCAscores(savemasks, savescores,13);
  //  pca.storePCAscores(false,false);


#if 1
  printf("\t*** Making plots...");
  string componentsPlot=outputName + "_pca_components.ps/cps";
  pca.plotComponents( componentsPlot,8);

  string evPlot=outputName + "_pca_eigenvalues.ps/cps";
  pca.plotEigenValues( evPlot);
#endif

  if( reconstruct ){
    for(int i=0; i< 86; i++){
      if( i % 4 ==0 ){
	//	pca.reconstruct(i+1,6);
	pca.reconstruct(i+1,3);
	pca.reconstruct(i+1,8);
	pca.reconstruct(i+1,12);		
      //    pca.reconstruct(i+1,10);
	//	pca.reconstruct(i+1,40 );        
	  //      pca.reconstruct(i+1,28,5);
      }
    }//for

    JVMessages::message("*****");
  }//if

  


  
  if( plotScorePoints ){
    JVMessages::message("PC selection number = ", pcselect);
  // plot scores of first 10 scores
    for(int i=0; i<7; i++){
      for(int j=i+1; j<i+3; j++){
	string scorePlot= outputName + "_pca_scores_pc_" + to_string(i+1) + "_" + to_string(j+1) + ".ps/cps";
	if( !greenmonster ){
	  pca.plotScores( i, j, scorePlot);
	}else{
	  float min1=-1, max1=-1, min2=-1, max2=-1;
	  if( i == 0 ){
	    min1=-0.006;
	    max1=0.02;
	  }
	  if( i == 2 ){
	    min1=-0.04;
	    max1=0.04;
	  }
	  if( i == 4 ){
	    min1=-0.04;
	    max1=0.04;
	  }
	  if( i == 6 ){
	    min1=-0.04;
	    max1=0.04;
	  }	  
	  if( j == 1 ){
	    min2 = -0.04;
	    max2 = 0.04;
	  }
	  if( j == 3 ){
	    min2 = -0.04;
	    max2 = 0.04;
	  }
	  if( j == 5){
	    min2 = -0.04;
	    max2 = 0.04;
	  }
	  if( j == 7){
	    min2 = -0.04;
	    max2 = 0.04;
	  }	  	  	  
	  pca.plotScores( i, j, scorePlot, min1, max1, min2, max2);	  
	}
	
	string scoreImage= outputName + "_pca_scores_pc_" + to_string(i+1) + "_" + to_string(j+1) + ".img.gz";
	pca.imageScores(i,j,scoreImage);
	scoreImage= outputName + "_pca_scores_pc_" + to_string(i+1) + "_" + to_string(j+1) + "_masked.img.gz";
	pca.imageScores(i,j,scoreImage,true, true, pcselect);
      }//for j
    }//for i
  }//ifr


  
  return 0;
}

