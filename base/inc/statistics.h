#ifndef __STATISTICS_H__
#define __STATISTICS_H__

/******************************************************************************/
/*
*  FILE        :  statistics.h
*  AUTHOR      :  Jacco Vink 
*  DESCRIPTION :  Prototyping for statistical functions.
*
*
*  COPYRIGHT   : Jacco Vink, SRON, 1995
*/
/******************************************************************************/
/*  MODIFICATIONS :                                                           
*
*/
/******************************************************************************/

#include "common.h"


#define MAX_RAND (float)(0x7FFFFFFF)
#define RANDOM   (float)rand()/MAX_RAND


typedef struct tagMoments
{
	float moment0, moment1, moment2, moment3, moment4;
} MomentType;

/* PROTOTYPING: */
float meanfValue( float *data, int ndata);
float stdDeviationf(float *data, int ndata);


#ifndef __cplusplus

Status 
binData( double *data, int ndata, double lim1, double lim2, double binwidth,	
                                  double *binvalues, double *bins, int *nbins );
#elseif

template <class T>
Status rebin( T *binvalues, T *bins, int *nbins, int bin1,int bin2,int factor );
template <class T>
Status binData2D( T *xdata, T *ydata, int ndata, T limx1, T limx2, 
                        T limy1, T limy2, T xbinwidth, T ybinwidth, T **bins, 
                                                     int *nxbins, int *nybins );
#endif


Status 
binDataF( float *data, int ndata, float lim1, float lim2, float binwidth, float 
	*binvalues, float *bins, int *nbins );
Status 
binData2D( double *xdata, double *ydata, int ndata, double limx1, double limx2,
 	double limy1, double limy2, double xbinwidth, double ybinwidth, 
	double **bins, int *nxbins, int *nybins );
Status 
binData2DF( float *xdata, float *ydata, int ndata, float limx1, float limx2,
 	float limy1, float limy2, float xbinwidth, float ybinwidth, 
	float **bins, int *nxbins, int *nybins );

Status 
binDataFadd( float *data, int ndata, float lim1, float lim2, 
		float binwidth, float *binvalues, float *bins, int nbins );
#endif
