#ifndef __JVTOOLS_IMAGE_H__
#define __JVTOOLS_IMAGE_H__

/*****************************************************************************/
//
// FILE        :  jvtools_image.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Definition of astroimage class.
//
//
// COPYRIGHT   : Jacco Vink, SRON, 1996, AIP, 1999, COLUMBIA 2000/1, UU,
//               UvA 2024
//
/*****************************************************************************/
#include "jvtools_basic.h"      // include common.h jvtools_polarization.h
#include "jvtools_histogram.h"    // includes also jvtools_basic.h common.h
#include <complex>
//#include "srfftw.h"
//#include "fftw3.h"


#define USEFFTW 1
#undef USEFFTW

using namespace::std;

#define MAX_SIZE         16384
#define MAX_MEDIAN       25
#define MIN_DIV          1.0E-12
#define NOT_A_NUMBER     0.00
#define BLANK            0.0

bool is_power_of_2( int n );
Status printRADEC( double ra, double dec);
Status readfont();


void fourn(float data[], unsigned long nn[], int ndim, int isign);
void realft(float data[], unsigned long n, int isign);
void four1(float data[], unsigned long nn, int isign);


Status makeRadialPolarisationProfiles(const Image &stokesI, const Image &stokesQ, const Image &stokesU,
				      const Image &mask,
				      float x0, float y0, float rad, const string &outputName);

Status generatePolarisationVectors(const Image &polAngleMap, const Image &mask, const Image &pvalueMap,
				   float minP,
				   const string &outputName);



typedef struct moments{
  double xc, yc, xx, yy, xy;
} Moments;

typedef enum FFTWdir{
  FFTW_REAL_TO_COMPLEX, FFTW_COMPLEX_TO_REAL
} fftw_direction;


typedef enum image_scaling {
  SCAL_LIN=0, SCAL_SQRT=1, SCAL_LOG=2,SCAL_SQR=3
} ImageScaling;


enum Filter {nofilter=0, gauss=1, boxcar=2, hamming=3, hanning=4, welch=5};

namespace jvtools{
  // forward declarations
  struct SpectralParameters;
  class Auxiliary;
  class ExtractionParameters;
  class Region;
  class Profile;
  class PGPlot;
  class Plot;  
  class Response;
  class Histogram;
  class PSFmodel;
  class Polarization;
  class Morphology;
  class Image;
  

  class Image : public BasicData
    {
      friend class ImageFriends;
      friend class ImageImportExport;
      friend class PGPlot;
      friend class Region;
      friend class Pca;
    public:
      Image( void );
      Image( const Image &im ); // copy constructor
      Image( int xsize, int ysize, float val=0.0 );
      
      ~Image();
      Image &operator=(Image const &im );
      Image &operator=(float value);


      Status allocate( int n, int m, float initval = 0.0);
      Status deallocate();
      Status readfits(  const char fileName[], int imnr = 1, int hdunr=1 );
      Status read(  const char fileName[], int imnr = 1, int hdunr=1 );
      Status read(  const string fileName, int imnr = 1, int hdunr=1 ){
	return read(fileName.c_str(), imnr,hdunr);
      };
      Status save( const char fileName[], bool append=false);
      Status save( const string fileName, bool append=false){
	return save( fileName.c_str(), append);
      };
      Status writefits( const char fileName[], bool append=FALSE);
      Status writefits_complex( const char filename[] );
      Status saveComplex( const char fileName[] );
      Status extract( const ExtractionParameters &selection, Image *variance=NULL);
      Status extractFermi( int argc, char *argv[]);
      Status extractFermi( const char *eventFile,  double emin, double emax,
			   double ra, double dec,
			   double size=10.0, double pixsize=0.1);
      Status extractImage( const char filename[], const char gtifile[],
			   Image &mask, 
			   unsigned int im_size=512, 
			   unsigned int binfactor=16,
			   int ch_min=0, int ch_max=2048, 
			   double dx=0.0, double dy=0.0,
			   int ccdnumber=-1,
			   Coordinate_type ctype= SKY_XY, char ch_type[]="PI",
			   double tstart=0.0, double tend=3.15e7,
			   unsigned long b_size = DEFAULT_BUFFER_SIZE);
      Status extract_image( char filename[], char gtifile[], 
			    Image &mask, unsigned int im_size=512, 
			    unsigned int binfactor=16,
			    double tstart= 0.0, double tend=3.15e7,
			    int ch_min=0, int ch_max=2048,
			    double dx=0.0, double dy=0.0,
			    Coordinate_type ctype = SKY_XY, 
			    char ch_type[]="PI",
			    unsigned long b_size =  DEFAULT_BUFFER_SIZE);
      Status phase_image(char filename[], char gtifile[], Image &mask,
			 double period, unsigned int phase=1, 
			 unsigned int nbins=10,
			 unsigned int im_size=512, unsigned int binfactor=16,
			 double tstart= 0.0, double tend=3.15e7,
			 int ch_min=0, int ch_max=2048, 
			 double dx=0.0, double dy=0.0,
			 Coordinate_type ctype = SKY_XY, 
			 unsigned long b_size = DEFAULT_BUFFER_SIZE);
      Status phase_image(char filename[], char gtifile[],
			 double period, unsigned int phase=1, 
			 unsigned int nbins=10,
			 unsigned int im_size=512, unsigned int binfactor=16,
			 double tstart= 0.0, double tend=3.15e7,
			 int ch_min=0, int ch_max=2048,
			 double dx=0.0, double dy=0.0,
			 Coordinate_type ctype = SKY_XY, 
			 unsigned long b_size = DEFAULT_BUFFER_SIZE );
      Status extract_rgs_spec(char filename[], char expmapname[], 
			      Image *expmap, int order= -1,
			      float lambdamin=5.0, float lambdamax=30.0,
			      unsigned int betabinf=1,
			      unsigned int xdspbinf=1,
			      double dalpha = 0.0, bool plotbanana=FALSE, 
			      unsigned long b_size = DEFAULT_BUFFER_SIZE);
      Status extract_rgs_spec(char filename[], int order, bool plotbanana, 
			      unsigned long b_size =  DEFAULT_BUFFER_SIZE);
      Status extract_detmap( char filename[], char gtifile[],
			     Image &mask, int ccdnumber=1,
			     unsigned int binfactor=1,
			     double tstart=0.0, double tend=1.0e37,
			 int ch_min=1, int ch_max=20000, 
			     Coordinate_type ctype=SKY_XY,char ch_type[]="PI", 
			     unsigned long b_size=DEFAULT_BUFFER_SIZE  );
      Status specprofile( int j0, int jn, char filename[], 
			  float delta_alpha=0.0);
      Status setobject( char objname[] ){
	set_object(objname);
	return SUCCESS;}
      Status setCoordinates( float nwcrval1, float nwcrval2, 
			     float nwcrpix1, float nwcrpix2, 
			     float nwcdelt1, float nwcdelt2, 
			     float nwcrota2=0.0, float equinox=2000.0,
			     const char ct1[]="RA---TAN",
			     const char ct2[]="DEC--TAN",
			     const char sys[]="FK5"   );
      Status getCoordinates( float *nwcrval1, float *nwcrval2, 
			     float *nwcrpix1, float *nwcrpix2, 
			     float *nwcdelt1, float *nwcdelt2, 
			     float *nwcrota2, float *nwequinox) const;
      Status getCoordinates( float *nwcrval1, float *nwcrval2, 
			     float *nwcrpix1, float *nwcrpix2, 
			     float *nwcdelt1, float *nwcdelt2, 
			     float *nwcrota2, float *nwequinox,
			     char *oradecsys, char *octype1, 
			     char *octype2) const;
      Status setblank( float val );
      Status setexposure( float newexposure );
      
      Status rebin( int n = 2 );
      Status rebinx( int n = 2 );
      Status rebiny( int n = 2 );
      Status rebin( const Image &im, int n );
      Status block( int n=2 );
      Status scaledown( double xscale, double yscale);
      Status zoom2( Image im );
      Status zoom( Image im, int factor=2 );
      Status zoom( int factor=2 );//in place
      Status rotate( double angle );
      Status orientate( void );
      Status resize( int newxy );
      Status convertNaN( float replaceVal=0.0 );
      Status subset( int x1, int y1, int x2, int y2, float value=0.0 );
      Status hardness(const Image &image1, const Image &image2,
		      bool normalize=false,
		      bool simple=true,  int ncounts=20);
      Status statistics( float *mean, float *sdev, float *skewn, float *kurt, 
			 int i1= -1, int j1 = -1, int i2=-1, int j2 =-1); 
      double median( bool ignoreZero=true ) const;
      Status rescale( int low, int high );
      Status operator+=( const Image &im );
      Status operator-=( const Image &im);
      Status operator*=( const Image &im);
      Status operator/=( const Image &im);
      
      Status operator+=( double a );
      Status operator-=( double a );
      Status operator*=( double a );
      Status operator/=( double a );
      
      Status squareroot( void );
      Status logarithm( void );
      Status gammaCorrection( double gamma=2.0);
      Status countrate( void );
      
      float minimum( int i1=-1, int j1=-1, int i2= -1, int j2= -1 ) const;
      float minimum( int *imin, int *jmin, int i1=-1, int j1=-1, int i2= -1, int j2= -1 ) const;
      float maximum( int i1=-1, int j1=-1, int i2= -1, int j2= -1 ) const;
      float maximum( int *imax, int *jmax );

      double total( void ) const;
      double total( int i1, int j1, int i2, int j2 ) const;
      int nonzeropixels() const;
      //      bool exists( void ) const;      
      bool exists( void ) const{
	if( nx * ny > 0 )
	  return true;
      	else
      	  return false;
      }//exists()
       bool sameSize( const Image &im) const{
	 return (this->nx == im.nx && this->ny == im.ny);
       }//sameSize()
      Status center( float *x, float *y, float *dr) const;
      Status setpixel( int i = 0, int j=0, float val=0.0 );
      float get(  int i, int j) const;
      float get(  int pixnr ) const;            
      float getMax( int i1, int j1, int i2, int j2, 
		    int *imax, int *jmax ) const;
      float getMax( double x0, double y0, double rad,
		    int *imax, int *jmax ) const;
      Status weightedPosition( double *x0, double *y0, double rad) const;
      Status setExposure( double expotime );
      long getnxny( int *m, int *n) const;
      int getnx() const{ 
      	return  nx;
      };
      int getny() const{ 
	return  ny;
      };
      Status getChanMinMax( int *chMin, int *chMax ) const;
      inline double getEmin( void ) const{
	return emin;
      };
      inline double getEmax( void ) const{
	return emax;
      };
      Status getPixelsize( double *xscale, double *yscale) const;
      Status set(int i , int j, float val=0.0 );
      Status clip( float val1, float val2, bool saturate=false );
      Status flipX( void );
      Status flipY( void );
      Status flipXY( void );
      Status invert( void );
      Status replaceBlank(float val=0.0);
      Status bilevel( float low, float up, float val = 1.0 );
      Status threshold( float threshold, float val=1.0 );
      Status testPrint(FILE *testStream=stdout ) const;
      //      Status info(FILE *testStream ) const;
  
//
//  These can be found in image2.cc:
//
      Status sourcestat( double x0, double y0, double rad, 
			 Moments *pos, double *ncounts, double *nbgd,
			 bool detBgd=false);
      Status getCentroid(float x0, float y0, float rad, 
			 float *xc, float *yc, float *xyc, 
			 float *x2c, float *y2c, float *ntotal ) const;
      Status mosaic( const Image &im, bool scalecorrection=TRUE,
		     bool interpolate=TRUE);
      Status mosaic_discrete( const Image &im );
      Status exposuremap( const Image &im );
      Status newcoord( const Image &im, double dx0=0.0, double dy0=0.0,
		       bool interpolate = FALSE);
      Status newcoord2( const Image &im, double dx0=0.0, double dy0=0.0,
			bool interpolate = false);
      Status sin2tan(const Image &im );
      Status getXorY(double xory, double raordec, int whichxy, int whichradec, 
		     double *outputxy);
      Status concat_h( const Image im1, const Image im2);
      Status concat_v( const Image im1, const Image im2);
      Status MonteCarlo(const Image &im, unsigned long nevents);
      Status radialPolarisation(const Image polfracim, float x0, float y0);
      Status generateMonteCarloEvents(const Image &im,
				      //		      float slope,
				      //				      float norm_1keV,
				      SpectralParameters parameters,
				      PSFmodel *psf,
				      Auxiliary *arf,
				      Response *rsp,
				      //				      float emin, float emax,
				      Polarization *polarization,
				      unsigned long nevents=100000
				      //				      float expTime=1.0e5
				      );

      unsigned long int findk(double val);



//
//  These can be found in image3.cc:
      Status selectregion(const char regionfile[], bool chat=TRUE);
      Status selectRegion(const char regionfile[], bool chat=true);
      Status medianfilter(int n = 3);
      Status boxcarfilter(int filtersize = 3, 
			  int i1= -1, int j1= -1,int i2= -1,int j2= -1, bool errSm=false );
      Status boxcarfilter(const Image &im, int filtersize, 
			  int i1=-1, int j1=-1, int i2=-1, int j2=-1);

      Status hammingfilter(int filtersize = 7, 
			   int i1= -1, int j1= -1,int i2= -1,int j2= -1,
			   bool errSm=false,
			   float alpha=0.53836);
      Status welchfilter(int filtersize = 7, 
			 int i1= -1, int j1= -1,int i2= -1,int j2= -1,
			 bool errSm=false);
      Status hanningfilter(int filtersize = 5, 
			   int i1= -1, int j1= -1,int i2= -1,int j2= -1,
			   bool errSm=false){
	return hammingfilter( filtersize, i1, j1, i2, j2, errSm, 0.5);
      }      
      Status gaussianfilter(float sigma=1.0,
			    int i1= -1, int j1= -1,int i2= -1,int j2= -1,
			    bool errSm=false);
      Status gaussianfilter(const Image &im,float sigma, 
			    int i1=-1, int j1=-1, int i2=-1, int j2=-1);
      Status makeGauss( const Image &im, float sigma_x, float sigma_y );
      Status makeGaussFT( const Image &im, float sigma_x, float sigma_y);
      Status makeRingFT( const Image &im, float sigmax, float sigmay);
      Status fft( void );
      Status convolve( Image *psf, bool newfft=false);
      Status convolve(const Image &im,Image *psf,bool newfft=false);
      Status shift_origin(void );
      Status edgedetect();
      double correlation(const Image &model) const;
      double sqrDistCorrelation(const Image &image,
				double dx=0., double dy=0., 
				double x0= -1.0, double y0=-1.0,
				double expf=1.0, double phi=0.
				) const;
      double likelihood(const Image &model, long *nmodel) const;
      double likelihood(const Image &model, long *nmodel, 
			int i1, int j1, int i2, int j2) const;
      double likelihood_image(const Image &model, const Image &image,
			      long *nmodel);
      Status poisson( float mean );
      Status poisson(const Image &im, float factor=1.0, long idum=1);
      float fraction( float fraction0, int i1, int j1, int i2, int j2 );
      Status writeHistogram(char filename[], 
			    int i1= -1, int j1 = -1, int i2=-1, int j2 =-1);
      Status getcnts(int ic, int jc, float radius, float *cnts, 
		     float *error, bool bgdsubtr=TRUE) const;
      Status getflux2(int ic, int jc, float radius, float *flux, float *error);
      float getCountRate(float xc, float yc,
			 float radius, float bgdradius, float *error);
      Status radialprofile( Profile *prof, const Image &mask,
			    double xc, double yc, double rad, double radmin=0.0,
			    double phi1=0.0, double phi2=360.0, int bin=1) const;
      Status createHistogram( Histogram *histogram, int nbins=1024,
			      bool ignoreNull=true, bool discrete=false, bool rescale=true);
      int  startMark8(float low, float up);
      Status mark8( float value, int iseed, int jseed );
      int  startMark4(float low, float up);
      Status mark4( float level, float value, int iseed, int jseed );
      Status erode(float val);
      Status dilate(float val);
      int nBoundaryPix8( int i, int j, float val);
      int nBoundaryPix4( int i, int j, float val);
      Status addgaussian(float x, float y, float norm,
			 float sigma_x, float sigma_y, float rot=0.0);

      Status meshrefinement(Image *meshedimage, Image *index,
			    Image *binMap,
			    float maxCnts=100,
			    int levels=4);
      Status meshedIndexBinning(const Image &indexmap, const Image &binMap, double *mean );            
      //
      // Can be found in Image4.cc
      Status drawregion(char regionfile[],float intensity, 
			unsigned int zoom=1, bool chat=true, int thickness=1);
      Status drawline( float x1, float y1, float x2, float y2, 
		       float intensity, int thickness=1);
      Status drawcircle( float xc, float yc, float r, float intensity);
      Status drawgrid( float intensity, bool in_hours = true, 
		       bool border=false, bool writetag=true);
      Status putcharxy( int x, int y, unsigned char ch, float intensity, 
			int size=1);
      Status putstring( const char str[], int x, int y, float intensity, 
			int size=1);
      Status draw3ColorBar( int color, int width=100, float bgvalue=0.0 );

      //
      // image_regions.cc
      //
      Status applyRegion( Region *region );
      Status applyMask( const Image &mask );
      Status radian2deg();
      Status maskWindow(int *i1, int *j1, int *i2, int *j2, float threshold=0.5);
//
// Vignet corrections
//
      Status rhri_vignet( const Image &im, float energy );
      Status pspc_vignet( const Image &im, float energy,
			  bool apply_ringcorr=FALSE);
      Status gis_vignet( const Image & im, float energy1, float energy2);
      Status vignet_correction( const Image &im, float energy, 
				bool apply_ringcorr=FALSE );

      //
      // Wavelet analysis
      //
      Status wavelet( Image &im, int isign, int ndaub );
      Status a_trous( int npix, float sigma);
      Status significanceSelection( Image variance1, Image variance2,
				  double significance );
      Status mexicanhat( int npix, float sigmax, float sigmay);
      Status mexicanhat_neg( int npix, float sigmax, float sigmay);
      Status mhfilter(float  sigmax, float sigmay,
		      int i1= -1, int j1= -1,int i2= -1,int j2= -1 );
      Status mhfilter(const Image &im, float  sigmax, float sigmay,
		      int i1= -1, int j1= -1,int i2= -1,int j2= -1 );
      Status mexicanhat(const Image &im,float sigmax, float sigmay);
      Status mexicanhatFT(const Image &im, float sigmax, float sigmay );
      Status negmexicanhatFT(const Image &im,float sigmax, float sigmay);
      Status ringwavelet(int n=1024);
      
      Status exposuremask(const Image &expmap);
      

      Status hillbinning1( Image *indexmap, float minval=10.0);
      Status adaptiveSmooth(int maxStep=4, 
			    float threshold=3.0, float sigma0=1.0);
      Status contourBinning( Image *smoothedim, Image *mask, float snr=3.0 );
      Status htransform( unsigned int order=0, bool backward=FALSE,
			 double threshold=0.27,
			 bool filter=true);
      Status htransform( Image *errorImage,
			 unsigned int order=0, bool backward=false,
			 double snr=3.0, bool filter=true, bool posdef=true,
			 int xstart=1, int ystart=1 );
      Status hfilter( int stepsize);
      
      //
      // Misceleneous
      //
      Status plotImage();
      Status binXY();
      Status plot(void );
      Status contour( int nc=5, bool squareroot=false);
      Status findbadpix( int *i0, int *j0, float threshold);
      Status findBadPixels( int ccdnr , float threshold );
      Status findBadPixels(const Image &detmap, int ccdid, 
			   float threshold );
      Status markbadPNrow();
      Status casa( const Image &im85, float e0, float e1, float ew1, 
		   float ew2, float ewf );
      Status pspcpsf( const Image &im, float energy );
      Status hripsf( const Image &im, Instrument instr);
      Status hripsf( int size, const Image &im );
      Status mecspsf( const Image &im, float energy, int instr=0 );
      Status mecspsf(float energy, Psfpar mecs );
      Status makeXMMpsf(const Image &im, float energy=1.0, float angle=0.0);
      Status epicPSFimage( double rcore, double alpha );
      Status vignetCorrectionXMM(jvtools::Response *resp, double energy,
				 double x0, double y0);
      Status im2psf( const Image &im, int n, int m, float xc, float yc);
      Status lucy(const Image &im, Image *psf, Image *phi, 
		  Image *psi, int niter );
      Status beam2psf();
      Status expansion(const Image &im, float x0, float y0, float factor,
		       float dx0=0.0, float dy0=0.0, float angle=0.0,
		       bool interpolate=true);
      double correlate(const Image &events,		       
		       double dx, double dy, double exp=1.0,
		       double x0 = 0.0, double y0 = 0.0);
      Status calcError(const Image &im );
      Status thinShell(double x0, double y0, double maxRad, 
		       double *radius, double *surfBr, int n);
      Status synchrotronSelfAbs( double x0, double y0, double freq );
      double chisqr( const Image &error);
      Status betamodel( Image &im, float xc, float yc, float beta, 
			float rc, float norm);
      Status writejpeg(char filename[], int quality=75);
      Status colormap_BB( Image *red, Image *green,
			  Image *blue, float gamma=2.0,
			  float offset1= 0.25, float offset2=0.5) const;
      Status colormap_B( Image *red, Image *green,
			 Image *blue, float gamma=2.0,
			 float offset1= 0.25, float offset2=0.5, 
			 float offset3 = 0.75) const;
      Status colormap_C( Image *red, Image *green, 
			 Image *blue, float cpar=1.0 ) const;
      Status map_SIN( Image *im, float peaks, float phase) const;
      Status map_SIN_B( Image *im, float peaks, float phase) const;
      Status makeborder( int extraspace=20, bool drawWedge=false );
      Status testimage( void ); 
      int nonzeropixels();
      double sigma(double *mean );
      Status mandelbrot(float x1, float y1, float x2, float y2, 
			int nx0, int maxit );
      Status ising(double kT, int n=512);
      Status ising_energy(const Image &image);
      Status signal( Image &im, float scale );
      
      float power();
      Status maxima( unsigned int *imax, unsigned int *jmax, float *val );
      Status centroid(float x0, float y0, float rad, float *max,
		      float *xc, float *yc, float *xyc, 
		      float *x2c, float *y2c ) const;
      double extractCounts( float x, float y, float radius, 
			    long *npix) const;
      Status atrousThreshold( const Image &background, 
			    float scale, float probsigma );
      Status weighted_position( int i, int j, float *x, float *y, 
				float *xerr, float *yerr, float *ncnts,
				float radius) const;
      Status replace_background( float xc, float yc, float radius,float bgd);
      Status setpixels( int i1, int j1, int i2, int j2, float value);
      float localbackground(float xc, float yc, float radius );
      Status backgroundimage( const Image &inputimage, int dpix=1);
      Status makemodel( int i1, int j1, int in, int jn, int x0, int y0,
			float backgrnd, float norm, 
			const Image &source);
      Status mksource( int size, float sigma, const Image &im );
      Status weighted_max( int i1, int j1, int i2, int j2, float *x0, 
			   float *y0, float *xsigma, float *ysigma);
      //    Status erode( float level );
      Status find_box(int *i1, int *j1, int *i2, int *j2);
      Status fill_box(int i1, int j1, int i2, int j2);
      Status mark_3sigma(int i0, int j0, float sigma);
      float calc_flux(int i0, int j0, float radius);
      Status makeContinuum( const Image powindex, double eratio);
      Status bieging();
      Status extract_image( char filename[], char gtifile[],
			    double ra, double dec, double rmin, double rmax,
			    double phimin, double phimax,
			    Image &mask,  unsigned int im_size=512, 
			    unsigned int binfactor=4, 
			    int ch_min=0,int ch_max=4096,
			    double dx=0.0, double dy=0.0,  int ccdnumber=0,
			    Coordinate_type ctype=SKY_XY, char ch_type[]="PI",
			    double tstart=0.0, double tend=3.15e7,  
			    unsigned long b_size = DEFAULT_BUFFER_SIZE );
      double calcRGSangleOffset( double energy0, int rflorder=-1);
      
      Status projectionX(float **xrarray, int *n);
      Status projectionY(float **yrarray, int *n);
      Status reprojectXY(float *xprojection, float *yprojection, 
			 int nx, int ny );

      Status checkerEffect( const int blocksize=5,
			    const float threshold=30.0,
			    const float maxval=255.0 );
      Status interChange( const int blocksize=5,
			  const float threshold=30.0,
			  const float maxval=255.0 );
      Status nearestSegment(const  Region &region);

      Status compare(const Image &image2, PGPlot *window);
      // neutron star models
      Status projectedSphereWithPoles( float rot, 
				       float rotAlpha, float rotBeta, 
				       float precAlpha=0.0, float precBeta=0.0,
				       int size=512);
      Status equipotential( float q, float a=100, float rmax=500.0, int n=1024);

      Status polarizationVectors( const string &regionFileName,
				  const Polarization &polarization,
				  const Image &stokesQ,
				  const Image &stokesU,
				  const Image &polDegree,
				  const Image &significance,
				  float minSigma=3.0,
				  int deltapix=5, bool plotCircles=false
				  );
      Status polarizationVectors2( const string &regionFileName,
				   const Polarization &polarization,
				   const Image &stokesQ,
				   const Image &stokesU,
				   const Image &polDegree,
				   const Image &chiSqr,
				   float minChiSqr=12.83,
				   int deltapix=5,
				   int vector=1,
				   bool colorCode=false,
				   bool plotCircles=false
				  );      
      Status chiSqr_to_Pvalue(const Image &chiSQR);
      Status plotCorrelation(Image *image2, PGPlot *window);
      
      Status selectMCevents( Image *eventlist,
			     Image *error,
			     const Stokesparameters stokes, const WeightingScheme weighting=NONE);
      Status selectMCevents( Image *eventlist,
			     Image *stokesI,  Image *stokesIvar,
			     Image *stokesQ, Image *stokesQvar,
			     Image *stokesU, Image *stokesUvar,
			     const WeightingScheme weighting=NONE);


      // image_polarisation.cc
      Status polarisationAngle(const Image &stokesQ, const Image &stokesU);
      Status radiallyRotQU( Image *stokesQ, Image *stokesU, float x0, float y0);      

    protected:
    private:
      float *pixel;
      float emin, emax;
      int nx, ny;
      
#ifdef USEFFTW
      fftwf_complex *fftw_pixels;
      complex<float> *pixels_fft;

      
      unsigned long ncomplex;           //  size of complex pixels used by FFTW
      bool ftransformed, fftplanexist;

      //      rfftwnd_plan fftplan;
      //      fftw_direction fftwdir;

      fftwf_plan fftplan;
      fftw_direction fftwdir;


#endif 
      char classid[12];
      //      unsigned int imgbin;
      double imgbin;
      
      Status allocate_complex( fftw_direction dir= FFTW_REAL_TO_COMPLEX );
      
      Status reset_detector_keywords();
      //    void warning(char message[], int messnr=0) const;
      
      
      // in image2.cc:
      Status mosaic_limits( const Image &im, int *xpix, int *ypix);
      
      // in image3.cc:
      Status  hasNeighbour( int i, int j, float val);
      Status selectcircle(FILE *fp, char sign, RegionSystem system=IMAGE,
			  int nReg=-1);
      Status selectellipse(FILE *fp, char sign, RegionSystem system=IMAGE,
			   int nReg=-1);
      Status selectpolygon(FILE *fp, char sign, RegionSystem system=IMAGE, int nReg=-1 );
      Status selectbox(FILE *fp, char sign, RegionSystem system=IMAGE,
		       int nReg=-1);
      // in Image4.cc:
      Status drawCircleRegion(FILE *fp, float intensity, unsigned int zoom=1);
      Status drawPolygonRegion(FILE *fp, float intensity, unsigned int zoom=1);
      Status drawBoxRegion(FILE *fp, float intensity, unsigned int zoom=1, 
			   int thickness=1);
      Status windowfilter(float *window, int n, bool renormalize=TRUE,
			  int i1= -1, int j1= -1,int i2= -1,int j2= -1 );
      Status windowfilter(const Image &im, float *window, int n,
			  bool renormalize, 
			  int i1=-1, int j1=-1, int i2=-1, int j2=-1);
      //
      // image_regions()
      //
      
      Status selectPolygon(Region *region, unsigned int regionNumber,
			   unsigned int nstart);

      Status hillbinning2( Image *indexmap, float minval, float index);
    }; // class Image

  #define EXCLUDE_REGION   (-2.0)
  #define IGNORE_REGION   (-1.5)
  #define OLDMETHOD_REGION (-1.0)


};//namespec jvtools


// Some shared applications of the class:
Status getBadPixels(const char evt_file[], const char gti_file[], 
		    int chmin, int chmax,
		    jvtools::BasicData *astroproduct, jvtools::Image &mask, 
		    float threshold, int ccdid=-1, 
		    Coordinate_type xytype=SKY_XY);


#endif

