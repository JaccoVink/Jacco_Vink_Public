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
  class Profiles : BasicData {
  public:
    Profiles();
    ~Profiles();
    allocate(unsigned int n);
    deallocate( void );
    save( char filename[]);
  protected:
    unsigned int nbins;
    float *radius, *profile, *error;
  };
};
