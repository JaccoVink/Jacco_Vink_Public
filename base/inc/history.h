#ifndef __HISTORY__H__
#define __HISTORY__H__

/*****************************************************************************/
//
// FILE        :  history.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  
//
//
// COPYRIGHT   : Jacco Vink,  1999
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include <stdio.h>
#include <string.h>


#define HIST_LEN 70

class History
{
 public:
  History();// constructor
  ~History(); // destructor
  void destroy();
  void insert_text( char newtext[]);
  void print();
 protected:

 private:
  char text[HIST_LEN];
  History *next;
};//

#endif
