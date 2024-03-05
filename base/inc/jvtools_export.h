#ifndef __JVTOOLS_EXPORT_H__
#define __JVTOOLS_EXPORT_H__

/*****************************************************************************/
//
// FILE        :  jvtools_export.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  
//
//
// COPYRIGHT   : Jacco Vink, SRON, 1996, AIP, 1999, COLUMBIA 2000/1
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
//#include "jvtools_basic.h"      // include common.h

namespace jvtools{
  class ImageExport
    {
    public:
      ImageExport(void);
      //      ~ImageExport(){return;};
      Status writejpeg(char filename[], 
		       const Image & imR, 
		       const Image & imG,
		       const Image & imB,
		       float rf, float gf, float bf, int quality);      
    };
};
#endif
