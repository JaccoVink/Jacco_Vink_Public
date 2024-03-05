#ifndef __JVTOOLS_QT__H__
#define __JVTOOLS_QT__H__

/*****************************************************************************/
//
// FILE        :  jvtools_qt.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  
//
//
// COPYRIGHT   : Jacco Vink,  2002
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include "jvtools_image.h"


class QtJVTools : public QWidget
{
  Q_OBJECT
public:

public slots:
    Status extractImage( void );
};



#endif
