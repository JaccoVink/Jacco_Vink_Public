#ifndef __JVTOOLS_PROFILE_H__
#define __JVTOOLS_PROFILE_H__

/*****************************************************************************/
//
// FILE        :  image_profiles.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Definition the  Profiles class.
//                Filtering functions and other functions
//
// COPYRIGHT   : Jacco Vink,  2002-2012
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_basic.h"
#include "jvtools_image.h"
#include "jvtools_pgplot.h"

//#define NMAX_PROF 16192
#define NMAX_PROF 16192

using namespace::jvtools;

enum ProfileType{ RADIAL, RECTANGULAR, ANNULUS, SPECTRAL};
enum NormType{ MAXIMUM, AVERAGE, NORMALIZE, DONOTHING};
typedef enum profModelType { UNIFORMSHELL, EXPONENTIALMODEL, GAUSSIANMODEL } ProfModelType;



//Status tycho();
Status tycho( double *norm, double *offset, double *radius,
	      double *deltaR, double *deltaR2, double *sigma, bool expMod=true, bool fitRW2=false);
Status kepler( double *norm, double *offset, double *radius, 
	       double *deltaR, double *deltaR2, double *sigma, bool expMod, bool fitRW2);
Status plotPanel( PGPlot *plotWindow, const char fileName[], 
		  double deltaR, double radius, double offset, double norm,
		  bool uniformShell=false,
		  double r1=0.0, double r2=100.0, double p1=0.0, double p2=1.2);

namespace jvtools{
  class Profile : public BasicData {
    friend class Image;
    friend class Response;
    friend class PGPlot;
    friend class OptSpec;
  public:
    Profile();
    ~Profile();
    Profile(const Profile & prof);
    Status allocate( int n);
    Status deallocate( void );
    Status reset( void );
    Profile & operator=( const Profile & prof);
    Profile & operator+=( const Profile & prof);
    Profile & operator/=( const Profile & prof);
    Profile & operator*=(float val);
    Status operator-=( const Profile & prof);
    Status operator+=( double val);
    Status operator-=( double val);
    Status setType( ProfileType newType){
      type=newType;
      return SUCCESS;
    }
    Status normalize( NormType method = NORMALIZE, 
		      bool changeSkyRadius=false);
    Status normalizeModel(double rmin, double rmax);
    Status copyScaledProf2Prof(double rmin=0.0, double rmax=-1.0);
    Status renormalize( NormType method = NORMALIZE){return normalize(method);};
    Status rescale( float factor);
    float getScaleProfile( float skyCoordinate);
    float getProfScaling( ){return scaling;};
    int getStartIndex(double rsky) const; 
    int getNbins(double rsky1, double rsky2) const; 
    int getNbins() const{ 
      return nbins;
    }
    float getPixSize(){
      return pixsize;
    }    
    float getBinWidth(){
      return pixsize*bin;
    }    
    double getProfile(int i ) const;
    double getProfileError(int i ) const;
    double getNormProfile(int i ) const;
    double getNormProfileError(int i ) const;
    Status save( const char filename[], bool waddToFile=false) const;
    Status read( const char filename[], int hunit=0 );
    Status read( const string &filename, int hunit=0){
      return read(filename.c_str(),hunit);
    };
    Status print(FILE *fp=stdout);
    Status rebin( int binfactor,   NormType meth = NORMALIZE);
    Status rebin(const Profile &input, int factor, 
		 NormType meth = NORMALIZE);
    Status scaledown(const Profile &input, float scalefactor,
		     NormType meth = NORMALIZE);
    Status setTarget( float r0);
    double sum( double x1=-1, double x2=-1) const;
    double totalCounts( void ) const;
    double minimum( int istart=-1, int istop=-1) const;
    double maximum( int istart=-1, int istop=-1, bool useNormalized=false ) const;
    double maximum( double x1, double x2, bool useNormalized=false ) const;
    double FWHM(double x1=-1.0, double x2=-1.0) const;
    Status flatProfile(double rad, int n=1000);
    //    Status project( float rmax );
    double average(double rmin, double rmax);
    Status AbelTransform(int imax, bool differentiate=true );
    Status lucyRichardsonCorrection(int imax, const Profile &observedProfile);
    Status deProject( Profile *modelProf, float rmin, float rmax, bool initialize, 
		      int niter=1 );
    Status pieImage( Image *im);
    Status sphericalProjection( int imax );
    Status exponentialModel( const Profile &inputProf, double norm,
			     double radius, double rw1, double rw2=-1.0, double norm2ratio=0.1445);
    Status uniformShellModel(  const Profile &inputProf,double norm,
			       double radius, double rw1);
    Status profileModel( const Profile &inputProf, 
			 double norm, double offset,
			 double radius, double rw1, 
			 double norm2ratio=0.0, double rw2=0.0,
			 double sigma=0.0,
			 ProfModelType modelType=UNIFORMSHELL,
			 //bool expModel=true,			 
			 double x1=-1., double x2=-1.);
    Status uniformShellProjected(double radius, double dr, int n=1000);
    Status uniformShellProjected(const Profile &input, 
				 double norm, double offset,
				 double rad, double dr,
				 double sigma=-1.0);
    Status projectedShell(const Profile &input, double rmax,
			  double (*radProf)( double , double , double ,
					     double , double ),
			  double radius, double rw1, double rw2=-1.0,
			  double normRatio=0.0, double norm=0.0,
			  double offset=0.0, double sigma=-1.0,
			  int m=1000 );
    Status exponentialModelProjected( const Profile &inputProf, double norm, 
				      double offset,
				      double radius, double rw1, double rw2, 
				      double sigma=-1.0, double norm2ratio=0.1445);
    
    Status exponentialProfileProjected(double rad, double r0, 
				       double rmax, double offset=0.0,
				       int n=1000, 
				       int m=1000);

    Status exponentialProfileProjected(const Profile &input, 
				       double rad, double r0, 
				       double offset=0.0, double norm=1.0,
				       double x1=-1.0, double x2=-1.0,
				       int m=1000);
    Status fit_profile(const Profile &data, double rmin, double rmax,
			   float *chisqr,
			   double *norm, int fitnorm,
			   double *offset, int fitoffset,
			   double *radius, int fitrad, 
			   double *rw1, int fitrw1,
			   double *rw2, int fitrw2,
			   double *sigma, int fitsigma=0,
			   int nit=10);
    void evaluate_exponential_model( float *parameters, int npar, float *dyda, int ntot, 
				     const Profile &inputProf );
    void mrqmin( const Profile &data, int imin, int imax, float a[], int ia[],
		 int ma, float **covar, float **alpha, float *chisq,
		 float *alamda);
    void mrqcof( const Profile &data, int imin, int imax, float a[], int ia[], 
		 int ma, float **alpha, float beta[], float *chisq );
    
    double chiSquare( const Profile &model, double r1, double r2, int *ndof,
		      bool useScaledProf=true);
    Status enter(int i, float rad, float radSky, float prof, float err){
      if( i < nbins && i >= 0 ){
	radius[i] = rad;
	skyRadius[i] = radSky;
	profile[i] = prof;
	profError[i] = err;
	return SUCCESS;
      }
      return FAILURE;
    }
    //
    //
    Status changeSkyOrigin( void );
    Status wrapAroundOrigin(const Profile &inputProfile);
    Status vignetCorrectionXMM( double energy );
    Status alpha2beta( const Profile &inputProfile, int nchan,
		       int order, double lineSep,
		       double lambda0, double beta0, double beta_wid,
		       double rgsalpha);
    Status fitPolynomial( int order );
    Status profileBox( const Image &image,
		    double x1, double y1, double x2, double y2, 
		    double boxWidth, int binFactor=1);
    Status profileAnnulus( const Image &image, 
			   double xc, double yc, double meanRadius, 
			   double anWidth, int binFactor=1);

    //
    // export
    //
    Status radialShell( double rad, double deltaR, int n=1000);
    Status haarTransform(double sign=3.0,
			 bool reverse=false, int level=1, int maxlevel=2,
			 bool gradients=false, int gradlevel=1, 
			 bool shift=false);
    Status haarTransformError( double sign, int maxlevel);
    double maximumGradient(double r0, double dr, int gr=2);
    double chiSqr( Profile *profile2, int *n, double *cusumMax, 
		   int shift, int maxshift, bool useKS=false,
		   bool bothErrors=true);
    Status calcResiduals(const Profile &model);
    double cStat( Profile *profile2, int *n, int shift, int maxshift, 
		  bool useHT=true);
    Status eveline(char *filename, float rmin, float rmax);
    double correlation( Profile *prof2, int shift, int maxshift);
    double centroid(int istart=-1, int istop=-1, int shift=0);
    double chisqrProf( Profile profile, int *dof, 
		       double rshift=0.0,
		       double renorm=1.0,
		       double r1=-1.0, double r2=-1.0);
    Status plotProfile( PGPlot *window, float r1 = -1.0, float r2=-1.0, 
			float pmin=0.0, float pmax=1.0, bool redraw=false,
			bool plotLines=true, bool plotPoints=false, 
			bool plotNorm=true, bool calcnorm=true);
    Status plotProfile( PGPlot *window, bool overPlot=true){
      return plotProfile(window, -1.0, -1.0, 0.0, -1.0, overPlot);
    }    
    Status plotPixProfile(PGPlot *window, 
			  float minval=0.0, float maxval=30000.0,
			  bool useWcs=false,
			  bool overPlot=true );
    Status smooth( float sigma, Profile *inputProf);
    Status smooth( float sigma );
    Status wavelet( float sigma, Profile *inputProf);
    Status waveletErr( float sigma, Profile *inputProf);
    Status moments( int indexnr, int di, float *centroid, float *var);
    Status searchLocalMaxima(int i1, int i2, float min, float max, 
			     int *ipos, float *locMaxVal);
    Status shift( int ishift);
    Status montecarlo(const Profile &prof, double shiftArcsec, int n,
		      double fluctuation=0.05);
    float determineMedian(bool chop, int chopval=10);
    Status smoothGaussian( float sigma=1.0);
    Status read_gc(string filename);
    Status read_Eger(string filename);
    Status cr_precursor(float emin, float emax, float gamma=2.2,
			float delta=1.0, float lprec_TeV_arcmin=3.0,
			int n=200,
			int nenergy=10);
    Status xrism_psf( );
  protected:
    int nbins, bin;
    int *npixels;
    float *radius, *skyRadius, *profile, *profError, *profCounts;
    float *scaledProfile, *scaledError, *integratedProfile;
    float *gradient1, *gradient2;
    ProfileType type;
    double ncounts;
    float x0, y0, xn, yn, width, annulusRad, phi1, phi2, pixsize, scaling;
    float radMin, radMax;
    float rtarget, deltaPhi;
  };
};





Status fitProfile( const string &profname,
		   double r1, double r2,
		   double *radius, double *deltaR, double *norm, double *bkg,
		   double *chisqr_red,
		   bool uniformShell=false);
Status fitProfile( Profile  *data, Profile *modelProfile, 
		   double x1, double x2,double *norm, 
		   double *radius, double *rw1, double *rw2,
		   double *offset,  double *sigma, bool expMod=true  );

#endif

