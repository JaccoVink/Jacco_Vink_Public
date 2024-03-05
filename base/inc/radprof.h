#ifndef __RADPROF_H__
#define __RADPROF_H__

/******************************************************************************/
//
// FILE        :  radprof.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Definition of eventlist class.
//
//
// COPYRIGHT   : Jacco Vink, SRON, 1996
//
/******************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/******************************************************************************/
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "common.h"
#include "xray.h"
#include "astrophys.h"
#include "/home/jaccov/c_code/fitsio/inc/fitsio.h"
#include "/home/jaccov/c_code/fitsio/inc/fitsio2.h"
#include "/home/jaccov/c_code/fitsio/inc/longnam.h"

#define NBUF        1024
#define NSEDOV      2048
#define ROSAT_PIX   0.5

class radprof
{
public:
	radprof(char inputDirectory[], char outputDirectory[], long nmax=100000, 
			Instrument instr = ASCASIS0 );
	~radprof();
	Status read(char fileName[], float xc, float yc, float rmin = 0.0, 
		    float rmax = 300.0, float phi1=0.0, float phi2=360.0 ,
			int chan1=1, int chan2=1024);
	Status binradii( float width );
	Status makeSedov( float rmax );
	Status write(char filename[]);
	Status testPrint(FILE *testStream );
protected:
private:
	char *inputDir;
	char *outputDir;

	long nevents, nmaxevents;
	int nbins;

	float *profile, *radval, *radius, *sedov;
	float centerX, centerY, centerRA, centerDec;
	float angle1, angle2, channel1, channel2, rmin, rmax;
	float binwidth;
	float crval1, crval2, crpix1, crpix2, cdelt1, cdelt2;
        float blank, equinox;

	Boolean sedovflag;
	Instrument instrument;
	Status select( int x[], int y[], int pi[], long nelem );
	void warning(char message[]);
};
#endif
