#ifndef __JVTOOLS_REGION_H__
#define __JVTOOLS_REGION_H__

/*****************************************************************************/
//
// FILE        :  jvtools_region.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Definition of astroimage class.
//
//
// COPYRIGHT   : Jacco Vink,  COLUMBIA 2002
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
//
/*****************************************************************************/
#include "jvtools_image.h"      // include common.h


typedef enum ShapeTag{
  CIRCLE = 1, RECTANGLE = 2 , ELLIPSE = 3, POLYGON =4
} Shape;

namespace jvtools{
  class Image;
  class Region : public BasicData{
    friend class Image;
  public:
    Region();
    ~Region();
    Status read( const char filename[], const char extensionName[] );
    Status save( const char filename[], const char extensionName[] );
    Status readDS9(const char fileName[] );
    Status saveDS9(const char fileName[] );
    Status saveSAS(const char fileName[]);
    Status world2image(const Image & image);
    Status image2world(const Image & image);
    Status image2world2(const Image & image);
  protected:
  private:
    unsigned int nregions, *npoints;
    float *xpos, *ypos, *size1, *size2;
    double *angle;
    bool  *exclude;
    RegionSystem *system;
    Shape  *shape;

    Status allocate( unsigned int nreg, unsigned int np=DEFAULT_BUFFER_SIZE);
    Status deallocate();

  };//classe

}//namespace
#endif

