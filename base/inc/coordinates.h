#ifndef __COORDINATES_H__
#define __COORDINATES_H__

/*****************************************************************************/
//
// FILE        :  coordinates.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Contines some inline functions
//
//
// COPYRIGHT   : Jacco Vink, SRON, 1997
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include <math.h>
#include <stdlib.h>

#define REF_YEAR 1950.0
#define REF_JD   2433282.0

double date2juliandate( double year, double month, double day);
void jd2date( double jd, double *year, double *month, double *day);

//
// Convert from one coordinate system to another
//
// all input value are in radians
// phi, theta -> old system
//
// alpha, delta -> new system (output)
inline void ch_coord( double phi, double theta, double phip, double ap, 
		      double dp,  double *alpha, double *delta)
{
  phi -= phip;

  double csina, ccosa;
  double cosphi= cos( phi ), sindp=sin(dp), cosdp = cos(dp);
  double costh=cos(theta), sinth=sin(theta);


  *delta = asin( sinth * sindp + costh * cosdp * cosphi );
  csina =  -costh * sin( phi );
  ccosa = sinth * cosdp - costh * sindp * cosphi;

  // ccosa =cos(*delta) cos(*alpha -ap); 
  // csina= cos(*delta) sin(*alpha -ap)

  *alpha = atan2( csina, ccosa) + ap;

}//ch_coord




//
// Convert from one coordinate system to another
//
inline void _coord( double phi, double theta, double phip, double ap, double dp,
		   double *alpha, double *delta)
{
  double csina, ccosa, torad;
  // first change the reference points to radians
  //
  torad = M_PI/180.0;
  dp *= torad;
  ap *= torad;
  phip *= torad;
  phi -= phip;

  *delta = asin( sin(theta) * sin( dp ) + cos(theta) * cos( dp ) * cos( phi) );
  csina =  -cos(theta) * sin( phi );
  ccosa = sin(theta) * cos( dp ) - cos(theta)*sin(dp)*cos( phi);

  // ccosa =cos(*delta) cos(*alpha -ap); 
  // csina= cos(*delta) sin(*alpha -ap)

  *alpha = atan2( csina, ccosa) + ap;


}//coord


//
// convert alpha delta (in radians) from 1950 to equinox 2000
// coordinates
//
// From The Astron. Almanac 1997 B18
inline void toequinox2000(double *alpha, double *delta, double t=1950.0)
{
  double a0, d0, sina0, cosa0, sind0, cosd0, sind, cosd, zeta, za, theta;
  double t2, t3;

  t-= 2000.0;
  t/= 100.0;

  t2 = t*t;
  t3= t*t*t;

  zeta  = 0.6406161 * t + 0.0000839 * t2 + 0.0000050 * t3;
  za    = 0.6406161 * t + 0.0003041 * t2 + 0.0000116 * t3;
  theta = 0.5567530 * t + 0.0001185 * t2 + 0.0000116 * t3;

  zeta  *= M_PI/180.0;
  za    *= M_PI/180.0;
  theta *= M_PI/180.0;

  sind = sin( *delta );
  cosd = cos( *delta );

  sind0 = -cos( *alpha -za) * sin(theta) * cosd + cos(theta) * sind;
  d0    = asin( sind0 );
  cosd0 = cos( d0 );

  sina0 = sin( *alpha - za)*cosd/cosd0;
  cosa0 = (cos( *alpha - za)*cos(theta)*cosd + sin(theta)*sind)/cosd0;

  a0 = atan2( sina0, cosa0 );
  a0 -= zeta;
  if( a0 < 0.0 ){
    a0+= 2.0*M_PI;
  }

  *alpha = a0;
  *delta = d0;

  return;
}// toequinox2000()


//
// convert alpha delta (in radians) from 2000 to equinox 1950
// coordinates
//
// From The Astron. Almanac 1981 B18
inline void toequinox1950(double *alpha, double *delta, double t=2000.0)
{
  double a0, d0, sina0, cosa0, sind0, cosd0, sind, cosd, zeta, za, theta;
  double t2, t3;

  t-= 1950.0;
  t/= 100.0;

  t2 = t*t;
  t3 = t*t*t;

  zeta  = 0.6402633 * t + 0.0000839 * t2 + 0.0000050 * t3;
  za    = zeta + 0.0002197 * t2;
  theta = 0.5567376 * t + 0.0001183 * t2 + 0.0000117 * t3;

  zeta  *= M_PI/180.0;
  za    *= M_PI/180.0;
  theta *= M_PI/180.0;

  sind = sin( *delta );
  cosd = cos( *delta );

  sind0 = -cos( *alpha -za) * sin(theta) * cosd + cos(theta) * sind;
  d0    = asin( sind0 );
  cosd0 = cos( d0 );

  sina0 = sin( *alpha - za)*cosd/cosd0;
  cosa0 = (cos( *alpha - za)*cos(theta)*cosd + sin(theta)*sind)/cosd0;

  a0 = atan2( sina0, cosa0 );
  a0 -= zeta;
  if( a0 < 0.0 ){
    a0+= 2.0*M_PI;
  }

  *alpha = a0;
  *delta = d0;

  return;
}// toequinox1950()


//
// ignores stellar motions
//
inline void b1950toj2000(double *alpha, double *delta)
{
  double r, r1, r2, r3, x, y, z; // vector elements
  double a1, a2, a3, a;
  double cosa, sina, cosd, sind;

  r1 = cos( *alpha ) * cos( *delta );
  r2 = sin( *alpha ) * cos( *delta );
  r3 = sin( *delta );

  a  = ( -1.62557*r1 - 0.31919*r2 -0.13843*r3)*1.0e-6;
  a1 = a*r1 -1.62557e-6;
  a2 = a*r2 -0.31919e-6;
  a3 = a*r3 -0.13843e-6;
  r1 += a1;
  r2 += a2;
  r3 += a3;

  x = 0.9999256782 * r1 - 0.0111820611 * r2 - 0.0048579477 * r3;
  y = 0.0111820610 * r1 + 0.9999374784 * r2 - 0.0000271765 * r3;
  z = 0.0048579497 * r1 - 0.0000271474 * r2 + 0.9999881997 * r3;
  r  = sqrt(x*x + y*y + z*z);

  sind = z/r;
  cosd = sqrt(1.0 - sind*sind);
  cosa = (x/r)/cosd;
  sina = (y/r)/cosd;

  *alpha = atan2(sina, cosa);
  *delta = asin(sind);
  if( *alpha < 0.0 ) *alpha += 2.0 * M_PI;

  return;
}// b1950toj2000

//
// convert alpha delta 2000 (in radians) to galactic coordinates
// coordinates
//
inline void togalactic(double *alpha, double *delta, double equinox=2000.0)
{
  const double alpha_gpole = 192.25 * M_PI/180.0;
  const double delta_gpole = 27.4 * M_PI/180.0;
  //  const double alpha_gpole =  192.859340209* M_PI/180.0;
  //  const double delta_gpole =  27.128307186 * M_PI/180.0;
  const double gal_theta = 123.0 * M_PI/180.0;

  if( equinox != 1950.0 )
    toequinox1950( alpha,delta, equinox);

    //if( equinox != 2000.0 )
    //  toequinox2000( alpha,delta, equinox);

  double cosdgp =  cos( delta_gpole);
  double sindgp =  sin( delta_gpole);

  double sina, cosa , sinl, cosl, sinb, cosb, l, b;

  cosa = cos( *alpha - alpha_gpole );
  sina = sin( *alpha - alpha_gpole );

  sinb =  sindgp * sin( *delta ) + cosdgp * cos( *delta ) * cosa;
  b = asin( sinb );
  cosb = cos( b );

  if( fabs(b-0.5*M_PI) < 1.0e-6 )
    l = 0.0;
  else{
    cosl = (sin(*delta) - sindgp*sinb)/(cosdgp*cosb);
    sinl = cos(*delta)*sina/cosb;
    l = gal_theta - atan2(sinl,cosl);
    //    printf("%f\n",l);
    if( l < 0 )
      l = l + 2.0*M_PI;
  }
  *alpha = l;
  *delta = b;
  return;
}
#endif
  
