#ifndef __SEARCH_H__
#define __SEARCH_H__

/*****************************************************************************/
//
// FILE        :  search.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Typedefs for the search routines (application of astroimage)
//
//
// COPYRIGHT   : Jacco Vink, 1999
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/

#include "astroimage.h"
#define MAXSOURCES 5000

typedef struct source{ 
  float x;
  float y;
  float xsky;
  float ysky;
  float dloglik;
  float flux;
  float error;
  int i0, j0, in, jn;
} Pointsources;


/*****************************************************************************/
// Prototyping
//
//
/*****************************************************************************/
Status searchsource0( Pointsources *sources, int *nsources);
Status searchsource1( Pointsources *sources, float *background, int isource,
		      int nsources);
Status searchsource2( Pointsources *sources, float *background, int isource,
		      int nsources );
Status searchflux1( Pointsources *sources , float *background, int isource,
		    int nsources );
Status searchflux2( Pointsources *sources , float *background, int isource, 
		    int nsources, int nstep=50 );
Status searchflux3( Pointsources *sources , float *background, int isource, 
		    int nsources, int nstep=50 );

Status make_model( astroimage *newmodel, float flux, int i, int j,
		   Pointsources *sources , 
		   int nsources, int isource, float *background, float area);

Status initialize_list( Pointsources *sources, float flux0, int nx, int ny);

Status calcxy( Pointsources *sources, int nsources );
Status calclikelihood( float background, Pointsources *sources, int nsources );
Status printResults( FILE *fp, float background, Pointsources *sources, 
		     int nsources );

Status reducelist( float radius, int *xpos, int *ypos, float *flux, 
		   int *nsources);

Status makelist(Pointsources *sources, int nmax,
		char inputname[], char psfname[], char maskname[], 
		char outputname[], char outputlist[]);

#endif
