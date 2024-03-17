#ifndef __ASTRO_BASIC__H__
#define __ASTRO_BASIC__H__

/*****************************************************************************/
//
// FILE        :  jvtools_basic.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  X-ray astronomy base class.
//
//
// COPYRIGHT   : Jacco Vink,  1999, 2002
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <string>
//#include <values.h>
#include "fitsio.h"
#include "jvtools_messages.h"
#include "jvtools_history.h"
#include "jvtools_cfitsio.h"
#include "jvtools_gti.h"
//#include "jvtools_polarization.h"
#include "io_tools.h"
#include "xray.h"      
#include "statistics.h"
#include "common.h"
#include "astrophys.h"  


using std::string;
using namespace::jvtools;


#define MAX_FLOAT        (float)(0x7FFFFFFF)
//#define DEFAULT_BUFFER_SIZE  32768
//#define DEFAULT_BUFFER_SIZE  180
//#define DEFAULT_BUFFER_SIZE  360
//#define DEFAULT_BUFFER_SIZE  720
//#define DEFAULT_BUFFER_SIZE  1024
//#define DEFAULT_BUFFER_SIZE  1440
//#define DEFAULT_BUFFER_SIZE  2048
//#define DEFAULT_BUFFER_SIZE  2880
//#define DEFAULT_BUFFER_SIZE  4096
//#define DEFAULT_BUFFER_SIZE  5760
//#define DEFAULT_BUFFER_SIZE  8192
//#define DEFAULT_BUFFER_SIZE  11520
//#define DEFAULT_BUFFER_SIZE  16384
//#define DEFAULT_BUFFER_SIZE  23040
//#define DEFAULT_BUFFER_SIZE  32768
//#define DEFAULT_BUFFER_SIZE  65536
//#define DEFAULT_BUFFER_SIZE  1048576 
//#define DEFAULT_BUFFER_SIZE  2097152
//#define DEFAULT_BUFFER_SIZE  4194304
#define DEFAULT_BUFFER_SIZE  8388608
//1048576 (=2^20), 2097152 (=2^21) , 4194304 (=2^22), 8388608 (=2^23), 
//16777216 =(2^24)

#define NHIST           15
//#define HIST_LEN       128
#define IGNORE_STRING "--IGNORE--"
#define CLOSE_FITS_FILE "close_fits_file"



typedef enum projectiontype{
  UNKNOWN = 0, RADECTAN = 1, RADECSIN = 2, RADECSTG = 3 , RADECARC=4, 
  DETECTOR=10
} Projection;


typedef enum coordinate_type{
  UNDEF_XY = 0,
  RAW_XY = 1, DET_XY = 2, SKY_XY = 3,// SKY_XY_SIN = 4, SKY_XY_ARC = 5,
  RA_DEC = 10, L_B = 11, AXAF_XY = 12, 
  XMM_RGS_XDSP = 20, XMM_RGS_PI = 21, XMM_RGS_XDSP_RAD=22 , 
  XMM_RGS_PI_RAD = 23, XMM_RGS_XDSP_UNBINNED=24, LETGS_LAMBDA_TGD=25
} Coordinate_type;




typedef enum io_status{
  IO_ERROR = 0, IO_SUCCESS = 1, IO_FINISHED = 2 
} IO_status;

typedef struct badpixstruct{
  int rawx, rawy, ccdnr, type, yextent, flag;
} Badpixels;



typedef enum stokesparameters{
  Stokes_I=0, Stokes_Q=1, Stokes_U=2, Stokes_V=3
} Stokesparameters;

typedef enum weightingscheme{ // for polarisation images
  NONE=0, DIRECT=1, ERRORWEIGHTED=2, ERRORWEIGHTEDNORM=3, OPTIMAL=4, ERRORWEIGHTEDCORR=5,MEANMU=6
} WeightingScheme;


double date2juliandate( double year, double month, double day);
void jd2date( double jd, double *year, double *month, double *day);

namespace jvtools{
  extern char fname[FLEN_STATUS];
  extern char defaultname[FLEN_STATUS];
  class Image; //forward declaration
  class Timing;
  class BasicData
    {
    public:
      BasicData();                     // constructor
      BasicData( const BasicData &obs ); // copy constructor
      virtual ~BasicData();           // destructor
      
      BasicData &operator=( const BasicData &obs );
      BasicData &operator+=( const BasicData &obs );
      virtual Status allocate( unsigned long n);
      virtual Status deallocate();
      Status allocateGTI( int nccd );
      Status deallocateGTI( void );
      Status allocate_gti(  unsigned long n );
      Status deallocate_gti( );
      Status allocate_badpix(  unsigned long n = 1024);
      Status deallocate_badpix( );
      
      inline void set_fname(char str[]) const{
	//	strncpy(fname, str, FLEN_STATUS-1);
	JVMessages::setFunctionID(str);
      }//set_fname()
      inline void set_defaultname(char str[]) const{
	strncpy(defaultname, str, FLEN_STATUS-1);
      }//set_basename()
      Status setDetector(const string &name){
	strncpy(detector,name.c_str(),FLEN_VALUE);
	return SUCCESS;
      };
      virtual Status reset_keywords();
      virtual Status resetKeywords();
      virtual Status copy_keywords( BasicData const &obs );
      virtual Status copyKeywords( BasicData const &obs );
      Status copy_badpix( const BasicData &obs);
      Status copyBadpix( const BasicData &obs);
      Status copy_GTIs( const BasicData &obs);
      //  virtual Status readfits(  char file_name[] );
      
      Status constrain_time(double tstart, double tstop );
      Status excludeInterval( double tbegin, double tend);
      Status exclude_time( double tbegin, double tend);
      unsigned long get_nevents() const;
      Coordinate_type getxytype( void ) const;
      Coordinate_type setCoordinateType();
      Status setprojection( void );
      Status toggleApplyPSF( void ){
	applyPSF = !applyPSF;
	JVMessages::message("applyPSF=",applyPSF);
	return SUCCESS;
      }
      Status applyPSFevents(long int n_elements);
      Status applyPSFeventsGauss(long int n_elements);
      Status applyPSFeventsXRISM(long int n_elements);
      
      Instrument getInstrument( void );
      Instrument getInstrumentConst ( void ) const;
      Instrument get_instrument( void );
      double getExposure(void  ) const;
      double getTstart( void ) const;
      double getTstop( void ) const;
      double getMjdMean(void ) const;
      double getMjdObs( void ) const;
      void get_chipsize( Instrument instr, int *xsize, int *ysize, 
			 int *nchip) const;
      Status getFilter( char *filtername) const;
      int getChanMin( ) const {return chanMin;};
      int getChanMax( ) const  {return chanMax;};
      Status getopticxy( double *optx, double *opty) const;
      double calc_exposure();
      double calcExposure(int detector = -1);
      double get_timespan() const;
      double getTimespan() const;
      Status setCreator( const char creatorName[]);
      Status setObsDate( const char date[]);
      Status set_telescope( char telescopename[]);
      Status set_instrument( char instrname[]);
      Status set_object( char objectname[]);
      Status set_filter( char objectname[]);
      Status getradec(double x,double y, double *alpha, double *delta) const;
      Status getxy(double alpha, double delta, double *x, double *y) const;
      Status convert_detxy( const BasicData &obs, double *x, double *y) const;
      Status convert_chipxy(const BasicData &obs, const int &ccdnr,
			    const int &xsize, double *x) const;
      Status convertRGS_XY( const BasicData &obs,
			    double *x, double *y) const;
      
      // to be found in BasicData_io
      Status saveEvents(const string &outputFile );
      int getNrHDUs(const char fileName[]);
      Status readGTI( const char gtifile[] , int ccdnumber);
      Status read_gti( const char filename[], int ccdnumber = -1 );
      Status get_gti(char eventsfile[], char gtifile[], int ccdnumber = -1 );
      Status make_gti( const char filename[] );
      Status writeGTI(const char fileName[]);
      Status write_gti(char filename[], int ccdnr=-1, bool createfile=false);
      Status origin(int argc, char *argv[]);
      Status puthistorytxt(const char histtxt[] );
      virtual Status print( FILE *testStream ) const;
      inline void setChatty( bool chat = true ){ 
	chatty=chat; 
	if( chat )
	  JVMessages::setChatLevel(JVMessages::CHATTY);
	else
	  JVMessages::setChatLevel(JVMessages::SUPPRESS_ALL_MESSAGES);
      };
      
      // other
      Status saveAsciiList( string outputname);
      Status readAsciiList( string inputname);
      int get_psf_hdunr(char ccf_file[], float energy, float angle,
			double *xpixscale, double *ypixscale);
      Status backgroundFilter( 
			      const string &eventFile, 
			      const string &gtiFile,
			      const string &outputName,
			      double tstart, double tstop, 
			      bool epicpn, bool epicmos,
			      Coordinate_type xy_type );

#if 0
      float polarizationAngle(float polfrac = 0.1,
			      float polangle = 0.0, float polErr=0.15,
			      bool radialpol=false,
			      float x0=512.0, float y0=512.0);
      inline void switchOnPolarization(){
	polarizationColumns=true;
	return;

      };

#endif
      
    protected:
      bool chatty;        // defines if warnings should be displayed or not
      bool applyPSF;
      unsigned long n_events, ntot_events, buffer_size, n_gti;
      int *channel, *grade, *pattern, *ccdnr, *node_id;
      long *flag;
      float *x, *y, *detx, *dety, *chanEnergy;
      bool polarizationColumns;
      float *polAngle, *polx, *poly, *modfactor;
      double *time;
      //  int    *gti_ccdnr;
      double *start_gti, *stop_gti;
      int nGTI;
      GoodTimeIntervals *gti;

      char object[FLEN_VALUE];  // FLEN_VALUE define in the fitsio library
      char instrument[FLEN_VALUE], telescope[FLEN_VALUE], detector[FLEN_VALUE];
      char filter[FLEN_VALUE], grating[FLEN_VALUE];
      char obsdate[FLEN_VALUE], enddate[FLEN_VALUE];
      char chan_type[FLEN_VALUE];
      char observer[FLEN_VALUE], creator[FLEN_VALUE];
      char ctype1[FLEN_VALUE], ctype2[FLEN_VALUE], radecsys[FLEN_VALUE];
      
      int chanMin, chanMax, rflorder;
      double crval1, crval2, crpix1, crpix2, cdelt1, cdelt2, crota2, longpole;
      double rgsalpha, line_sep;
      double beta_ref, beta_wid, xdsp_ref, xdsp_wid, alphanom;// RGS keywords
      double pc001001, pc001002, pc002002, pc002001, opticx, opticy;
      double drpix1, drpix2, drdelt1, drdelt2, drval1, drval2, roll_nom;
      float blank, bscale, bzero, blankFloat;
      float dec_nom, ra_nom, equinox, mjd_obs;
      double exposure, ontime, deadtime_corr, start_time, stop_time;
      double mjdref, mjdrefi, mjdreff, mjdmean;
      Projection projectiontype;
      Coordinate_type xy_type;
      Instrument instrumentID;
      unsigned long nbadpix, nbadpixbuffer;
      Badpixels *badpix;
      int nhistory;    
      History history;
      
      
      // in BasicData_io.cc
#if 0
      virtual void warning(char message[], int messnr=0) const;
      virtual void warning(char message[], long int val) const;
      virtual void warning(char message[], double value) const;
      void warning( char message[], const char message2[] ) const;


      inline Status success( void ) const{ 
	//	strncpy(fname, defaultname, FLEN_STATUS-1);
	return JVMessages::success();}
      inline Status failure( void ) const{ 
	//	strncpy(fname, defaultname, FLEN_STATUS-1);
	return FAILURE;};
#endif
      virtual Status errorwarning(char message[], int messnr=0) const{
	return JVMessages::warning(message, (long int)messnr);
      };
      virtual Status errorwarning( char message[], char idstring[]) const{
	return JVMessages::warning(message, idstring);
      };
      virtual Status errorwarning( char message[], double val) const{
	return JVMessages::warning(message, val);
      };
#if 0
      void message(char message[]) const;
      void message(char message[], int val) const;
      void message(char message[], unsigned long val) const;
      void message(char message[], long val) const;
      void message(char message[], double val) const;
      void message(char message[], const char strpar[]) const;
#endif
      void processtatus( char message[], float percentage, bool start=false,
			 bool finish=false);
      IO_status read_events( const char filename[], 
			     unsigned long *n_elements,
			     const char ch_type[] = "PI", 
			     Coordinate_type coord_type = SKY_XY,
			     unsigned long b_size = DEFAULT_BUFFER_SIZE,
			     bool usechipxy=false);
      IO_status readEvents( const char filename[], 
			     unsigned long *n_elements,
			     const char ch_type[] = "PI", 
			     Coordinate_type coord_type = SKY_XY,
			     unsigned long b_size = DEFAULT_BUFFER_SIZE,
			     bool usechipxy=false);
      Status readStdKeywords( const char gtiFileName[] );
      Status readStdKeywords( CfitsioClass *fitsFile );
      Status readStdKeywords( fitsfile *fptr );
      Status read_std_keywords( fitsfile *fptr );
      Status write_std_keywords(fitsfile *fptr, bool writehistory=false) const;
      Status writeStdKeywords(fitsfile *fptr, bool writehistory = false) const;
      Status writeWCSkeywords(fitsfile *fptr);
      Status mergeGTI( void ) ;      

      //
      // INLINE FUNCTIONS
      //
      inline bool check_time( double t ) const{
	
	if( n_gti == 0)
	  return JVMessages::error("no GTIs specified");
	bool in_gti = false;
	for( unsigned long i=0; in_gti == false && i < n_gti; i++ ){
	  if( t >= start_gti[i] && t <= stop_gti[i])
	    in_gti = true;
	}//for
	return in_gti;
      };  
      inline bool checkTime( double t, int ccdnr ) const{
	//	return false;
	//!!!!!!!!!!!
	if( nGTI == 0)
	  return JVMessages::error("no GTIs specified");

	int iGTI;
	if( ccdnr <= 0 )
	  iGTI = 0;
	else
	  if(ccdnr <= nGTI )
	    iGTI = ccdnr - 1;
	  else{
	    JVMessages::warning("CCD NR and number of GTI mismatch", nGTI);
	    return JVMessages::error("CCD NR and number of GTI mismatch", 
				     ccdnr);
	  }

	for( int i=0; i < gti[iGTI].nGTI; i++ ){
	  if( t >= gti[iGTI].startGTI[i] && t <= gti[iGTI].stopGTI[i])
	    return true;
	}//for
	return false;
      };  
      inline bool check_flags(  int patt, long int qflag) const{

	if(
	   ( (instrumentID != XMM_PN) ||  	 // (qflag == 0 &&
	     ( (qflag & BAD_PN_EVENTS) == 0 && 
	       (qflag & OUT_OF_FOV) == 0 &&
	       (qflag & CLOSE_TO_CCD_WINDOW) == 0 && 
	       //(qflag & ON_OFFSET_COLUMN)== 0 &&
	       //(qflag & NEXT_TO_OFFSET_COLUMN) == 0 &&
	       (qflag & OUT_OF_CCD_WINDOW) == 0 &&   patt <= 4 ) ) // XMM_PN
	   &&
	   ( (instrumentID !=  XMM_MOS1)  ||  (qflag == 0 &&
	    //( (qflag & BAD_MOS_EVENTS) == 0  &&
	    //(qflag & OUT_OF_FOV) == 0 &&
					    (patt <= 12)  ) ) // XMM_MOS1
	   && 
	   ( (instrumentID !=  XMM_MOS2)  || (qflag == 0 &&  //( (qflag & BAD_MOS_EVENTS) == 0 &&
					   //   (qflag & OUT_OF_FOV) == 0 &&
					      (patt <= 12)   ) ) // XMM_MOS2
	   &&
	   (   (instrumentID !=  XMM_RGS1)  || (( (qflag & BAD_RGS_EVENTS) == 0)  && (patt <= 12)    )   )
	   &&
	   (  (instrumentID !=  XMM_RGS2)  || ( ( (qflag & BAD_RGS_EVENTS) == 0)  && (patt <= 12) )   ) //XMM_RGS1 / 2
	   &&
	   ((  (instrumentID != CHANDRA_ACIS) || ( (patt <= 6)  &&( patt != 1) &&  (patt != 5))  ) )
	   ||(  ( (instrumentID== SUZAKU_XIS0) || (instrumentID== SUZAKU_XIS1) ||
		  (instrumentID== SUZAKU_XIS2) || (instrumentID== SUZAKU_XIS3) )
		&& (patt <4) )
	   )
	  return true;
	else
	  return false;

      }//check_flags()

      /* Bad pixel checker                                          */
      inline bool is_not_badpix( int rawx, int rawy, int ccd_id){
	Badpixels *badpixptr = &badpix[0];
	for( unsigned long i = 0; i < nbadpix ; i++){
	  if( ccd_id == badpixptr->ccdnr &&
	      rawx == badpixptr->rawx && rawy == badpixptr->rawy ){
	    return false;
	  }
	  badpixptr++;
	}//for
	
    return true;
      }//check_badpix()
      

#if 1
  IO_status petersonslist( char filename[], 
			   unsigned long *n_elements,
			   char ch_type[] = "PI", 
			   Coordinate_type coord_type = SKY_XY,
			   unsigned long b_size = DEFAULT_BUFFER_SIZE);
#endif
    private:
  char classid[15];

    };// class BasicData

};//namespace jvtools

#endif




