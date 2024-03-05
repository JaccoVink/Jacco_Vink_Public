#include "common.h"

/*****************************************************************************/
//
// FUNCTION    : is_power_of_2()
// DESCRIPTION : Determines if an integer is a power of 2
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//       n               I                        input integer
//
//  RETURNS    : TRUE or FALSE
//
/*****************************************************************************/
Boolean is_power_of_2( int n )
{
  if( n < 1 ){
    return FALSE;
  }//if

  while( n > 2 ){
    if( n  % 2 == 1 ){
      return FALSE;
    }
    n /= 2; 
  }//while

  return TRUE;
}//  is_power_of_2( int n )
