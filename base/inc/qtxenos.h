#ifndef __QT_XENOS__H__
#define __QT_XENOS__H__

/*****************************************************************************/
//
// FILE        :  qt_xenos.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  X-ray astronomy base class.
//
//
// COPYRIGHT   : Jacco Vink,  2001
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
//#include "qtxenos.h"
#include "jvtools_image.h"


class QtXenos : public QWidget
{
  Q_OBJECT

    public:
  void something(){
    puts("Hallo");
    return;
  } 
 
};



#endif
