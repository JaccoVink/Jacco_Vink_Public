#ifndef __XRAY_H__
#define __XRAY_H__

/*****************************************************************************/
//
// FILE        :  xray.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Header file containing some usefull definitions for xray-
//                astronomy
//
//
// COPYRIGHT   : Jacco Vink, SRON, 1998
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "common.h"

// XMM RELATED

#define BAD_RGS_EVENTS 0x82c0000
//#define BAD_RGS_EVENTS 0x82a0000
//#define BAD_MOS_EVENTS 0x366b0000
//#define BAD_PN_EVENTS  0xfb0000
#define BAD_PN_EVENTS 0xfa0000
#define BAD_MOS_EVENTS 0x766b0000
#define OUT_OF_FOV 0x10000
#define OUT_OF_FOV 0x10000
#define MAX_PN_PATTERN  12
#define MAX_MOS_PATTERN  12
#define CLOSE_TO_CCD_WINDOW 0x4
#define ON_OFFSET_COLUMN    0x8
#define NEXT_TO_OFFSET_COLUMN 0x10
#define OUT_OF_CCD_WINDOW 0x2000000

#define ON_BAD_PIX
#define NEXT_TO_CCDBORDER


#define SINGLE_MOS_EVENT 0

#define XMM_FOCAL_LENGTH 7.500 // meter
//#define DEFAULT_MOS1_MAX_CNTRATE 0.136
#define DEFAULT_MOS1_MAX_CNTRATE 0.136
#define DEFAULT_MOS2_MAX_CNTRATE 0.2
#define DEFAULT_PN_MAX_CNTRATE 0.2
#define DEFAULT_MAX_CNTRATE 1.0


// RGS related:
//#define BETA_0   2.1395     //degrees
#define BETA_0   2.0     //degrees
//#define BETA_0   2.1586     //degrees
#define RGSALPHA 2.74285152554512E-02 // in rad = 1.576191 degr.
#define DGROOVE 1.54894550781250E+04 // in angstrom
#define ANG2ENERGY 12.398      // angstrom to energy
#define BETA_OFFSETR1R2   -7.0
#define XDSP_OFFSETR1R2   4.0



typedef enum tagInstrument
{
	ASCASIS0=0, ASCASIS1=1, ASCAGIS=2, ROSAT_PSPC = 3, ROSAT_HRI= 4,
	SAX_LE = 10, SAX_ME = 11, SAX_HP = 12, SAX_PDS = 13, CGRO_OSSE=15,
	XTE =8,
	EXOSAT_LE = 20, EXOSAT_ME = 21, 
	EINSTEIN_SSS = 30, EINSTEIN_HRI= 31,
	XMM_PN = 40, XMM_MOS1= 41, XMM_MOS2 = 42, XMM_RGS1=43, XMM_RGS2=44,
	XMM_OM = 45,
	CHANDRA_ACIS = 50, CHANDRA_HRC=51,
	CHANDRA_METG_ACIS=52, CHANDRA_HETG_ACIS=53,
	CHANDRA_LETG_ACIS= 54, CHANDRA_LETG_HRC=55,
	SIXA=60, 
	INTEGRAL_IBIS=70, INTEGRAL_SPI=72, INTEGRAL_JEMX=74,
	SUZAKU_XIS=80, 	SUZAKU_XIS0=81, SUZAKU_XIS1=82, SUZAKU_XIS2=83,SUZAKU_XIS3=84,
	SWIFT_XRT=90,
	HITOME=92,
	XIPE_GPD=94,
	IXPE_GPD=95,
	SPITZER_IRAC=200,
	SPITZER_MIPS=201,
	SPITZER_IRS=202,
	HERSCHEL_PACS=210,
	FERMI_LAT=300,
	HST_ACS=401,
	UNDEF = -1
} Instrument;

typedef enum tagRegionSystem
{
  IMAGE=0, FK5=1, ICRS=2, BETA_XDSP=3, BETA_PI=4
}RegionSystem;

typedef struct psfpar{
  float sigma, sa, sb, sc, sd, se;
  float rl, rla, rlb, rlc, rld, rle;
  float me, mea, meb, mec, med, mee;
  float re, rea, reb, rec, red, ree;
  float toarcmin, ps, cg, cl, norm;
}Psfpar;



float totalPSPCpsf( float r, float energy );
float gaussianpsf( float r, float energy );
float exppsf( float r, float energy);
float lorentzianpsf( float r, float energy );
float gaussianfrac(float energy );
float expfrac(float energy );
float lorentzianfrac( float energy );
float psfPSPCintegral( float rmax, float energy);


Instrument getinstrument(char instrStr[] );
Status readMECSpar( char filename[], Psfpar *mecs);
Status calcpsf(float energy, Psfpar *mecs);


#endif




