#ifndef __IO_TOOLS__H__
#define __IO_TOOLS__H__

/*****************************************************************************/
//
// FILE        :  io_tools.h
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  definitions for io tools.
//
//
// COPYRIGHT   : Jacco Vink,  1999
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/


#define SPACE            32
#define EOSTR             0
#define BORDERCHAR       '*'
#define LOGOWIDTH        60

char *deg2hour_str( char *s, double ra,  bool omitIndicator=false);
char *deg2deg_str( char *s, double deg,  bool omitIndicator=false);
char *deg2hour_str2( char *s, double ra, bool omitIndicator=false);
char *deg2deg_str2( char *s, double deg, bool omitIndicator=false);


void inserttext( char basestr[], char infostr[], char *outputstr);
Status printlogo( char name[] );





#endif
