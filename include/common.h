#ifndef __COMMON_H__
#define __COMMON_H__

/*****************************************************************************/
/*
*  FILE        :  common.h
*  AUTHOR      :  Jacco Vink 
*  DESCRIPTION :  Contains common typedefand definitions.
*
*
*  COPYRIGHT   : Jacco Vink, SRON, 1995
*/
/*****************************************************************************/
/* MODIFICATIONS :                                                           
*
*
*/
/*****************************************************************************/
//#include "nr.h"
//#include "nrutil.h"

/* Typedefs: */

/* The following typedefs can be used to remind the size of the type
	e.g. Float4 is a 4 byte floating point number.
*/
//typedef int    Int4;
typedef long    Int4;
typedef short  Int2;
typedef float  Float4;
typedef double Float8;


typedef enum TagStatus{
     FAILURE = 0, SUCCESS = 1
} Status;

//typedef enum TagBoolean{
//     FALSE = 0, TRUE = 1
//} Boolean;

#ifndef TRUE
  #define TRUE true
  #define FALSE false
#endif

typedef bool Boolean;

/* Some definitions : */

#ifndef SQR
# define	SQR( a )    ( (a)*(a) )
# define	CUBE( a )  ( (a)*(a)*(a) )
#endif
#ifndef MAX
# define MAX( a, b) ( ((a) > (b)) ? (a) : (b) )
# define MIN( a, b) ( ((a) > (b)) ? (b) : (a) )
#endif
#ifndef ABS
# define ABS( a )   ( ((a) < 0) ? (-(a)) : (a) )
#endif
#if 0
#ifndef SIGN
# define SIGN( a )  ( ((a) < 0) ?  -1  : 1 )
#elseif
# define _SIGN( a )  ( ((a) < 0) ?  -1  : 1 )
#endif
#endif

# define _SIGN( a )  ( ((a) < 0) ?  -1  : 1 )

#ifndef exp10
#define	exp10( a )    ( exp(a*log(10.0)) )
#endif
/* EXIT ERRORS */

//#define RANDOM(a, b) ( rand()/RAND_MAX)*((b)-(a))+(a) )

enum ExitErrors{
 _NO_ERROR_ = 0, _NPARAM_ERROR_ = 1, _UNKNOWN_PARAM_ERROR_=2,
 _IO_ERROR_ = 3, _EXEC_ERROR_=4
};

#ifdef __cplusplus
// some common inline functions

template <class T>
inline T sqr( T val ) //returns the square of val
{
    return val*val;
}// end sqr()

template <class T>
inline T abs( T val ) //returns the absolute value  of val
{
    if( val > 0 )
    {
        return val;
    }
    else
    {
	return -val;
    }// end if
}// end abs()

template <class T>
inline T max( T val1, T val2)
{
    if( val1 >= val2 )
    {
        return val1;
    }
    else
    {
        return val2;
    }// end if
}// end max()


template <class T>
inline T min( T val1, T val2)
{
    if( val1 <= val2 )
    {
        return val1;
    }
    else
    {
        return val2;
    }// end if
}// end max()

#endif

#endif
