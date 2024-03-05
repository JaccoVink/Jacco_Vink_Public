/*****************************************************************************/
//
// FILE        :  polarization.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  contains main() : application of class image.
//
//
// COPYRIGHT   : Jacco Vink, 2015
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_polarization.h"
//#include "jvtools_montecarlo.h"
#include "jvtools_messages.h"
#include "jvtools_pgplot.h"

using namespace::jvtools;
using namespace::JVMessages;

  
#define STRL 1024


float gaussrnd();

/****************************************************************************/
//
// FUNCTION    : Polarization (constructor)
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
Polarization::Polarization()
{

  //  weightingSccheme=NONE;
  ndphi=0;
  integrateCrosssection(180);
  
  radialPol=false;
  polfraction=0.100;
  polangle=0.0;
  angleError= 5.0/180.0*M_PI;


  energy0_keV = 4.0; //eV reference energy for angle_err0
  //  angle_err0 = 35.0*M_PI/180.0; // default 35 degrees at 4 keV. Scales with sqrt
  angle_err0 = 37.0*M_PI/180.0; // needs to be updated!!
  angle_err_offset=5.0*M_PI/180.0;
  angle_err0 = 32.0*M_PI/180.0; // needs to be updated!!


  //    angle_err_offset=0.1*M_PI/180.0;
  //    angle_err0 = 0.1*M_PI/180.0; // needs to be updated!!

  
  polFractionInterior=0.0;
  polFractionRing=0.20;
  polInterior=tangential;
  polRing=radial;

  polModel=ring;
  
  r1=0.0;
  r2=500.0;

  cdelt1=1.0;
  crpix1=1.0;
  cdelt2=1.0;
  crpix2=1.0;

  crval1 = 0.0;
  crval2 = 0.0;
  
  strncpy( ctype1, "UNDEFINED", FLEN_VALUE);
  strncpy( ctype2, "UNDEFINED", FLEN_VALUE);
  return;
}

Polarization::~Polarization()
{
  if(ndphi != 0){
    delete [] integrated_crosssection;
    delete [] delta_phi;
  }
  return;
}

/****************************************************************************/
//
// FUNCTION    : integrateCrosssection(int n)
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
Status Polarization::integrateCrosssection(int n)
{
  setFunctionID("integrateCrosssection");

  if( n<10)
    return error("n is too small");
  phimin=-M_PI/2.0;
  phimax=M_PI/2.0;

  ndphi=n;
  dphi= (phimax-phimin)/(ndphi-1.);
  integrated_crosssection= new double [ndphi];
  delta_phi = new double [ndphi];

  for(int i=0; i < ndphi; i++){
    delta_phi[i]= phimin + i*dphi;
    integrated_crosssection[i]=0.5*(delta_phi[i] + sin(delta_phi[i])*cos(delta_phi[i])) + M_PI/4.0;
    integrated_crosssection[i] /= M_PI/2.0;
    //    printf("%i %f %f %f\n", i, delta_phi[i], delta_phi[i]/M_PI,
    //	   integrated_crosssection[i]);
  }

  /*
  for(int i=0; i< 10; i++){
    printf("%3i %10f %10f\n", i,
	   polarizationAngle2electronAngle(0.0)/M_PI,
	   polarizationAngle2electronAngle(M_PI/4.0)/M_PI);
  }
  */
  
  return success();
}


/****************************************************************************/
//
// FUNCTION    : 
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : (double) electron track angle
//
/****************************************************************************/
double Polarization::polarizationAngle2electronAngle(double inputAngle)
{
  double rnd=double(rand())/RAND_MAX;

  int i=0;
  double deltaPhi = 0.0, f0;
  
  while( i < ndphi && rnd > integrated_crosssection[i])
    i++;
  if( i > 0 ){
    f0=integrated_crosssection[i-1];
    deltaPhi= i-1.0 +
      (rnd-f0)/(integrated_crosssection[i]-f0);
  }else{
    f0=integrated_crosssection[i];
    deltaPhi= i +
      (rnd-f0)/(integrated_crosssection[i+1]-f0);
  }
  deltaPhi = phimin + deltaPhi*dphi;


  //  printf("rnd = %f %i phi[i]=%f F[i]=%f dphi=%f\n",
  //	 rnd, i, delta_phi[i], integrated_crosssection[i], deltaPhi);
  return (inputAngle+deltaPhi);
}

/****************************************************************************/
//
// FUNCTION    : polarizationAngle
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
float Polarization::polarizationAngleUniform(float x, float y)
{

  float angle;
  //  setFunctionID("polarizationAngle");

  
  if( polfraction < 0 ||polfraction > 1.0 ){
    angle=0.0;
  }
  else{
    float rnd=double(rand())/RAND_MAX;
    if( rnd <= polfraction){
      angle= polangle;// + angleError*gaussrnd();
      if(angle<0)
	angle += M_PI;

    }
    else{
      rnd=double(rand())/RAND_MAX;
      angle=M_PI*rnd;
     
    }
  }
  return angle;

}

/****************************************************************************/
//
// FUNCTION    : setX0Y0radialPol()
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
Status Polarization::setX0Y0radialPol( float xc, float yc, const Image &image)
{

  setFunctionID("setX0Y0radialPol");
  
  x0 = xc;
  y0 = yc;

  image.getradec(x0, y0, &x0wcs, &y0wcs);
  
  return success();
}

Status Polarization::setR1R2( float rin, float rout, const Image &image)
{
  setFunctionID("setR1R2");
  if( rin >= rout)
    return error("Error in input: Rin>Rout");

  copyWCScoordinates(image);
  
  r1=rin;
  r2=rout;
  r1arcsec= r1*sqrt( fabs(cdelt1*cdelt2))*3600.;
  r2arcsec= r2*sqrt( fabs(cdelt1*cdelt2))*3600.;

  message("Rin polarized emission  = ", r1);
  message("Rout polarized emission = ", r2);
  
  return  success();
}


/****************************************************************************/
//
// FUNCTION    : 
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
float Polarization::polarizationAngleRing(float x, float y)
{

  float rnd;
  float angle, r, defaultAngle;


  rnd=double(rand())/RAND_MAX; //random
  angle=defaultAngle=2*M_PI*rnd; //random angle as default

  r= sqrt(SQR(x-x0)+SQR(y-y0));
  if( r < r1 && polFractionInterior > 0.0 ){
    rnd=double(rand())/RAND_MAX;
    if( rnd <= polFractionInterior ){
      
      //angle=atan2( x-x0, y-y0);
      angle=atan2( y-y0, x-x0);
      switch (polInterior ){
      case radial:
	break;
      case tangential:
	angle += 0.5*M_PI;
	break;
      case uniform:
	angle = polangle;
	break;
      default:
	angle = defaultAngle;
	break;
      }
    }// else default
    
  }else if( r> r1 && r<= r2 && polFractionRing > 0.0 ){

    rnd=double(rand())/RAND_MAX;
    if( rnd <= polFractionRing ){
      
      angle=atan2( y-y0, x-x0);
      switch (polRing ){
      case radial:
	break;
      case tangential:
	angle += 0.5*M_PI;
	break;
      case uniform:
	angle = polangle;
	break;
      default:
	angle = defaultAngle;
	break;
      }
    }// else default
  }// else


  angle= polarizationAngle2electronAngle(angle);
  
  
  
  //  angle= angle + angleError*gaussrnd();
  
  return angle;
}


/****************************************************************************/
//
// FUNCTION    : polarizationAngleMaps()
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
float Polarization::polarizationAngleMaps(double ra, double dec)
{
  float angle=0.0, polfraction, rnd;
  double x, y;
  anglesIm.getxy(ra, dec, &x, &y);

  polfraction=  polFractionIm.get((int)(x+1.5), (int)(y+1.5))/100.0;


  rnd=double(rand())/RAND_MAX;
  if( rnd < polfraction ){
    angle= anglesIm.get((int)(x+1.5), (int)(y+1.5)) * M_PI/180.0;
    angle= polarizationAngle2electronAngle(angle);
    // + angleError*gaussrnd();
    //printf("Polfraction = %f rnd=%f\n", polfraction,rnd);
    //    exit(0);
  }else{
    rnd=double(rand())/RAND_MAX;
    angle=rnd*M_PI;
  }

    //  if( polfraction > 0 ){
    //    printf("Polfraction = %f\n", polfraction);
    //    exit(0);
    //  }
    
  return angle;
}


/****************************************************************************/
//
// FUNCTION    : setPolAngleError()
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
// angle_err I  ange error (gaussian sigma) in degrees
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Polarization::setPolAngleError(float angle_err)
{
  setFunctionID("setPolAngleError");

  if( angle_err <= 0.0 || angle_err > 80.0){
    return error("invalid input error for angle: ", angle_err);
  }

  message("Polarization angle error/width (in degrees) set to ", angle_err);
  angleError = angle_err * M_PI/180.0;
  
  return success();
}

#if 1
/****************************************************************************/
//
// FUNCTION    : polarizationVectors2
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
Status Image::polarizationVectors2(
				  const string &regionFileName,
				  const Polarization &polarization,
				  const Image &stokesQ,
				  const Image &stokesU,
				  const Image &polDegree,
				  const Image &chiSqr,
				  float minChiSqr, int deltapix,
				  int vector,
				  bool colorCode,
				  bool plotCircles
				  )
{
  setFunctionID("polarizationVectors");

  
  if( minChiSqr <= 0.0 ){
    return error("invalid input parameter, minimum X^2_2 = ", minChiSqr);
  }
  if( deltapix > 32 || deltapix < 1 ){
    return error("invalid input parameter, delta pixel = ", deltapix);
  }

  if( stokesQ.nx <= 0 ||
      stokesQ.nx != stokesU.nx || stokesQ.nx != chiSqr.nx ||
      stokesQ.nx != polDegree.nx ){
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

  if( !(fp=fopen(regionFileName.c_str(),"w"))){
    return error("Could not open region file for writing: %s", regionFileName.c_str());
  }

  //  fprintf(fp,"image\n");
  fprintf(fp,"fk5\n");

  bool isolatedpixel=false;
  false;
  string color="blue";
  int i2, j2, width=2;
  float x1, y1, x2, y2, x0, y0, l=0.5*deltapix;
  double ra1, dec1, ra2, dec2, ra0, dec0;


    fprintf(fp,"# Region file format: DS9 version 4.1\n");
  fprintf(fp,"# Polarization directions\n");
  fprintf(fp,"global color=%s dashlist=8 3 width=1 font=\"helvetica 10 normal\" select=1 highlite=1 dash=0 fixed=0 edit=1 move=1 delete=1 include=1 source=1\n",color.c_str());


  

  //  l=10;

  message("length of vectors = ", l);
  
  float angle, q, u;
  for(int j=0; j< ny; j++){
    for(int i=0; i< nx; i++){

      if( chiSqr.pixel[i + j*nx] >= minChiSqr ||
	  (colorCode &&  (chiSqr.pixel[i + j*nx] >= 0.001)) ){ //&&	  (i % deltapix == 0) && (j % deltapix == 0) ){
	q = stokesQ.pixel[i + j*nx];
	u = stokesU.pixel[i + j*nx];
	angle= 0.5* atan2(u,q);
	//	if( angle < 0)
	//	  angle += M_PI;
	//	angle *= 0.5;
	//	angle += M_PI/2.0; // to go from B to E-vectors
	i2=i;
	j2=i;
	// check neigboring pixels
	isolatedpixel=false;
	if( i > 0 && i < nx-1 && j > 0 && j < ny-1 ){
	  if( chiSqr.pixel[(i-1) + j*nx] < minChiSqr &&
	      chiSqr.pixel[(i+1) + j*nx] < minChiSqr){
	    isolatedpixel=true;
	  }
	  if( chiSqr.pixel[i + (j-1)*nx] < minChiSqr &&
	      chiSqr.pixel[i + (j+1)*nx] < minChiSqr ){
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
	    if(  chiSqr.pixel[i + j*nx] >= minChiSqr ){
	      color="green";
	    }else{
	      //	      color="red";
	      color="white";	      
	    }
	  }
	  fprintf(fp,"# vector(%f,%f,%f\",%f) vector=%i width=%i color=%s\n",
		  ra0,dec0,0.5*sqrt(fabs(cdelt1*cdelt1))*3600.0,angle*180/M_PI,vector,width,
		  color.c_str());
	  fprintf(fp,"# vector(%f,%f,%f\",%f) vector=%i width=%i color=%s\n",
		  ra0,dec0,0.5*sqrt(fabs(cdelt1*cdelt1))*3600.0,angle*180/M_PI+180.0,vector,width,
		  color.c_str());	  
	  //# vector(23:23:18.802,+58:51:37.296,143.253",46.94022) vector=1 width=4	  

	}
	
	pixel[i + j*nx] = angle * 180.0/M_PI;
      }//else a zero will be in place
      else{
	pixel[i + j*nx] = 0.0;
      }
    }

  }//for j

  
  if( plotCircles ){
    fprintf(fp,"fk5;circle( %f,%f,%.2f\") # width=%i dash=1\n",
	    polarization.getX0wcs(),
	    polarization.getY0wcs(),
	    polarization.getR1arcsec(),width);
    fprintf(fp,"fk5;circle( %f,%f,%.2f\") # width=%i dash=1\n",
	    polarization.getX0wcs(),
	    polarization.getY0wcs(),
	    polarization.getR2arcsec(),width);
  }

  fclose(fp);

  message("length of vectors = ", l);
  
  return success();
}

#endif


/****************************************************************************/
//
// FUNCTION    : copyWCScoordinates()
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
Status Polarization::copyWCScoordinates(const Image &image)
{
  setFunctionID("copyWCScoordinates");



  image.getCoordinates( &crval1, &crval2, &crpix1, &crpix2, 
			&cdelt1,&cdelt2, &crota2, &equinox);
  
  
  return success();
}



#if 0
/****************************************************************************/
//
// FUNCTION    : polarizedFractions
// DESCRIPTION : This plots various GDP error parameters
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



Status polarizedFractions()
{
  setFunctionID("polarizedFractions");



  
  char line[STRL];
  FILE *fp;
  string filename="/Users/vinkj/Google Drive/Projects/XIPE/InstrumentaData/simulated_polfrac.txt";
  puts(filename.c_str());
  
  if( !(fp=fopen(filename.c_str(),"r") ))
    return error("error opening file ", filename.c_str());


  int n;
  fscanf(fp,"%i\n",&n);
  printf("n=%i\n",n);

  
  if( n<= 0 ){
    fclose(fp);
    return error("error in input table");
  }
  fgets(line,STRL-1,fp);
  puts(line);

  float *angle_err=new float [n];
  float *pol_frac=new float [n];

  for(int i=0; i<n; i++){
    fscanf(fp,"%f %f\n",&angle_err[i], &pol_frac[i]);
    printf("%f %f\n", angle_err[i], pol_frac[i]);
  }
  fclose(fp);


  PGPlot window;
  window.setBoundaries(0.0, 70.0, 0.0, 100.0);
  window.setXlabel("E-vector angle error (degrees)");
  window.setYlabel("Measured polarisation (\%)");
  window.open();
  window.setYscaling(100.0);
  window.drawCurve( angle_err, pol_frac,n);


  int n2=20;
  float emin=1.0, emax=10.0;
  float de=(emax-emin)/(n2-1);
  float angle4=35.0, a;
  float *energy = new float [n2];
  float *angle_err_e = new float [n2];
  float *pol_frac_e=new float [n2];

  for(int i=0; i < n2; i++){
    energy[i]= emin + i*de;
    a=angle_err_e[i]= angle4/sqrt( energy[i]/4.0 );
    int j=0;
    while( j< n && a > angle_err[j]){
      j++;
    }
    if( j!=n-1 && j != 0){
      pol_frac_e[i]= pol_frac[j-1] +
	      (pol_frac[j]-pol_frac[j-1])/(angle_err[j] - angle_err[j-1])
	      * (a - angle_err[j-1]);
    }else
      pol_frac_e[i]=pol_frac[j];
    printf("%i %i %f %f %f\n", i, j, energy[i], angle_err_e[i], pol_frac_e[i]);
  }

  window.setColor(PG_COLOR_RED);
  window.drawCurve(angle_err_e, pol_frac_e, n2);
  
  PGPlot window2;
  window2.setBoundaries(emin, emax, 0.0, 80.0);
  window2.setXlabel("Energy (keV)");
  window2.setYlabel("Measured polarisation fraction");
  window2.open();
  window2.setYscaling(100.0);
  window2.drawCurve(energy, pol_frac_e, n2);
  
  delete [] energy;
  delete [] angle_err_e;
  delete [] pol_frac_e;
  
  delete []  angle_err;
  delete []  pol_frac;


  
  filename="/Users/vinkj/Google Drive/Projects/XIPE/InstrumentaData/XIPE_GPD_monte_carlo.txt";

  if( !(fp=fopen(filename.c_str(),"r") ))
    return error("error opening file ", filename.c_str());
  

  int nx;
  fscanf(fp,"%i\n",&nx);
  printf("n=%i\n",nx);

  
  if( nx<= 0 ){
    fclose(fp);
    return error("error in input table");
  }
  fgets(line,STRL-1,fp);
  puts(line);

  energy=new float [nx];
  pol_frac_e=new float [nx];

  for(int i=0; i<nx; i++){
    fscanf(fp,"%f %f\n",&energy[i], &pol_frac_e[i]);
    printf("%f %f\n", energy[i], pol_frac_e[i]);
  }
  fclose(fp);

  window2.setColor(PG_COLOR_RED);
  window2.drawCurve(energy, pol_frac_e,nx);
  
  delete [] energy;
  delete []  pol_frac_e;

  
  return success();
}
#endif


/****************************************************************************/
//
// FUNCTION    : polarisationAngleError()
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
// energy_ev              I        the energy for which to determine the error
//  RETURNS    : the error for the ionisation angle
//
/****************************************************************************/
double Polarization::polarisationAngleError(double energy)
{
  if( energy<= 1.0 || energy>10.0)
    return 0.0;


  angleError= angle_err_offset + angle_err0/sqrt( energy/energy0_keV );
  
  return angleError;
  
}


/****************************************************************************/
//
// FUNCTION    : readInputMaps
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
Status Polarization::readInputMaps(const string &panglemap,
				   const string &pfmap )
{
  setFunctionID("readInputMaps");
  if( anglesIm.read( panglemap) == FAILURE ||
      polFractionIm.read(pfmap) == FAILURE)
    return error("Error reading polarisation-model maps");

  warning("It is assumed that the two polarization-model images have the same coordinates!");
  //polFractionIm.save("test.img.gz");


  polInterior=maps;
  polRing =maps;
  
  return success();
}





#if 0
/****************************************************************************/
//
// FUNCTION    : PolarizationCorrection
// DESCRIPTION : Class to deal with polarisation correction on event by event
// base.
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
PolarizationCorrection::PolarizationCorrection()
{

  setFunctionID("PolarizationCorrection");
  
  
  useCorrection=false;
  polf4keV =0.43;
  energy_ev0=1500.0;
  energy_max_ev=15000.0;
  
  exponent=0.4;

  polf4keV /= pow( 4000-energy_ev0, exponent);
  
  //  printf("---polff4keV= %f\n",polf4keV);

  
  success();
  
  return;
}
PolarizationCorrection::~PolarizationCorrection()
{
  return;
}
Status PolarizationCorrection::setCorrectionOn()
{
  useCorrection=true;
  
  return SUCCESS;
}

double PolarizationCorrection::getModulationFactor(double energy_ev)
{
  double factor=1.0;
  
  if(energy_ev > energy_ev0 && energy_ev0 < energy_max_ev)
    factor= (pow(energy_ev-energy_ev0, exponent)*polf4keV);
  else
    factor = 0.0;
  
  return factor;
}
double PolarizationCorrection::correctionFactor(double energy_ev)
{
  if( !useCorrection)
    return 1.0;

  double factor=1.0;
  if(energy_ev > energy_ev0)
    factor= 1.0/(pow(energy_ev-energy_ev0, exponent)*polf4keV);
  else
    factor = 0.0;
  
  return factor;
}
#endif
