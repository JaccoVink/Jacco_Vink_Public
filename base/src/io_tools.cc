/*****************************************************************************/
//
// FILE        :  io_tools.cc
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
#include <math.h>
#include <string.h>
#include "common.h"
#include "io_tools.h"


/*****************************************************************************/
//
// FUNCTION    : deg2hour_str()
// DESCRIPTION : Converts the input variable to a string. The input variable
//               is expected to be an angle in degrees. Converts to hours.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/*****************************************************************************/
char *deg2hour_str( char *s, double ra, bool omitIndicator)
{
  
  ra *= 12.0/180.0;
  if( ra >= 0.0 ){
    if( omitIndicator == true)
      sprintf(s, "%c%i %2i %3.3f",'+', (int)ra, (int)( (ra-(int)ra)*60.0),
	      ( ((ra-(int)ra)*60.0)-(int)( (ra-(int)ra)*60.0) )*60.0);
    else
      sprintf(s, "%c%2ih %2im %3.3fs",'+', (int)ra, (int)( (ra-(int)ra)*60.0),
	      ( ((ra-(int)ra)*60.0)-(int)( (ra-(int)ra)*60.0) )*60.0);
  }
  else{
    ra *= -1.0;
    if( omitIndicator == true)
      sprintf(s, "%c%i %2i %3.3f",'-', (int)ra, (int)( (ra-(int)ra)*60.0),
	  ( ((ra-(int)ra)*60.0)-(int)( (ra-(int)ra)*60.0) )*60.0);
    else
      sprintf(s, "%c%3ih %2im %3.3fs",'-', (int)ra, (int)( (ra-(int)ra)*60.0),
	      ( ((ra-(int)ra)*60.0)-(int)( (ra-(int)ra)*60.0) )*60.0);
  }
  return s;

}// deg2hour_str()



/*****************************************************************************/
//
// FUNCTION    : deg2deg_str()
// DESCRIPTION : Converts the input variable to a string. The input variable
//               is expected to be an angle in degrees.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/*****************************************************************************/
char *deg2deg_str( char *s, double deg, bool omitIndicator )
{
  
  if( deg >= 0.0 ){
    if( omitIndicator == true )
      sprintf(s, "%c%i  %2i %3.2f",'+',(int)deg, (int)((deg-(int)deg)*60.0),
	      ( ((deg-(int)deg)*60.0)-(int)( (deg-(int)deg)*60.0) )*60.0);
    else
      sprintf(s, "%c%3i  %2i' %3.2f\"",'+',(int)deg,(int)((deg-(int)deg)*60.0),
	      ( ((deg-(int)deg)*60.0)-(int)( (deg-(int)deg)*60.0) )*60.0);
  }
  else{
    deg *= -1.0;
    if( omitIndicator == true )
      sprintf(s, "%c%i  %2i %3.2f",'-',(int)deg,(int)((deg-(int)deg)*60.0),
	      ( ((deg-(int)deg)*60.0)-(int)( (deg-(int)deg)*60.0) )*60.0);
    else
      sprintf(s, "%c%3i  %2i' %3.2f\"",'-',(int)deg,(int)((deg-(int)deg)*60.0),
	  ( ((deg-(int)deg)*60.0)-(int)( (deg-(int)deg)*60.0) )*60.0);
  }
  return s;

}// deg2deg_str()


/*****************************************************************************/
//
// FUNCTION    : deg2hour_str2()
// DESCRIPTION : Converts the input variable to a string. The input variable
//               is expected to be an angle in degrees. Converts to hours.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/*****************************************************************************/
char *deg2hour_str2( char *s, double ra, bool omitIndicator)
{
  int rah, ramin;
  float rasec;
 
  ra *= 12.0/180.0;
  if( ra >= 0.0 ){
    rah = (int)ra;
    ramin = (int)( (ra - rah )*60.0);
    rasec = (ra - (float)rah - (float)ramin/60.0)*3600.0;
    if( fabs(rasec-60.0) < 0.1){
      rasec = 0.0;
      ramin++;
    }//if
    if( ramin-60 == 0){
      ramin = 0;
      rah++;
    }//if
    if( fabs( rasec - rint(rasec)) < 0.1 )
      sprintf(s, "%ih%im%.0fs",rah, ramin, rasec);
    else
      sprintf(s, "%ih%im%.1fs",rah, ramin, rasec);
  }
  else{
    ra *= -1.0;
    rah = (int)ra;
    ramin = (int)( (ra - rah )*60.0);
    rasec = (ra - (float)rah - (float)ramin/60.0)*3600.0;
    if( fabs(rasec-60.0) < 0.1){
      rasec = 0.0;
      ramin++;
    }//if
    if( ramin-60 == 0){
      ramin = 0;
      rah++;
    }//if
    if( fabs( rasec - rint(rasec)) < 0.1 )
      sprintf(s, "-%ih%im%.0fs",rah, ramin, rasec);
    else
      sprintf(s, "-%ih%im%.1fs",rah, ramin, rasec);
  }
  return s;

}// deg2hour_str2()


/*****************************************************************************/
//
// FUNCTION    : deg2deg_str()
// DESCRIPTION : Converts the input variable to a string. The input variable
//               is expected to be an angle in degrees.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/*****************************************************************************/
char *deg2deg_str2( char *s, double deg, bool omitIndicator)
{
  int degd, degmin;
  float degsec;

  if( deg >= 0.0 ){
    degd = (int)deg;
    degmin = (int)( (deg - degd )*60.0);
    degsec = (deg - (float)degd - (float)degmin/60.0)*3600.0;
    if( fabs(degsec-60.0) < 0.05){
      degsec = 0.0;
      degmin++;
    }//if
    if( degmin-60 == 0){
      degmin = 0;
      degd++;
    }//if
    sprintf(s, "%i%c%i'%.0f\"",degd, 248, degmin, degsec);
  }
  else{
    deg *= -1.0;
    degd = (int)deg;
    degmin = (int)( (deg - degd )*60.0);
    degsec = (deg - (float)degd - (float)degmin/60.0)*3600.0;
    if( fabs(degsec-60.0) < 0.05){
      degsec = 0.0;
      degmin++;
    }//if
    if( degmin-60 == 0){
      degmin = 0;
      degd++;
    }//if
    sprintf(s, "-%i%c%i'%.0f\"",degd, 248, degmin, degsec);
  }
  return s;

}// deg2deg_str2()

/*****************************************************************************/
//
// FUNCTION    : printlogo()
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status printlogo( char name[] )
{

  int i, strl = strlen(name);
  char noinfo[LOGOWIDTH], bottomline[LOGOWIDTH];
  char topline[LOGOWIDTH], infoline[LOGOWIDTH];
  char package[] = " ";
  char author[]  = "Jacco Vink";
  char company[] = " ";

  if( strl > LOGOWIDTH-4 || strl < 2 ){
    return FAILURE;
  }//if

  for( i = 0; i < LOGOWIDTH-1 ; i++){
    bottomline[i] = BORDERCHAR;
    if( i == 0 || i == LOGOWIDTH-2 ){
      noinfo[i] = BORDERCHAR;
    }else{
      noinfo[i] = SPACE;
    }//if
  }//for
  bottomline[LOGOWIDTH-1] =  EOSTR;
  noinfo[LOGOWIDTH-1] = EOSTR;

  inserttext( bottomline, name, topline );
  puts(topline);
  puts(noinfo);
  puts(noinfo);

  //  inserttext( noinfo, "ASTROIMAGE", infoline);
  inserttext( noinfo, name, infoline);
  puts(infoline);
  puts(noinfo);
  inserttext( noinfo, author, infoline);
  puts(infoline);
  inserttext( noinfo, company, infoline);
  puts(infoline);

  puts(noinfo);
  puts(bottomline);
  puts("");

  return SUCCESS;
}// printlogo()


/*****************************************************************************/
//
// FUNCTION    : copytext()
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
void inserttext( char basestr[], char infostr[], char *outputstr)
{
  int i, istart, strl = strlen(infostr);
  
  strcpy(outputstr, basestr );
  istart = LOGOWIDTH/2 - strl/2 - 2;
  outputstr[istart-1]    =  SPACE;
  outputstr[istart+strl] =  SPACE;
  for( i= istart ; i < istart+strl; i++){
    outputstr[i] = infostr[i-istart];
  }//for

}// inserttext()


/*****************************************************************************/
//
// FUNCTION    : printRADEC()
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  ra                   I                       ra (in decimal degrees)
//  dec                  I                       dec (in decimal degrees)
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status printRADEC( double ra, double dec)
{
  double alpha = ra, delta = dec , sign=1.0;

  alpha *= 12.0/180.0;
  printf("%d",(int)alpha);
  alpha -= (int)alpha;
  alpha *= 60.0;
  printf(" %d",(int)alpha);
  alpha -= (int)alpha;
  alpha *= 60.0;
  printf(" %.2f",alpha);
    
  sign = (delta < 0.0 ? -1.0 : 1.0);
  delta= fabs(delta);
  if( sign >= 0.0 ){
    printf("  %d",(int)delta);
  }
  else{
    printf("  -%d",(int)delta);
  }
  delta -= (int)delta;
  delta *= 60.0;
  printf(" %d",(int)delta);
  delta -= (int)delta;
  delta *= 60.0;
  printf(" %.2f",delta);

  printf("  (%.4f, %.4f)\n", ra, dec);
  
  return SUCCESS;
}//printRADEC()


