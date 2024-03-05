/*****************************************************************************/
//
// FILE        :  image_profiles.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Definition the  Profiles class.
//                Filtering functions and other functions
//
// COPYRIGHT   : Jacco Vink,  2002
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_image.h"

#define NMAX_PROF 4096

namespace jvtools{
  class Profile : BasicData {

  public:
    Profile();
    ~Profile();
    Status allocate(int n);
    Status deallocate( void );
    Status save( char filename[], float scaling=1.0);

  protected:
    int nbins;
    int *npixels;
    float *radius, *profile, *error;
  };
};
