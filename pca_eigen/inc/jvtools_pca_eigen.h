#ifndef __JVTOOLS_PCA_EIGEN_H__
#define __JVTOOLS_PCA_EIGEN_H__

/*****************************************************************************/
//
// FILE        :  jvtools_pca.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION : 
//
//
// COPYRIGHT   : Jacco Vink, 2008
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           

//
/*****************************************************************************/
#include <iostream>
#include "Eigen/Dense"
#include "jvtools_image.h"

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using Eigen::JacobiSVD;


#define NMIN 4 //minimum of components

namespace jvtools{
  class Pca{
  public:
    Pca(const string& outputname="pca");
    ~Pca();
    Status processInputFile(const string& inputfile);
    Status processInputFile_mesh(const string& inputfile);    
    Status allocate(int m);
    //    Status allocate(int m, const Image &maskim);
    Status deallocate();
    Status PCA();
    Status reconstruct(int iband, int pcmax=4, int pcskip=-10);    
    Status storePCAscores(bool savemasks=false, bool hardcopy=true, int pcmax=10);


    Status readSelectionMask( const string& inputfile);
    Status plotComponents(const string &device="/xw", int nmax=6);
    Status plotScores(int pc1, int pc2, const string &device="/xw",
		      float min1=-1, float max1=-1, float min2=-1, float max2=-1);
    Status imageScores(int pc1, int pc2,
		       const string &filename,
		       bool useSelectionMask=false, bool smoothresults=false,
		       float applySelection=-1.0);


    
    Status plotEigenValues( const string &device);  

    Status toggleUseMesh(){
      useMesh=true;
      JVMessages::message("A mesh refined image will be used");
      return SUCCESS;
    }
    Status toggleKeepScores(){
      keepScores = true;
      JVMessages::message("PCA scores will be stored in memory");
      return SUCCESS;
    }
    Status setBinFactor(int bf){
      if( bf > 32 || bf <=1 ){
	JVMessages::warning("bin factor must be 2 <= b<=32, bf=",bf);
	return FAILURE;	
      }

      binfactor=bf;
      JVMessages::message("Bin factor set to ", binfactor);
      
      return SUCCESS;
    }
    Status setMinCounts( double minCnts ){
      if( minCnts < 10 ){
	JVMessages::warning("minCounts not set, value invalid: ",minCnts);
	return FAILURE;
      }

      minCounts=minCnts;
      JVMessages::message("minimum counts per pixel set to ",minCounts);
      return SUCCESS;
    }//

    Status setClipMin( double clipmin){
      if( clipmin < 0 ){
	JVMessages::warning("clip minimum not set, value invalid: ",clipmin);
	return FAILURE;
      }

      clipMin = clipmin;
      JVMessages::message("minimum counts per pixel of the total image set to ",clipMin);
      return SUCCESS;
    }//

    Status setMeshLevels(int nlevels){

      if( nlevels <=1 || nlevels > 6){
	JVMessages::warning("Invalid request, mesh refinement levels must be between 2 and 6, request was ",nlevels);
	return FAILURE;
      }
      levels=nlevels;
      JVMessages::message("#mesh refinement levels set to ",levels);
      return SUCCESS;
    }

  protected:

  private:
    int nn, mm;
    int nx, ny; // Input images size
    bool normalize, pcaComputed, useEnergies, keepScores, select, useMesh;

    Image total, mask, indexmap, binMap, *pcaScores, selectionMask;
    MatrixXd dataMatrix, scoreMatrix;
    //eigenVectorMatrix, sqrtEigenValues, uMatrix;
    //    JacobiSVD<MatrixXd, Eigen::ComputeThinU | Eigen::ComputeThinV> svd;
    Eigen::JacobiSVD<MatrixXd> svd1;
    Eigen::BDCSVD<MatrixXd> svd2;



    int binfactor, levels;
    double minCounts, clipMin;
    int *index_to_pix;    
    double *spectralMean, *spectralVariance;
    double *eigenVectors, *energyBand,  *singularValues;
    string *inputImageNames, outputName;
    


    Status readInputFile(const string& inputfile);
    Status createMask();    
    //    Status createMeshIndex(int nlevels=4);
    Status createMeshIndex();
    Status determine_index_to_pixel( );//const Image &indexmap);
    Status storePCAscores_mesh(bool savemasks=false, bool hardcopy=true, int pcmax=10);     // version for mesh-refined images
    
  };
}
#endif
