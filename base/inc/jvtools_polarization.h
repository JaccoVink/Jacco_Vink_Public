#ifndef __JVTOOLS_POLARIZATION_H__
#define __JVTOOLS_POLARIZATION_H__

/*****************************************************************************/
//
// FILE        :  jvtools_montecarlo.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  
//
//
// COPYRIGHT   : Jacco Vink, 2015
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
//
/*****************************************************************************/
//#include "jvtools_image.h"
//#include "jvtools_psf.h"
#include "jvtools_pgplot.h"


namespace jvtools{
  class Image;
  class PGPlot;
}

using namespace::std;
using namespace::JVMessages;
using namespace::jvtools;



typedef enum{
  radial=1, tangential=2, uniform=3, none=4, maps=5
} PolarizationType;


typedef enum{
  unidirectional=0, circle=1, ring=2, ellipse=3, image=4
} PolarizationModel;



double calculate_fx(double x);

double calculate_gx(double x);

namespace jvtools{

  Status plotSynchrotronFunctions( PGPlot *window);
  
  class Polarization{
  public:
    Polarization();
    ~Polarization();
    Status integrateCrosssection(int n);
    void setPolFraction(float f){
      if( f>= 0.0 && f<= 1.0)
	polfraction=polFractionRing= f;
      
      message("Polarization fraction set to ", polfraction);
      return;
    }
    void setPolFractionInterior( float f){
      if( f>= 0.0 && f<= 1.0)
	polFractionInterior=f;
      message("Polarization fraction interior set to ", polFractionInterior);
      return;
    }
    void setPolFractionRing( float f){
      if( f>= 0.0 && f<= 1.0)
	polFractionRing=f;
      message("Polarization fraction ring set to ", polFractionRing);
      return;
    }
    void usePolarizationMaps(){
      polInterior=polRing=maps;
      return;
    }
    inline bool useMaps(){
      if(  polInterior == maps && polRing ==maps )
	return true;
      else
	return false;
    }
    void setPolTypeInterior( PolarizationType polt){
      polInterior=polt;
      return;
    }
    void setPolTypeRing( PolarizationType polt){
      polRing=polt;
      return;
    }
    void setEnergy( float energy){
      energy_keV = energy;
      angleError = polarisationAngleError(energy_keV);
      return;
    }
    Status setPolAngleError(float angle_err=5.0);
    void toggleRadialPol(){
      radialPol = !radialPol;
      return;
    }
    Status setX0Y0radialPol( float xc, float yc, const Image &image);
    Status setR1R2( float rin, float rout, const Image &image);
    inline float polarizationAngle(float x, float y){
      switch(polModel){
      case uniform:
	return polarizationAngleUniform(x,y);
      case ring:
	return polarizationAngleRing(x,y);
      default:
	return polarizationAngleRing(x,y);
      }
      return 0.0;
    };
    float polarizationAngleUniform(float x, float y);
    float polarizationAngleRing(float x, float y);
    float polarizationAngleMaps(double ra, double dec);
    Status copyWCScoordinates(const Image &image);
    double getR1arcsec() const{
      return r1arcsec;
    };
    double getR2arcsec() const{
      return r2arcsec;
    };
    double getX0wcs() const{
      return x0wcs;
    };
    double getY0wcs()const{
      return y0wcs;
    };
    double polarisationAngleError(double energy=5.0);
    Status readInputMaps(const string &panglemap, const string &pfmap );
  protected:
  private:
    bool radialPol;
    Image anglesIm, polFractionIm;
    float polFractionInterior, polFractionRing;
    PolarizationType polInterior, polRing;
    PolarizationModel polModel;
    //    WeightingScheme weightingScheme;
    float angle_err_offset, angle_err0, energy0_keV, energy_keV;
    float polfraction, polangle, angleError;
    float x0, y0, r1, r2;
    int ndphi;
    double *integrated_crosssection, *delta_phi, dphi, phimin, phimax;
    double x0wcs, y0wcs, r1arcsec, r2arcsec;
    float crpix1, crpix2, crval1, crval2, cdelt1, cdelt2, crota2, equinox;
    char ctype1[FLEN_VALUE], ctype2[FLEN_VALUE], radecsys[FLEN_VALUE];


    double polarizationAngle2electronAngle(double inputAngle);
    
  };

  class PolarisationCorrection{
  public:
    PolarisationCorrection();
    ~PolarisationCorrection();
    Status setCorrectionOn();
    double correctionFactor(double energy_ev);
    double getModulationFactor(double energy_ev);
  protected:
  private:
    bool useCorrection;
    double polf4keV, exponent;
    double energy_ev0, energy_max_ev;
  };
 
}

#endif
