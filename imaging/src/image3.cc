/*****************************************************************************/
//
// FILE        :  image3.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Implementation of the  Image class.
//                Filtering functions and other functions
//
// COPYRIGHT   : Jacco Vink, SRON, 1998
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_image.h"
//#include "nr.h"

using namespace::JVMessages;
using namespace::jvtools;

//
// Simple sorting routine used by medianfilter() and selectregion()
//
// inline for speed reasons
inline void sort(float data[], int n)
{
  float a;
  int i, j;

  if(n<2 ){
    data[0] = 0.0;
    return;
  }

  for( j = 1; j < n; j++){
    a = data[j];
    i= j-1;
    while( i>= 0 &&  data[i] > a ){
      data[i+1] = data[i];
      i--;
    }//while
    data[i+1] = a;
  }//for
  
}//end sort()

//
// based on select
// from Numerical Recipes in C, Press et al.
#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;
inline float select(unsigned long k, unsigned long n, float arr[])
{
	unsigned long i,ir,j,l,mid;
	float a,temp;

	l=1;
	ir=n;
	for (;;) {
		if (ir <= l+1) {
			if (ir == l+1 && arr[ir] < arr[l]) {
				SWAP(arr[l],arr[ir])
			}
			return arr[k];
		} else {
			mid=(l+ir) >> 1;
			SWAP(arr[mid],arr[l+1])
			if (arr[l] > arr[ir]) {
				SWAP(arr[l],arr[ir])
			}
			if (arr[l+1] > arr[ir]) {
				SWAP(arr[l+1],arr[ir])
			}
			if (arr[l] > arr[l+1]) {
				SWAP(arr[l],arr[l+1])
			}
			i=l+1;
			j=ir;
			a=arr[l+1];
			for (;;) {
				do i++; while (arr[i] < a);
				do j--; while (arr[j] > a);
				if (j < i) break;
				SWAP(arr[i],arr[j])
			}
			arr[l+1]=arr[j];
			arr[j]=a;
			if (j >= k) ir=j-1;
			if (j <= k) l=i;
		}
	}
}
#undef SWAP
/* (C) Copr. 1986-92 Numerical Recipes Software *;#1. */

/****************************************************************************/
//
// FUNCTION    : selectregion()
// DESCRIPTION : Creates a mask with all selected regions having the region
//               number, all neutral regions having -1 and all exclude regions
//               having -2. 
//               
//               
//         
//
//
//
// SIDE EFFECTS: UNDER CONSTRUCTION
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      regionfile       I                    filename
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
#define SPACE 32
#define LINELENGTH 1024
Status Image::selectRegion(const char regionfile[], bool chat)
{

  setFunctionID("selectRegion");

  message("************* EXPERIMENTAL ******************");

  const int idlen=30;
  FILE *fp;
  char ch, sign='+', identifier[idlen], line[LINELENGTH];
  int i, nreg = 0;
  bool globalSystem=false;
  RegionSystem system=IMAGE;

  setblank(0.0);
  //
  // first read the region file
  //
  if( !(fp=fopen(regionfile,"r")) )
    return error("cannot open region file ", regionfile);

  //    warning("selectregion(): Reading the following region file:");
  //    warning(regionfile);
  do{
    if( fscanf(fp,"%c",&ch) ==EOF){
      fclose(fp);
      return error("error reading file");
    }
    if( ch=='#'){
      fgets(line,LINELENGTH,fp);
      if( line[strlen(line)-1] == '\n' )
	line[strlen(line)-1] = '\0';
      JVMessages::message("region file comments #", line);
      //      do{
      //	fscanf(fp,"%c",&ch2);
      //      }while( ch2 != '\n' );
    }
  }while( ch=='#' );
  
  do{

    if( chat )
      JVMessages::message("reading region number ",(long)(nreg+1));

    // read to the end of line
    while( ch == SPACE || ch == '\n' ){
      if( fscanf(fp,"%c",&ch)== EOF){ // should be space or '+' or '-'
	fclose(fp);
	if( nreg != 0){
	  return JVMessages::success();
	}
	return JVMessages::error("parse error");
      }
    }// read space (char 32)
    sign = '+';
#if 0    
    // no longer needed see below
    if( ch== '-' ){
      sign = '-';
      message("found an exclusion region");
    }//if
#endif

    // read the identifier
    i=0;
    while( ch != '(' && ch != '\n' && ch != SPACE && i < idlen ){
      if( ch != '+' && ch != '-' ) // ignore + or -
	identifier[i++] = ch;
      else if( ch == '-' ){
	sign = '-';
	JVMessages::message("found an exclusion region");
      }
      else
	sign = '+';
      fscanf(fp,"%c",&ch);
      
    }// while
    identifier[i] = 0; // add end of string char


    if( strncasecmp(identifier,"GLOBAL",idlen) != 0 )
      while( ch != '(' )
	fscanf(fp,"%c",&ch);
    message("idlen = ",idlen);
    message("idlen = ",identifier);
    if( strncasecmp(identifier,"fk5",idlen) != 0 && strlen(identifier) == 3){
      system=FK5;
      globalSystem=true;
      message("System globally set to FK5");
      while( ch != '(' )
	fscanf(fp,"%c",&ch);
      message("region type", identifier);
    }else{
      if( strncasecmp(identifier,"IMAGE;",6) == 0){
	JVMessages::message("coordinate system = ","IMAGE");
	system = IMAGE;
      }else if( strncasecmp(identifier,"FK5;",4) == 0){
	JVMessages::message("coordinate system = ","FK5");
	//      JVMessages::warning("coordinate system not supported yet!");
	system = FK5;
      }else if( strncasecmp(identifier,"ICRS;",5) == 0){
	JVMessages::message("coordinate system = ","ICRS");
	//      JVMessages::warning("coordinate system not supported yet!");
	system = FK5;
      }// else if
    

      if( strncasecmp(identifier,"POLYGON",7) == 0 ||
	  strncasecmp(identifier,"IMAGE;POLYGON",13) == 0 ||
	  strncasecmp(identifier,"FK5;POLYGON",13) == 0 ){
	nreg++;
	if( selectpolygon( fp, sign, system, nreg)== FAILURE ){
	  fclose(fp);
	  return error("error selecting polygon");
	}
	fgets(line, LINELENGTH-1, fp);
      }else if( strncasecmp(identifier,"CIRCLE",6) == 0 ||
		strncasecmp(identifier,"IMAGE;CIRCLE",12) == 0 ||
		strncasecmp(identifier,"FK5;CIRCLE",12) == 0){
	nreg++;
	if( selectcircle( fp, sign, system, nreg)== FAILURE ){
	  fclose(fp);
	  return error("error selecting circle");
	}
	fgets(line, LINELENGTH-1, fp);
      }else if( strncasecmp(identifier,"ELLIPSE",7) == 0||
		strncasecmp(identifier,"IMAGE;ELLIPSE",13) == 0 ||
		strncasecmp(identifier,"FK5;ELLIPSE",13) == 0 ){
	nreg++;
	if( selectellipse( fp, sign,system, nreg)== FAILURE ){
	  fclose(fp);
	  return error("error selecting ellipse");
	}
	fgets(line, LINELENGTH-1, fp);
      }else if( strncasecmp(identifier,"BOX",3) == 0 ||
		strncasecmp(identifier,"IMAGE;BOX",9) == 0 ||
		strncasecmp(identifier,"FK5;BOX", 7) == 0){
	nreg++;
	if( selectbox( fp, sign, system, nreg)== FAILURE ){
	  fclose(fp);
	  return error("error selecting box");
	}
	fgets(line, LINELENGTH-1, fp);
      }else if( strncasecmp(identifier,"GLOBAL",6) == 0){
	JVMessages::message("found ds9 type of region file");
	fgets(line,LINELENGTH,fp);
	if( line[strlen(line)-1] == '\n' )
	line[strlen(line)-1] = '\0';
	JVMessages::message("attributes:", line);
      }else if( strncasecmp(identifier,"IMAGE",6) == 0){
	JVMessages::message("found ds9 type of region file");
	JVMessages::message("coordinate system = ","IMAGE");
	system = IMAGE;
	fgets(line,LINELENGTH,fp);
	if( line[strlen(line)-1] == '\n' )
	  line[strlen(line)-1] = '\0';
	JVMessages::message("attributes:", line);
      }else 
	return JVMessages::error("unsupported REGION type: ", identifier);
    }//else

  }while( fscanf(fp, "%c",&ch)!= EOF );
  
  fclose(fp);

  message("number of regions = ", nreg);
  if( nreg == 1 && sign == '-' ){
    message("only one region and this region is an exclusion region");
    *this += 2.0;
  }
  return JVMessages::success();

}// selectregion()


/****************************************************************************/
//
// FUNCTION    : selectregion()
// DESCRIPTION : 
//               
//               
//         
//
//
//
// SIDE EFFECTS: UNDER CONSTRUCTION
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      regionfile       I                    filename
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::selectregion(const char regionfile[], bool chat)
{

  setFunctionID("selectregion");

  const int idlen=30;
  FILE *fp;
  char ch, sign='+', identifier[idlen], line[LINELENGTH];
  int i, nreg=0;
  RegionSystem system=IMAGE;

  setblank(0.0);
  //
  // first read the region file
  //
  if( !(fp=fopen(regionfile,"r")) )
    return JVMessages::error("cannot open region file ", regionfile);

  //    warning("selectregion(): Reading the following region file:");
  //    warning(regionfile);
  do{
    if( fscanf(fp,"%c",&ch) ==EOF){
      fclose(fp);
      return JVMessages::error("error reading file");
    }
    if( ch=='#'){
      fgets(line,LINELENGTH,fp);
      if( line[strlen(line)-1] == '\n' )
	line[strlen(line)-1] = '\0';
      JVMessages::message("region file comments #", line);
      //      do{
      //	fscanf(fp,"%c",&ch2);
      //      }while( ch2 != '\n' );
    }
  }while( ch=='#' );
  
  do{

    if( chat )
      JVMessages::message("reading region number ",(long)(nreg+1));

    // read to the end of line
    while( ch == SPACE || ch == '\n' ){
      if( fscanf(fp,"%c",&ch)== EOF){ // should be space or '+' or '-'
	fclose(fp);
	if( nreg!= 0){
	  return JVMessages::success();
	}
	return JVMessages::error("parse error");
      }
    }// read space (char 32)
    sign = '+';
#if 0    
    // no longer needed see below
    if( ch== '-' ){
      sign = '-';
      message("found an exclusion region");
    }//if
#endif

    // read the identifier
    i=0;
    while( ch != '(' && ch != '\n' && ch != SPACE && i < idlen ){
      if( ch != '+' && ch != '-' ) // ignore + or -
	identifier[i++] = ch;
      else if( ch == '-' ){
	sign = '-';
	JVMessages::message("found an exclusion region");
      }
      else
	sign = '+';
      fscanf(fp,"%c",&ch);
      
    }// while
    identifier[i] = 0; // add end of string char


    if( strncasecmp(identifier,"GLOBAL",idlen) != 0 )
      while( ch != '(' )
	fscanf(fp,"%c",&ch);
    
    message("region type", identifier);
    if( strncasecmp(identifier,"IMAGE;",6) == 0){
      JVMessages::message("coordinate system = ","IMAGE");
      system = IMAGE;
    }else if( strncasecmp(identifier,"FK5;",4) == 0){
      JVMessages::message("coordinate system = ","FK5");
      //      JVMessages::warning("coordinate system not supported yet!");
      system = FK5;
    }else if( strncasecmp(identifier,"ICRS;",5) == 0){
      JVMessages::message("coordinate system = ","ICRS");
      //      JVMessages::warning("coordinate system not supported yet!");
      system = FK5;
    }// else if

    if( strncasecmp(identifier,"POLYGON",7) == 0 ||
	strncasecmp(identifier,"IMAGE;POLYGON",13) == 0 ||
	strncasecmp(identifier,"FK5;POLYGON",13) == 0 ){
      nreg++;
      if( selectpolygon( fp, sign, system)== FAILURE ){
	fclose(fp);
	return error("error selecting polygon");
      }
    }else if( strncasecmp(identifier,"CIRCLE",6) == 0 ||
	      strncasecmp(identifier,"IMAGE;CIRCLE",12) == 0 ||
	      strncasecmp(identifier,"FK5;CIRCLE",12) == 0){
      nreg++;
      if( selectcircle( fp, sign, system)== FAILURE ){
	fclose(fp);
	return error("error selecting circle");
      }
    }else if( strncasecmp(identifier,"ELLIPSE",7) == 0||
	      strncasecmp(identifier,"IMAGE;ELLIPSE",13) == 0 ||
	       strncasecmp(identifier,"FK5;ELLIPSE",13) == 0 ){
      nreg++;
      if( selectellipse( fp, sign,system)== FAILURE ){
	fclose(fp);
	return error("error selecting ellipse");
      }
    }else if( strncasecmp(identifier,"BOX",3) == 0 ||
	      strncasecmp(identifier,"IMAGE;BOX",9) == 0 ||
	      strncasecmp(identifier,"FK5;BOX", 7) == 0){
      nreg++;
      if( selectbox( fp, sign, system)== FAILURE ){
	fclose(fp);
	return error("error selecting box");
      }
    }else if( strncasecmp(identifier,"GLOBAL",6) == 0){
      JVMessages::message("found ds9 type of region file");
      fgets(line,LINELENGTH,fp);
      if( line[strlen(line)-1] == '\n' )
	line[strlen(line)-1] = '\0';
      JVMessages::message("attributes:", line);
    }else if( strncasecmp(identifier,"IMAGE",6) == 0){
      JVMessages::message("found ds9 type of region file");
      JVMessages::message("coordinate system = ","IMAGE");
      system = IMAGE;
      fgets(line,LINELENGTH,fp);
      if( line[strlen(line)-1] == '\n' )
	line[strlen(line)-1] = '\0';
      JVMessages::message("attributes:", line);
    }
    else 
      return JVMessages::error("unsupported REGION type: ", identifier);
  }while( fscanf(fp, "%c",&ch)!= EOF );
  
  fclose(fp);
  return JVMessages::success();

}// selectregion()



/*****************************************************************************/
//
// FUNCTION    : selectpolygon()
// DESCRIPTION : Should be used by selectregion(). This file reads
//               the points form a saoimage region file, whereas selectregion
//               already read the identifier being 'POLYGON'.
//               That part of the image falling within the polygon
//               is not changed, outside the polgygon will be set to BLANK
//               The opposite is true if sign=='-'
//
//
//
// SIDE EFFECTS: UNDER CONSTRUCTION
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      fp              I/O                    filepointer
//      sign            I                      characer indicating if the
//                                             region is an exclusion region
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
#define NPOL 1024
Status Image::selectpolygon( FILE *fp, char sign, RegionSystem system, int nReg )
{
  char ch;
  int npol, nborders,  i, j, k;
  double xpol[NPOL], ypol[NPOL]; // polygon coordinates
  float  xborders[NPOL];
  double a, b; 

  setFunctionID("selectpolygon");
  //puts("polgyon");
  if( system != FK5 && system != IMAGE )
    return error("unsupported coordinate system");

  //  if( nReg > 0 )
  //    return error("new method does not yet work for polygons");

  // Now read the coordinate numbers:
  npol= i = 0;
  if( fscanf(fp,"%lf",&xpol[npol]) ){
      i++;
      //      printf("%f ", xpol[npol]);
      while( fscanf(fp,",%lf",&a) && npol < NPOL){
	printf("%f ", a);	
	if( i % 2 == 0){
	  xpol[npol] = a;
	}
	else{
	  ypol[npol++] = a;
	}
      i++;
      }//while
  }//if
  //  printf("\n");
  if( i % 2 != 0 || npol < 3){
    message(" entries ", i);
    message(" pairs ", npol);
    return error("error incorrect number of ordinates");
  }

  xpol[npol]   = xpol[0]; // make the polygon cyclic
  ypol[npol++] = ypol[0]; // make the polygon cyclic


  if( system == FK5 )
    for( int m =0 ; m < npol; m++ )
      getxy( xpol[m], ypol[m], &xpol[m], &ypol[m] );


  do{
    fscanf(fp, "%c",&ch);
  }while( ch != ')' ); // Read to the end of the region
  
  //
  // Here we will start selecting the region in the image
  //
  for( j=0; j < ny; j++){ // for every row in the image do:

    // look for the crossing pixels 
    nborders=0;
    for( k = 1; k < npol; k++){
      // make a solution of the form y=ax+b -> x
      if( (ypol[k-1] < j+1.0 && ypol[k] >= j+1.0) ||
	  (ypol[k] < j+1.0 && ypol[k-1] >= j+1.0)     ){
	if( fabs(xpol[k]-xpol[k-1]) < 0.1 ){
	  xborders[nborders++] = xpol[k];
	}
	else{
	  a = (ypol[k]-ypol[k-1])/(xpol[k]-xpol[k-1]); //slope
	  b = ypol[k] - a * xpol[k];
	  xborders[nborders++] = (float)(j+1.0 - b)/a;
	}//else
      }//if

    } //for k

    sort(xborders,nborders);

    // check all pixels in row:
    k = 0;
    for( i = 0;  i < nx ; i++){
      if( nborders > 0 ){
	// the following depends on the order of execution
	// the 2nd may not be evaluated if k<nborders
	while( k < nborders && xborders[k] < i+0.5  ){
	  k++;
	}
	if( nReg > 0 ){ // new Method

	  if( (k % 2 != 0 && sign != '-' ) )
	    pixel[i + j*nx] = nReg;
	  if( (k % 2 != 0 && sign == '-') )
	    pixel[i + j*nx] = EXCLUDE_REGION;
	}else{ // old method

	  if( (k % 2 == 0 && sign != '-' ) || (k % 2 != 0 && sign=='-') )
	    pixel[i + j*nx] = blank;
	}//else

      }else{ // if nborders > 0
	if( nReg < 0 && sign != '-' )//old method
	  pixel[i + j*nx] = blank;
      }//else
    }//for i

  }//for j


  return success();
}//end selectpolygon

/*****************************************************************************/
//
// FUNCTION    : selectcircle()
// DESCRIPTION : Should be used by selectregion(). This file reads
//               the circle parameters from a saoimage region file, 
//               whereas selectregion
//               already read the identifier, being 'CIRCLE'.
//               That part of the image falling within the circle
//               is not changed, outside the polgygon will be set to BLANK
//               The opposite is true if sign=='-'
//
//
//
// SIDE EFFECTS: UNDER CONSTRUCTION
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      fp              I/O                    filepointer
//      sign            I                      characer indicating if the
//                                             region is an exclusion region
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::selectcircle( FILE *fp, char sign, RegionSystem system, 
			    int nReg )
{
  char ch;
  int i, j;
  double xc, yc, r, rsqr, rpsqr;

  setFunctionID("selectCircle");

  switch( system ){
  case IMAGE:
    if(! ( fscanf(fp,"%lf ,",&xc) && fscanf(fp,"%lf ,",&yc)  && 
	   fscanf(fp,"%lf",&r) ) )
      return error("error reading parameters");
    break;
  case FK5:
    if(! ( fscanf(fp,"%lf ,",&xc) && fscanf(fp,"%lf ,",&yc)  && 
	   fscanf(fp,"%lf\"",&r) ) )
      return error("error reading parameters");
    message("region system FK5 is still experimental!");
    if( fabs(cdelt1) < 1.0e-10 ||
	fabs(fabs(cdelt1) - fabs(cdelt2))/fabs(cdelt1) > 1.0e-4 )
      return error("pixel x & y size are unequal, unable to continue");
#if 1
    message("center x sky coordinate: ", xc);
    message("center y sky coordinate: ", yc);
    message("radius in arcsex: ", r);
#endif
    getxy(xc, yc, &xc, &yc);
    r /= 3600.0;
    r /= fabs(cdelt1);
#if 1
    message("center x image coordinate: ", xc);
    message("center y image coordinate: ", yc);
    message("radius in pixels: ", r);
#endif
    break;
  default:
    return error("region system not supported");
  }//switch

  do{
    fscanf(fp, "%c",&ch);
  }while( ch != ')' );

  if( nReg > 0 )
    message("using new method");

  rsqr = r*r;
  for(j=0; j < ny; j++ ){
    for(i=0; i< nx; i++){
      rpsqr= SQR(i + 1.0   - xc) + SQR(j + 1.0 - yc);

      if( nReg > 0 ){ // new mask method
	rpsqr /= rsqr;
	if( pixel[ i + nx*j] > IGNORE_REGION ){ 
	  if( rpsqr <= 1.0 && sign != '-')
	    pixel[ i + nx*j] = nReg;
	  else if ( rpsqr <= 1.0 && sign == '-') 
	    pixel[ i + nx*j] = EXCLUDE_REGION;
	}//if
      }
      else{ // old method
	if( (rpsqr > rsqr && sign != '-') || (rpsqr <= rsqr && sign == '-') )
	  pixel[ i + nx*j] = blank;
      }//else

    }//for i
  }//for  j

  return success();

}//selectcircle()



/*****************************************************************************/
//
// FUNCTION    : selectellipse()
// DESCRIPTION : Should be used by selectregion(). This file reads
//               the circle parameters from a saoimage region file, 
//               whereas selectregion
//               already read the identifier, being 'ELLIPSE'.
//               That part of the image falling within the ellipse
//               is not changed, outside the polgygon will be set to BLANK
//               The opposite is true if sign=='-'
//
//
//
// SIDE EFFECTS: UNDER CONSTRUCTION
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      fp              I/O                    filepointer
//      sign            I                      characer indicating if the
//                                             region is an exclusion region
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::selectellipse( FILE *fp, char sign, RegionSystem system, 
			     int nReg  )
{
  char ch;
  int i, j;
  double xc, yc, rs, rl, angle, rssqr, rlsqr;
  double x, y, sina, cosa, rpsqr;

  setFunctionID("selectellipse");

  switch( system ){
  case IMAGE:
    if( !( fscanf(fp,"%lf,",&xc) && fscanf(fp,"%lf,",&yc) && 
	   fscanf(fp,"%lf,",&rl) && fscanf(fp,"%lf,",&rs) &&  
	   fscanf(fp,"%lf",&angle) ) )
      return error("error reading parameters!");
    break;
  case FK5:
    if( !( fscanf(fp,"%lf,",&xc) && fscanf(fp,"%lf,",&yc) && 
	   fscanf(fp,"%lf\",",&rl) && fscanf(fp,"%lf\",",&rs) &&  
	   fscanf(fp,"%lf",&angle) ) )
      return error("error reading parameters!");
    message("region system FK5 is still experimental!");
    if( fabs(cdelt1) < 1.0e-10 ||
	fabs(fabs(cdelt1) - fabs(cdelt2))/fabs(cdelt1) > 1.0e-4 )
      return error("pixel x & y size are unequal, unable to continue");
#if 0
    message("center x sky coordinate: ", xc);
    message("center y sky coordinate: ", yc);
    message("semi major axis lenght in arcsec: ", rl);
    message("semi minor axis lenght in arcsec: ", rs);
    message("angle (degrees) = ", angle);
#endif
    getxy(xc, yc, &xc, &yc);
    rl /= 3600.0;
    rs /= 3600.0;
    rl /= fabs(cdelt1);
    rs /= fabs(cdelt1);

#if 0
    message("center x image coordinate: ", xc);
    message("center y image coordinate: ", yc);
    message("semi major axis lenght in pixels: ", rl);
    message("semi minor axis lenght in pixels: ", rs);
#endif
    break;
  default:
    return error("region system not supported");
  }//

  do{
    fscanf(fp, "%c",&ch);
  }while( ch != ')' );


  if( rl <= 0.0 || rs <=0.0 )
    return error("wrong ellipse axes");



  angle*= M_PI/180.0;
  sina = -sin(angle);
  cosa = cos(angle);
  rssqr= rs*rs;
  rlsqr= rl*rl;

  for(j=0; j < ny; j++ ){
    for(i=0; i< nx; i++){
      x = (i+1.0 - xc)*cosa - (j+1.0 - yc)*sina;
      y = (i+1.0 - xc)*sina + (j+1.0 - yc)*cosa;;
      rpsqr= x*x/rlsqr + y*y/rssqr;
      if( nReg > 0 ){
	if( pixel[ i + nx*j] > IGNORE_REGION ){ 
	  if( rpsqr <= 1.0 && sign != '-')
	    pixel[ i + nx*j] = nReg;
	  else if ( rpsqr <= 1.0 && sign == '-') 
	    pixel[ i + nx*j] = EXCLUDE_REGION;
	}//if
      }
      else{ //old method

	if( (rpsqr > 1.0 && sign != '-') || (rpsqr <= 1.0 && sign == '-') )
	  pixel[ i + nx*j] = blank;
	
      }//else
    }//for i
  }//for  j

  return success();

}//selectellipse()

/*****************************************************************************/
//
// FUNCTION    : selectbox()
// DESCRIPTION : Should be used by selectregion(). This file reads
//               the circle parameters from a saoimage region file, 
//               whereas selectregion
//               already read the identifier, being 'BOX'.
//               That part of the image falling within the box
//               is not changed, outside the polgygon will be set to BLANK
//               The opposite is true if sign=='-'
//
//
//
// SIDE EFFECTS: UNDER CONSTRUCTION
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//      fp              I/O                    filepointer
//      sign            I                      characer indicating if the
//                                             region is an exclusion region
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::selectbox( FILE *fp, char sign, RegionSystem system,int nReg  )
{

  setFunctionID("selectBox");
  if( 0 && nReg > 0 ){
    message("**********************************************\n");
    message("still experimental mask creation, nReg = ", nReg);
    message("**********************************************\n");
  }
  char ch;
  int i, j;
  double xc, yc, side1, side2, angle=0.0;
  double x, y, sina, cosa;
  switch( system ){
  case IMAGE:
    if( !( fscanf(fp,"%lf,",&xc) && fscanf(fp,"%lf,",&yc) && 
	   fscanf(fp,"%lf,",&side1) && fscanf(fp,"%lf,",&side2) ) ){
      return JVMessages::error("error reading parameters");
    }//if
    break;
  case FK5:
    if( !( fscanf(fp,"%lf,",&xc) && fscanf(fp,"%lf,",&yc) && 
	   fscanf(fp,"%lf\",",&side1) && fscanf(fp,"%lf\",",&side2) ) ){
      //      printf("xc = %f yc = %f side1 = %f side2=%f\n", xc, yc, side1, side2);
      return error("error reading parameters");
    }//if
    message("region system FK5 is still experimental");
    if( fabs(cdelt1) < 1.0e-10 ||
	fabs(fabs(cdelt1) - fabs(cdelt2))/fabs(cdelt1) > 1.0e-4 )
      return error("pixel x & y size are unequal");

    getxy(xc, yc, &xc, &yc);
    side1 /= 3600.0*fabs(cdelt1);
    side2 /= 3600.0 * fabs(cdelt1);
    break;
  default:
    return error("unsupported region system");
  }//case
  if( fscanf(fp,"%lf,",&angle) ){
    ;// the box is rotated
  }//if 
  do{
    fscanf(fp, "%c",&ch);
  }while( ch != ')' );
  
  if( side1 <= 0.0 || side2 <=0.0 )
    return JVMessages::error("invalid box lengths!");


  angle*= M_PI/180.0;
  sina = -sin(angle);
  cosa = cos(angle);

  for(j=0; j < ny; j++ ){
    for(i=0; i< nx; i++){
      x = ((i+1.0 - xc)*cosa - (j+1.0 - yc)*sina)/side1;
      y = ((i+1.0 - xc)*sina + (j+1.0 - yc)*cosa)/side2;

      if( nReg > 0 ){ // for creating mask
	if( pixel[ i + nx*j] > IGNORE_REGION ){ 
	  // i.e. not marked by exclusion region
	  if( ( (fabs(x) <= 0.5 && fabs(y) <= 0.5) && sign != '-'))
	    pixel[ i + nx*j] = nReg;
	  else if( ( fabs(x) <= 0.5 && fabs(y) <= 0.5 && sign == '-'))
	    pixel[ i + nx*j] = EXCLUDE_REGION;
	}
      }else{ // old method
	if( ( !(fabs(x) <= 0.5 && fabs(y) <= 0.5) && sign != '-') || 
	    ( fabs(x) <= 0.5 && fabs(y) <= 0.5 && sign == '-') ){
	  pixel[ i + nx*j] = blank;
	}
      }//if
	  
    }//for i
  }//for  j

  return success();

}//selectbox()


/*****************************************************************************/
//
// FUNCTION    : medianfilter()
// DESCRIPTION : Takes the mdian value of n x n pixels. 
//               At the borders the pixels are wrapped around.
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
Status Image::medianfilter(int n)
{
  if( n % 2 == 0 || n <= 2 || n > MAX_MEDIAN || nx < 2*n || ny < 2*n ){
    warning("medianfilter(): Need an odd input parameter, not larger than ",
	    MAX_MEDIAN);
    return FAILURE;
  }
  //  setblank( 0.0 );

  float *window;
  Image newim;

  newim= *this;

  window = new float [n*n];
  if( window == 0 ){
    warning("medianfilter(): Error allocating extra memory!");
    return FAILURE;
  }// window


  int i, m,  lnx, jnx, nmedian = (n+1)/2 - 1;
  //  register
  int k, l;

  for(int j = 0; j < ny ; j++){
    jnx =j * nx;
    for(i = 0; i < nx ; i++ ){

      m = 0;
      for( l = j - nmedian ; l <= j + nmedian ; l++){
	lnx = l*nx;
	for( k = i - nmedian ; k <= i + nmedian; k++ ){

	  if( k >= 0 && k < nx && l >= 0 && l < ny)
	    window[m++] = pixel[k + lnx];

	}// for k
      }//for l
      newim.pixel[i + jnx ] = select((m+1)/2, m, window-1);
    }// for i
  }//for j


  delete [] window;
  *this = newim;

  return SUCCESS;

}// end medianfilter()


/*****************************************************************************/
//
// FUNCTION    : boxcarfilter()
// DESCRIPTION : Filters an image using a moving average.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
// filtersize               I                    
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::boxcarfilter(int filtersize, int i1, int j1, int i2, int j2,
			   bool errSm)
{
  setFunctionID("boxcarfilter");

  if( nx < 6 || ny < 6 )
    return error("the image is too small");

  if( filtersize % 2 == 0 || filtersize < 3 || filtersize > 64)
    return error("nNeeds an odd positive input parameter between 3 and 9");


  setblank( 0.0 );

  int i, j, n = filtersize/2 + 1;
  float *window, norm, sum;
  bool normalize=true;
  
  window = new float [n*n];
  if( window == 0 )
    return error("error allocating extra memory!");


  // Make one quarter of the boxcar mask
  sum = 0.0;
  norm= 1.0/(filtersize * filtersize);
  for(j = 0 ; j < n ; j++){
    for( i = 0; i < n ; i++){
      window[ j*n + i] = norm;
      sum += window[ j*n + i];
      if( i != 0 && j!=0 ){
	sum += 3.0 * window[ j*n + i];
      }
      if( (i==0 && j!=0) || (i != 0 && j==0)){
	sum += window[ j*n + i];
      }
    }// for
  }//for 

  message("Window sum = ", sum);
  
  if( errSm ){// square the value in order to quadratically add
    normalize=false;
    for(j = 0 ; j < n ; j++)
      for( i = 0; i < n ; i++)
	window[j*n + i] *= window[j*n + i];
  }

  

  if( windowfilter( window, n, normalize, i1, j1, i2, j2) == FAILURE ){
    delete [] window;
    return error("error encountered");
  }//if

  delete [] window;


  return success();

}// boxcarfilter()




/*****************************************************************************/
//
// FUNCTION    : hammingfilter()
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
// filtersize               I                    
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::hammingfilter(int filtersize, int i1, int j1, int i2, int j2,
			    bool errSm, float alpha)
{
  setFunctionID("hammingfilter");

  
  if( nx < 6 || ny < 6 )
    return error("the image is too small");
  if( alpha <= 0.0 || alpha >= 1.0)
    return error("invalid hamming parameter alpha: ",alpha);

  if( filtersize % 2 == 0 || filtersize < 5 || filtersize > 63)
    return error("Needs an odd positive input parameter between 5 and 63");


  setblank( 0.0 );

  int i, j, n = filtersize/2 + 1;
  float *window, norm, sum, val, r;
  bool normalize=true;

  window = new float [n*n];
  if( window == 0 )
    return error("error allocating extra memory!");


  // Make one quarter of the window function
  sum = 0.0;
  norm= 1.0/( (M_PI/4.0 * alpha - (1.0-alpha)/M_PI )*SQR(filtersize));
  //  message("norm = ",norm);
  message("1/norm = ",1.0/norm);
	      
  for(j = 0 ; j < n ; j++){
    for( i = 0; i < n ; i++){
      r= sqrt( float(i*i + j*j) );
      if( r <= filtersize/2.0 ){
	val=norm *(
		   alpha  + (1.0-alpha)*cos( 2.0*M_PI*r/filtersize ));
      }else
	val=0.0;
      //      printf("--> i=%2i j=%2i val=%f\n",i,j,val);
      
      if( val < 0.0)
	warning("val < 0.0 THIS SHOULD NOT HAPPEN ",val);
      
      window[ j*n + i] = val;
      
      sum += window[ j*n + i];
      if( i != 0 && j!=0 ){
	sum += 3.0 * window[ j*n + i];
      }
      if( (i==0 && j!=0) || (i != 0 && j==0)){
	sum += window[ j*n + i];
      }
    }// for
  }//for

  message("Window sum = ", sum);
  
  norm = 1.0/sum;
  for(j = 0 ; j < n ; j++){
    for( i = 0; i < n ; i++){
      window[ j*n + i] *= norm;
    }//for
  }//for
  
  if( errSm ){
    // square the value in order to quadratically add values
    // needed for error analysis, i.e. correctly smearing variances
    normalize=false;
    message("filtering will be done with square of window function...");
    for(j = 0 ; j < n ; j++)
      for( i = 0; i < n ; i++)
	window[j*n + i] *= window[j*n + i];
  }

  
  if( windowfilter( window, n, normalize, i1, j1, i2, j2) == FAILURE ){
    delete [] window;
    return error("error encountered");
  }//if

  delete [] window;


  return success();

}// hammingfilter()


/*****************************************************************************/
//
// FUNCTION    : welchfilter()
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
// filtersize               I                    
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::welchfilter(int filtersize, int i1, int j1, int i2, int j2,
			  bool errSm)
{
  setFunctionID("welchfilter");

  
  if( nx < 6 || ny < 6 )
    return error("the image is too small");


  if( filtersize % 2 == 0 || filtersize < 5 || filtersize > 63)
    return error("Needs an odd positive input parameter between 5 and 64");


  setblank( 0.0 );

  int i, j, n = filtersize/2 + 1;
  float *window, norm, sum, val, r, s=4.0/SQR(float(filtersize));
  bool normalize=true;  

  window = new float [n*n];
  if( window == 0 )
    return error("error allocating extra memory!");


  // Make one quarter of the window function
  sum = 0.0;
  norm= 8.0/M_PI/SQR(float(filtersize));
  //  message("norm = ",norm);
  message("1/norm = ",1.0/norm);
	      
  for(j = 0 ; j < n ; j++){
    for( i = 0; i < n ; i++){
      r= sqrt( float(i*i + j*j) );
      if( r <= filtersize/2.0 ){
	val=norm*(1.0 - s*r*r);
      }else
	val=0.0;
      //      printf("--> i=%2i j=%2i val=%f\n",i,j,val);
      
      if( val < 0.0)
	warning("val < 0.0 THIS SHOULD NOT HAPPEN ",val);
      
      window[ j*n + i] = val;
      
      sum += window[ j*n + i];
      if( i != 0 && j!=0 ){
	sum += 3.0 * window[ j*n + i];
      }
      if( (i==0 && j!=0) || (i != 0 && j==0)){
	sum += window[ j*n + i];
      }
    }// for
  }//for

  message("Window sum = ", sum);
  
  norm = 1.0/sum;
  for(j = 0 ; j < n ; j++){
    for( i = 0; i < n ; i++){
      window[ j*n + i] *= norm;
    }//for
  }//for
  
  if( errSm ){// square the value in order to quadratically add
    normalize=false;    
      for(j = 0 ; j < n ; j++)
	for( i = 0; i < n ; i++)
	  window[j*n + i] *= window[j*n + i];
  }

  

  if( windowfilter( window, n, normalize, i1, j1, i2, j2) == FAILURE ){
    delete [] window;
    return error("error encountered");
  }//if

  delete [] window;


  return success();

}// welchfilter()

/*****************************************************************************/
//
// FUNCTION    : gaussianfilter
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
// sigma               I                         width of the gaussian
// i1, i2,j1, j2       I                         sub image
// errSm               I                         take square of coeff. for
//                                               error analysis purpose   
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::gaussianfilter(float sigma, int i1, int j1, int i2, int j2,
			     bool errSm)
{
  setFunctionID("gaussianfilter");
  
  if( sigma > 16.0 || nx < 2 || ny < 2 ){
    message("sigma = ", sigma);
    message("nx    = ", nx);
    message("ny    = ", ny);
    return error("the image is too small or sigma too big");
  }

  setblank( 0.0 );


  int i, j, n;
  float *window, norm, sum;
  bool normalize=true;  

  //  n = (int)(4.0 * sigma + 1.0);
  n = (int)(3.5 * sigma + 1.51);
  window = new float [n*n];
  if( window == 0 )
    return error("error allocating extra memory");
  else
    message("half filter window size = ",n);

  // Make a 2D Gaussian, but only half of it!
  sum=0.0;
  norm= 1.0/(2.0 * M_PI *sigma*sigma );
  for(j = 0 ; j < n ; j++){
    for( i = 0; i < n ; i++){
      window[ j*n + i] = norm * exp( -0.5*( i*i + j*j)/(sigma*sigma) );
      sum += window[ j*n + i];
      if( i != 0 && j!=0 ){
	sum += 3.0 * window[ j*n + i];
      }
      if( (i==0 && j!=0) || (i!=0 && j==0)){
	sum += window[ j*n + i];
      }
    }// for
  }//for 

  message("Window sum = ", sum);

  
#if 0
  // Seems not neccesary as gauss is accurate within 0.1%
  norm = 1.0/sum;
  for(j = 0 ; j < n ; j++){
    for( i = 0; i < n ; i++){
      window[ j*n + i] *= norm;
    }//for
  }//for
#endif


  
  if( errSm ){// square the value in order to quadratically add
      for(j = 0 ; j < n ; j++)
	for( i = 0; i < n ; i++)
	  window[j*n + i] *= window[j*n + i];
  }


  if( windowfilter(window, n, FALSE, i1, j1, i2, j2) == FAILURE ){ // do not renormalize
    delete [] window;
    return error("error encountered");
  }//if


  delete [] window;


  return success();

}// gaussianfilter

/*****************************************************************************/
//
// FUNCTION    : gaussfilter
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
// sigma               I                         width of the gaussian
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::gaussianfilter(const Image &im,
			     float sigma, int i1, int j1, int i2, int j2)
{

  setFunctionID("gaussianfilter");
  if( sigma > 16.0 || im.nx < 2 || im.ny < 2 )
    return error("the image is too small or sigma too big");

  setblank( 0.0 );

  int i, j, n;
  float *window, norm, sum;

  //  n = (int)(4.0 * sigma + 1.0);
  n = (int)(3.5 * sigma + 1.51);
  window = new float [n*n];
  if( window == 0 )
    return error("error allocating extra memory");


  // Make a 2D Gaussian, but only half of it!
  sum=0.0;
  norm= 1.0/(2.0 * M_PI *sigma*sigma );
  for(j = 0 ; j < n ; j++){
    for( i = 0; i < n ; i++){
      window[ j*n + i] = norm * exp( -0.5*( i*i + j*j)/(sigma*sigma) );
      sum += window[ j*n + i];
      if( i != 0 && j!=0 ){
	sum += 3.0 * window[ j*n + i];
      }
      if( (i==0 && j!=0) || (i!=0 && j==0)){
	sum += window[ j*n + i];
      }
    }// for
  }//for 

  if( windowfilter( im, window, n, FALSE, i1, j1, i2, j2) == FAILURE ){
    delete [] window;
    return error("error encountered");
  }//if

  delete [] window;

  return success();

}// gaussianfilter


/*****************************************************************************/
//
// FUNCTION    : windowfilter()
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   window             I               subimage containing smearing function
//   n                  I
//   i1,j1,i2,j2        I                defines window, negative numbers
//                                       indicate the whole image
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::windowfilter(float *window, int n, bool renormalize,
			   int i1, int j1, int i2, int j2)
{
  Image newim( *this);



  if( newim.windowfilter( *this, window, n, renormalize, 
			  i1, j1, i2, j2) == FAILURE ){
    return FAILURE;
  }
  
  *this = newim;



  return SUCCESS;

}// windowfilter()


/*****************************************************************************/
//
// FUNCTION    : windowfilter()
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   im                 I               image to be smeared
//   window             I               subimage containing smearing function
//   n                  I
//   i1,j1,i2,j2        I                defines window, negative numbers
//                                       indicate the whole image
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::windowfilter(const Image &im, float *window, 
				int n, bool renormalize,
				int i1, int j1, int i2, int j2)
{

  setFunctionID("windowfilter");


  if( nx != im.nx  || ny != im.ny )
    *this = im;
  else
    copy_keywords( im);


  if( i1 == -1 && j1 == -1 && i2 == -1 && j2 == -1 ){
    i1 = j1 = 0;
    i2 = im.nx-1;
    j2 = im.ny-1;
    //    printf("%i %i , %i %i\n", i1, j1, i2, j2);
  }//
  else{
    i1 -= 1; // FITS 2 C convention
    j1 -= 1;
    i2 -= 1;
    j2 -= 1;
  }//if

  if( n < 2 || i2 <= i1 || j2 <= j1 || i1 < 0 || j1 < 0 || 
      i2 >= im.nx || j2 >= im.ny || nx <= 2*n+1 || ny < 2*n+1){
    return error("invalid parameters");
  }


  setblank( 0.0 );



  //  float norm, norm0;
  float norm0;
  *this = im;

  norm0 = 0.0;
  //  register
  int k, l;
  // better to leave normalization not in this function
  if( renormalize == TRUE ){
    warning("window filter will be normalized to 1");
    long ln;
    for( l= 0 ; l < n ; l++){
      ln = l*n;
      for( k= 0 ; k < n ; k++){
	norm0 += window[ ln + k];
	if( k != 0 && l !=0 )
	  norm0 += 3.0 * window[ ln + k];
	if( (k==0 && l!=0) || (k!=0 && l==0))
	  norm0 += window[ ln + k];
	
      }//for
    }//for
    if( norm0 == 0.0 )
      return error("total sum == 0.0");

    message("renormalizing");
    for(int i=0;i<n*n;i++){
      window[i] /= norm0;
    }
  }else{
    warning("window filter NOT normalized to 1");    
  }//if renormalize


  //  float winval, pixval, val, corr=1.;0
  float pixval, val, corr=1.0;  
  long ipk, imk, jpl, jml, nl, jnx ,jplnx, jmlnx;
  int i, j;
  for(j = j1; j <= j2 ; j++){
    jnx = j*nx;
    for(i = i1; i <= i2 ; i++ ){

      //the following seems nonsense
#if 0 
      if( renormalize==TRUE && 
	  (i-n <= i1 || j-n <= j1 || i+n >= i2 || j+n >= j2) ){

	norm = 0.0;
	for( l= 0 ; l < n ; l++){
	  nl = n*l;
	  jpl = j+l; // for speed reasons: calculate only once
	  jml = j-l;
	  for( k= 0 ; k < n ; k++){

	    winval= window[k + nl];
	    // first determine normalization:
	    if( i+k <= i2 ){
	      if( jpl <= j2)         // first quadrant
		norm += winval;
	      if( l != 0 && jml >= j1 ) // fourth quadrant
		norm += winval;
	    }
	    if( k != 0 && i-k >= i1){
	      if( jpl <= j2)             // second quadrant
		norm += winval;

	      if( l != 0 && jml >= j1 )  // third quadrant
		norm += winval;
	    }// if 

	  }//for
	}//for
	if( norm > 0.0 )
	  corr = norm0/norm;
	else
	  corr = 1.0;
      }//if renormalize etc
#endif

      pixval = 0.0;

      // Now make the convolution
      for( l= 0 ; l < n ; l++){
	nl = n*l;
	jpl = j+l; // for speed reasons: calculate only once
	jml = j-l;
	jplnx = jpl*nx;
	jmlnx = jml*nx;
	for( k= 0 ; k < n ; k++){

	  ipk = i+k; // for speed reasons: calculate only once
	  imk = i-k;
	  val = window[ k + nl]*corr;
	  if( ipk <= i2 ){
	    if( jpl <= j2)         // first quadrant
	      pixval += im.pixel[ ipk + jplnx] * val;
	    if( jml >= j1 && l != 0 ) // fourth quadrant
	      pixval += im.pixel[ ipk + jmlnx] * val;	    
	  }

	  if( imk >= i1 && k != 0 ){
	    if( jpl <= j2)  // second quadrant
	      pixval += im.pixel[ imk + jplnx] * val;
	    if( jml >= j1 && l != 0) // third quadrant
	      pixval += im.pixel[ imk + jmlnx] * val;
	  }// if 
	}//for
      }//for

      pixel[i + jnx] = pixval;

    }// for i
  }//for j



  return success();

}// windowfilter()



/*****************************************************************************/
//
// FUNCTION    : shift_origin( void )
// DESCRIPTION : Shift the center of the image to (0,0).
//               This is useful if an actual point source is used
//               as a PSF model to be used with the FFT.
//               The values < nx/2,ny/2 are folded around, the origin so
//               x=0 will be nx/2+1
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
Status Image::shift_origin( void )
{

  setFunctionID("shift_origin");
  if( nx <= 1 || ny <= 1 || nx != ny || nx % 2 == 1 )
    return error("image has the wrong size or format");


  int i,j;
  float value00, value10, value01, value11;

  for( j=0; j < ny/2 ; j++){
    for(i=0; i < nx/2 ; i++){

      value00 = pixel[ i + nx*j];
      if( i + nx/2 < nx )
	value01 = pixel[ i + nx/2 + j*nx];
      if( j+ny/2 < ny){
	value10 = pixel[ i +  (j+ny/2)*nx];
	if( i + nx/2 < nx )
	  value11 = pixel[ i + nx/2 + (j+ny/2)*nx ];
      }

      pixel[i + j*nx] = value11;
      if( i + nx/2 < nx)
	pixel[i+nx/2 + j*nx] = value10;
      if( j + ny/2 < ny ){
	pixel[i+nx/2 + (j+ny/2)*nx] = value00;
	if( i + nx/2 < nx)
	  pixel[i + (j+ny/2)*nx] = value01;
      }//if

    }//for i
  }//for

  reset_detector_keywords();

  return success();
}// shift_origin()





/*****************************************************************************/
//
// FUNCTION    : edgedetect()
// DESCRIPTION : Uses Sobel's method for detecting edges
// 
//  Masks: Sr -1 -2 -1  Sc:  -1  0  1
//             0  0  0       -2  0  2
//             1  2  1       -1  0  1
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
Status Image::edgedetect()
{
  setFunctionID("edgedetect");
  if( nx <= 3 || ny<=3 )
    return error("image too small");

  setblank( 0.0 );

  int i, j;
  float sr, sc, grad;
  Image newim( *this );
  // Compute the convolution
  for( j = 1 ; j < ny-1 ; j++ ){
    for( i = 1 ; i < nx -1 ;i++ ){
      sr = pixel[(i-1)+(j+1)*nx] + 2*pixel[i+(j+1)*nx] + pixel[(i+1)+(j+1)*nx];
      sr-= pixel[(i-1)+(j-1)*nx] + 2*pixel[i+(j-1)*nx] + pixel[(i+1)+(j-1)*nx];
      sc = pixel[(i+1)+(j-1)*nx] + 2*pixel[(i+1)+j*nx] + pixel[(i+1)+(j+1)*nx];
      sc-= pixel[(i-1)+(j-1)*nx] + 2*pixel[(i-1)+j*nx] + pixel[(i-1)+(j+1)*nx];
      grad=fabs(sr) + fabs(sc);
      newim.pixel[ i + j*nx] =  grad;
    }//for i
  }//for j

  *this = newim;

  return success();
}//end edgedetect()



/****************************************************************************/
//
// FUNCTION    : applyMask
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::applyMask( const Image &mask)
{
  setFunctionID("applyMask");

  if( strncasecmp("RA---TAN",ctype1,8)!=0 ||
      strncasecmp("DEC--TAN",ctype2,8)!=0 || 
      strncasecmp(ctype1,mask.ctype1,8)!=0 ){
    message("ctype1=",ctype1);
    message("ctype2=",ctype2);
    return error("this function expects a RA DEC coordinate system");
  }
  int i2, j2;
  double ra, dec, x, y;
  for( int i=0; i < nx; i++){
    for( int j=0; j < ny; j++){


      getradec( (float)(i+1),(float)(j+1), &ra, &dec);
      mask.getxy(ra, dec, &x, &y);
      i2 = int(x+0.5);
      j2 = int(y+0.5);

      if( mask.get(i2, j2)  < 1.0 ){
	pixel[i + j*nx]= 0.0;
      }
    }

  }

  return success();
}//applyMask

/*****************************************************************************/
//
// FUNCTION    : correlation
// DESCRIPTION : 
//               
//               
//               
//
// SIDE EFFECTS: UNDER CONSTRUCTION
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   model              I                 the model image
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
double Image::correlation(const Image &model) const
{

  setFunctionID("correlation");

  if( nx <= 0 || ny <=0 || nx != model.nx || ny != model.ny){
    error("input images do not match in size");
    return 0.0;
  }

  //  long int i, nxy;
  double corr = 0.0, norm1 = 0.0, norm2=0.0;
  Image temp = model;
  

  temp.setChatty(chatty);
  temp *= *this;
  corr = temp.total();

  temp = model;
  temp *= model;
  norm1 = sqrt(temp.total());

  temp = *this;
  temp *= *this;
  norm2 = sqrt(temp.total());
  if( norm1 == 0.0 || norm2 == 0.0 ){
    error("division by zero");
    return 0.0;
  }
  corr /= norm1 * norm2;

  success();
  return corr;
}//correlation()

/*****************************************************************************/
//
// FUNCTION    : likelihood()
// DESCRIPTION : This routines compares the current image with a model image
//               using the maximum likelihood method for a poissonian
//               distribution (Cash, 1979, ApJ 228, 939).
//               The lower the return value the better the model.
//
// SIDE EFFECTS: UNDER CONSTRUCTION
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   model              I                 the model image
//   nmodel             I                 # pixels > 0 (only those are used)
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
double Image::likelihood(const Image &model, long *nmodel) const
{

  setFunctionID("likelihood");

  if( nx <= 0 || ny <=0 || nx != model.nx || ny != model.ny){
    error("input images do not match in size");
    return 0.0;
  }

  long int i, nxy;
  double likelihood=0.0; // actually log(likelihood) see Cash, 1979
  float *modelptr= model.pixel;
  float *imptr = pixel;

  message("test");
  
  *nmodel= 0;
  nxy = nx*ny;
  for( i=0 ; i < nxy; i++){
    if( *modelptr > 0.0 ){
      //      if( *imptr != 0.0 )
	likelihood += (*imptr * log( *modelptr ) - *modelptr);
	//      else
	//	likelihood -= *modelptr;
      *nmodel += 1;
    }
    modelptr++;
    imptr++;
  }//for

  message("test");
  success();
  return -2.0 * likelihood;
}//likelihood()


/*****************************************************************************/
//
// FUNCTION    : likelihood()
// DESCRIPTION : This routines compares the current image with a model image
//               using the maximum likelihood method for a poissonian
//               distribution (Cash, 1979, ApJ 228, 939).
//               The lower the return value the better the model.
//
// SIDE EFFECTS: UNDER CONSTRUCTION
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   model              I                 the model image
//   nmodel             I                 # pixels > 0 (only those are used)
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
double Image::likelihood(const Image &model, long *nmodel,
			      int i1, int j1, int i2, int j2) const
{

  setFunctionID("likelihood");

  *nmodel= 0;

  if( nx <= 0 || ny <=0 || nx != model.nx || ny != model.ny || 
      i2 <= i1 || j2 <= j1 || i1 <= 0 || j1 <= 0 || i2 > nx ||  j2 > ny){
    error("input images do not match in size or wrong window");
    return 0.0;
  }

  double modelval;
  double likelihood=0.0; // actually log(likelihood) see Cash, 1979

  i1 -= 1;
  j1 -= 1;
  i2 -= 1;
  j2 -= 1;

  int i, k;
  for( int j=j1 ; j <= j2; j++){
    for( i = i1 ; i <= i2 ; i++){
      k= i + j*nx;
      modelval = model.pixel[k];
      if( modelval > 0.0 ){
	likelihood += (pixel[k] * log( modelval ) - modelval);
	*nmodel += 1;
      }

    }//for i
  }//for j

  success();
  return -2.0 * likelihood;

}//likelihood()


/*****************************************************************************/
//
// FUNCTION    : likelihood()
// DESCRIPTION : This routines compares the current image with a model image
//               using the maximum likelihood method for a poissonian
//               distribution (Cash, 1979, ApJ 228, 939).
//               The lower the return value the better the model.
//
// SIDE EFFECTS: UNDER CONSTRUCTION
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   model              I                 the model image
//   nmodel             I                 # pixels > 0 (only those are used)
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
double Image::likelihood_image(const Image &model, 
				    const Image &image, long *nmodel)
{

  setFunctionID("likelihood_image");

  if( image.nx <= 0 || image.ny <=0 || 
      image.nx != model.nx || image.ny != model.ny){
    error("input images do not match in size");
    return 0.0;
  }

  if( nx != model.nx || ny != model.ny )
    if( allocate(model.nx, model.ny) == FAILURE )
      return 0.0;

  long int i, nxy;
  double likelihood =0.0; // actually log(likelihood) see Cash, 1979
  float *modelptr   = model.pixel;
  float *imptr      = image.pixel;
  float *thisptr    = pixel;
  *nmodel= 0;

  nxy = nx*ny;
  for( i=0 ; i < nxy; i++){
    if( *modelptr > 0.0 ){
      *thisptr    = -2.0*(*imptr * log( *modelptr ) - *modelptr);
      likelihood += *thisptr;
      *nmodel += 1;
    }
    else
      *thisptr    = 0.0;
    modelptr++;
    imptr++;
    thisptr++;
  }//for

  success();
  return likelihood;
}//likelihood_image()


#if 0
/*****************************************************************************/
//
// FUNCTION    : poisson()
// DESCRIPTION : Generates an image with a poissonian distribution 
//               of counts/pixel around a given mean
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
//      NAME            I/O                     DESCRIPTION
//   mean               I                     average counts/pixel
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::poisson(float mean)
{
  setFunctionID("poisson");
  // from Num. Rec.
  float poidev(float xm, long *idum);

  if( nx <=0 || ny <= 0 )
    return error("there is no valid image present!");


  int i, j;
  long idum= (long) (100 * mean);
  float val;

  for( j=0 ; j < ny ; j++){
    for( i=0 ; i < nx ; i++){
      val = poidev( mean, &idum );
      pixel[i + j*nx] = val;
    }//for i
  }

  reset_detector_keywords();

  return success();
}// poisson()



/*****************************************************************************/
//
// FUNCTION    : poisson()
// DESCRIPTION : Generates an image with poissonian fluctuations using
//               an input image as a model.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
//      NAME            I/O                     DESCRIPTION
//   im                I            the model image from which events are drawn
//   factor            I            a scaling for the count rate
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status Image::poisson(const Image &im, float factor, long idum)
{

  setFunctionID("poisson");
  // from Num. Rec.
  float poidev(float xm, long *idum);


  if( im.nx <=0 || im.ny <= 0 || factor == 0.0)
    return error("the model image is not a valid image or factor = 0");


  long i, nxy;
  float val;
  *this = im;

  srand((int)idum);

  nxy = nx*ny;
  for( i=0 ; i < nxy ; i++){

      val = factor * im.pixel[i];
      pixel[i] = poidev( val, &idum );
      //      if( i%10 == 0){
      //	idum = (long)random();
	//      }

  }//for i

  //  reset_detector_keywords();
  copyKeywords( im );

  return success();
}// poisson()
#endif

/*****************************************************************************/
//
// FUNCTION    : fraction( float fraction )
// DESCRIPTION : Determines the level0 at which 
//                #pixel( level >= level0) / #total pixels > fraction
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :
//
/*****************************************************************************/
float Image::fraction( float fraction0, int i1, int j1, int i2, int j2 )
{
  float min=minimum(), max=maximum();
  setFunctionID("fraction");
  if( nx <= 0 || ny <= 0 || min == max || i1 >= i2 || j1 >= j2 ||
      i1 <= 0 || j1 <= 0 || i2 > nx || j2 > ny || 
      fraction0 > 1.0 || fraction0 <= 0.0 ){
    error("empty image or wrong window coordinates!");
    return 0.0;
  }//if

  int i,j;
  long npix = 0;
  float nlevels = MIN( (float)(i2-i1)*(j2-j1), MAX(1000.0, 2.0/fraction0) );
  float level=max, step = (max - min)/nlevels;
  double frac;

  i1 -= 1;
  i2 -= 1;
  j1 -= 1;
  j2 -= 1;
  message("nlevels = ",nlevels);
  
  do{
    npix = 0;
    frac = 0.0;
    level -= step;

    for( j=j1 ; j <= j2 ; j++ ){
      for( i = i1 ; i <= i2 ; i++){ 
	if( pixel[i + j*nx] >= level ){
	  frac += 1.0;
	}
	npix++;
      }//for i
    }//
    frac /= npix;
    //    printf("level = %f , min = %f, max = %f, step=%e, frac=%e\n",level, min, max, step,frac);
  }while( frac < fraction0 && level > min);
  success();
  return level;
}// fraction()


#define NHISTO  512
/****************************************************************************/
//
// FUNCTION    : writeHisto()
// DESCRIPTION : 
//
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
/****************************************************************************/
Status Image::writeHistogram( char filename[], 
				  int i1, int j1, int i2, int j2)
{
  FILE *fp;
  long i, k, nxy, nmedian, sum, nzeros, npix=0;
  long histogram[NHISTO];
  float binwidth, max, min, val, mean, median;

  setblank( 0.0 );
  min = minimum(i1, j1, i2, j2);
  max = maximum(i1, j1, i2, j2);

  if( i1 == -1 && j1 == -1 && i2 == -1 && j2 == -1 ){
    i1 = j1 = 1;
    i2 = nx;
    j2 = ny; 
  }
  if( i1 >= i2 || j1 >= j2 || i1 < 1 || j1 < 1 || i2 > nx || j2 > ny ){
    warning("writeHistogram(): Error in input parameters.");
    return FAILURE;
  }//if
  if( nx <= 1 || ny <= 1 ){
    warning("writeHistogram(): cannot make a histogram.");
    return FAILURE;
  }//if
  i1 -= 1;
  i2 -= 1;
  j1 -= 1;
  j2 -= 1;

  printf("min = %f max=%f\n",min,max);

  binwidth= (max - min + 1.0)/NHISTO;

  // initialize
  for( i=0; i < NHISTO; i++){
    histogram[i] = 0;
  }

  //
  // Calculate the histogram and calculate the average
  //
  mean = 0.0;
  nxy = nx*ny;
  for( int j = j1; j <= j2; j++){
    for( i = i1; i <= i2; i++){
    // Note that the centering of the first bin is at min-0.5*binwidth
      // 0.5 should be added for rounding off
      k = (int)( (pixel[i + j*nx] - min)/binwidth ); 
      mean += pixel[i + j*nx];
      npix++;
      if( k < 0 || k >= NHISTO ){
	warning("writeHistogram(): This error should never occur!");
	return FAILURE;
      }

      histogram[k]++;
    }//for
  }//for
  mean/= npix;

  //
  // Calculate the median
  //
  nmedian = npix/2; 

  sum = 0;
  i = 0;
  while( i < NHISTO && sum < nmedian ){
    sum += histogram[i++];
  }//while
  median = min + (i-1) * binwidth;
  //
  // Calculate number of zero's in histogram
  //
  nzeros=0;
  for( i=0 ; i < NHISTO ; i++){
    if( histogram[i] == 0 )
      nzeros++;
  }//for

  //
  // write the histogram to a file
  //
  if( ( fp= fopen(filename,"w") ) ){
//    fprintf(fp,"%li\n",NHISTO-nzeros);
    fprintf(fp,"%i\n",NHISTO);
    fprintf(fp,"min = %.2e max = %.2e total = %.2e\n", min, max,total());
    fprintf(fp,"mean = %.2e median = %.2e npix = %li\n", mean, median,npix);
    fprintf(fp,"        value      histogram\n");
    for( i = 0; i < NHISTO ; i++){

      if( 1 || histogram[i] != 0 ){
	val = min + i * binwidth;
	fprintf(fp,"%4li   %.2e  %7ld\n", i,val , histogram[i]);
      }//if

    }//for
    fclose(fp);
  }
  else{
    warning("writeHistogram(): Error opening file!");
    return FAILURE;
  }//else

  return SUCCESS;  
}// writeHistogram()



/*****************************************************************************/
//
// FUNCTION    : startMark8()
// DESCRIPTION : Starts up marking regions.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : the number of objects found
//
/*****************************************************************************/
int Image::startMark8(float low, float up)
{

  setFunctionID("startMark8");
  if( nx <= 0 || ny <= 0 ){
    error("no image present");
    return 0;
  }//if

  message("identifying connected pixels and tagging them");


  float value = 1.0, level= -1.0;
  bilevel(low, up, level ); // set all pixels to be marked to a negative value


  for(int j = 0; j < ny ; j++)
    for(int i = 0; i < nx ; i++){
      if( fabs(pixel[i + j*nx] - level) < 0.1 ){
	value+= 1.0;
      	mark8( /*level,*/ value, i, j );
      }//if
    }//for

  message("number of regions found:",  (long int)(value-0.5));

  return (int)(value-0.5);
}//startMark8()


#if 0
/*****************************************************************************/
//
// FUNCTION    : mark8()
// DESCRIPTION : Marks all pixel unequal to 0.0. Adapted from Parker, 1994
//               Assumes that level < -0.5. 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
//      NAME            I/O                     DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE;
//
/*****************************************************************************/
#define  MAX_RECURSION 131072
//#define  MAX_RECURSION 193120
//#define  MAX_RECURSION 262144
//#define  MAX_RECURSION 262144
Status Image::mark8( /*float level,*/ float value, int iseed, int jseed )
{

  static float lastval=value;
  static long int nrec=0;
  if( lastval == value)
    nrec++;
  else{
    lastval = value;
    nrec=0;
  }

  if( nx <= 0 || ny <= 0 || value==0.0 || //value== level ||
      iseed < 0 || iseed >= nx || jseed < 0 || jseed >= ny // || level > 0.0 
      ){
    return FAILURE;
  }

  if( nrec > MAX_RECURSION ){
    //    nrec=0;
    setFunctionID("mark8");
    return error("preventing running out of stack memory");
  }//if

  if( pixel[iseed + jseed*nx] > 0.0 )
    return FAILURE;


  pixel[iseed + jseed*nx] = value;
  for( int j = jseed-1; j <= jseed+1 ; j++)
    if( j >= 0 && j < ny  )
      for( int i = iseed-1; i <= iseed+1 ; i++){
	//      if( i >= 0 && j >= 0 && i< nx && j < ny  )
	//        if( pixel[i + j*nx] < -0.5 ){
	if( i >= 0 && i < nx && pixel[i + j*nx] < -0.5 ){
	  //          mark8(level, value, i, j );
          mark8( value, i, j );
        }//if
    }//for


  return SUCCESS;
}//mark8()
#undef  MAX_RECURSION 
#endif

#if 1
/*****************************************************************************/
//
// FUNCTION    : mark8()
// DESCRIPTION : Marks all pixel unequal to 0.0. Adapted from Parker, 1994
//               Assumes that level < -0.5. 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
//      NAME            I/O                     DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE;
//
/*****************************************************************************/
Status Image::mark8(float value, int iseed, int jseed )
{

  if( nx <= 0 || ny <= 0 || iseed < 0 || jseed < 0 || iseed > nx || jseed > ny
      || value <= 0.0){
    setFunctionID("mark8");
    return error("no image data or invalid starting points");
  }
  if( pixel[iseed + jseed * nx] >= 0.0 )
    return FAILURE;

  pixel[iseed + jseed * nx] = value;
  bool again;
  do{
    again = false;
    for(int i = 0; i < nx; i++)
      for(int j = 0; j < ny; j++){
	if( pixel[i+ j*nx] == value ){
	  for( int n=i-1; n <=i+1; n++)
	    for( int m=j-1; m <=j+1; m++){
	      if( !(n >= 0 && n < nx && m >= 0 && m < ny) ) continue;
	      if( pixel[n + m*nx] < 0.0 ){
		pixel[n + m*nx] = value;
		again = true;
	      }//if
	    }//for
	}//if
      }//for

    for( int i = nx-1; i >= 0; i--)
      for( int j = ny-1 ; j >=0 ; j--){
	if( pixel[i + j*nx] == value){
	  for( int n=i-1; n<= i+1; n++)
	    for( int m=j-1; m <= j+1; m++){
	      if( !(n >= 0 && n < nx && m >= 0 && m < ny) ) continue;
	      if( pixel[n + m*nx] < 0.0 ){
		pixel[n + m*nx] = value;
		again = true;
	      }//if
	    }//for
	}
      }//for
    
  }while(again);//do

  return success();
}//
#endif

/*****************************************************************************/
//
// FUNCTION    : startMark4()
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : the number of objects found
//
/*****************************************************************************/
int Image::startMark4(float low, float up)
{
  if( nx <= 0 || ny <= 0 ){
    warning("startMark4(): No image present!");
    return 0;
  }

  int i, j;
  float value = 1.0, level= -1.0;

  bilevel(low, up, level*1.1 );

  for(j = 0; j < ny ; j++)
    for(i = 0; i < nx ; i++){
      if( pixel[i + j*nx] <= level ){
	mark4( level, value, i+1, j+1 );
	value += 1.0;
      }//if
    }//for

  return (int)(value-0.5);
}//startMark4()

/*****************************************************************************/
//
// FUNCTION    : mark4()
// DESCRIPTION : Marks all pixel unequal to 0.0. Adapted from Parker, 1994
//               Assumes that level <= 0.0. 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE;
//
/*****************************************************************************/
Status Image::mark4( float level, float value, int iseed, int jseed )
{


  if( nx <= 0 || ny <= 0 || value == 0.0 || value == level ||
      iseed <= 0 || iseed > nx || jseed <= 0 || jseed >= ny || level > 0.0 ){
    //    warning("mark4(): Error in the input values!");
    return FAILURE;
  }

  iseed -= 1;
  jseed -= 1;
  if( pixel[iseed + jseed*nx] > level ){
    return FAILURE;
  }

  pixel[iseed + jseed*nx] = value;

  int i, j;
  for( j = jseed-1; j <= jseed+1 ; j++)
    for( i = iseed-1; i <= iseed+1 ; i++){

      if( ((j != jseed && i == iseed) ||  (i != iseed && j == jseed)) &&
	  i >= 0 && j >= 0 && i < nx && j < ny  )
	if( pixel[i + j*nx] <= level ){
	  mark4(level, value, i+1, j+1 );
	}//if

    }//for

  return SUCCESS;
}//mark4()



/****************************************************************************/
//
// FUNCTION    : erode
// DESCRIPTION : strip outer (valid) pixels from regions. After J.R.Parker.
// 
//
// REMARKS     : val should be close to an integer, so round off errors.
//               
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::erode(float val)
{
  if( nx <= 0 || ny <= 0 ){
    setFunctionID("erode");
    return error("no image present");
  }
  Image temp(*this);

  /* Mark the boundary pixels */
  for(int j = 0; j < temp.ny; j++)
    for(int i = 0; i < temp.nx; i++){
      if( fabs(temp.pixel[i + j * temp.nx] - val) < 0.1 )
	if( temp.nBoundaryPix8(i, j, val ) != 8 )
	  pixel[i + j*nx] = 0.0;
    }//for

  return SUCCESS;
}//erode()


/****************************************************************************/
//
// FUNCTION    : dilate()
// DESCRIPTION : Adds an outer layer to a region, by marking pixels that
//               have a neighbour with the right value.
//               Inspired by J.R. Parker, but slightly modified
//               (simpler but slower, as all neighbours are investigated
//                whereas in J.R. Parker one neighbour is enough).
//
// REMARKS     : val should be close to an integer, so round off errors.
//               
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::dilate(float val)
{
  if( nx <= 0 || ny <= 0 ){
    setFunctionID("dilate");
    return error("no image present");
  }
  Image temp( *this);

  /* Mark the boundary pixels */

  for(int j = 0; j < temp.ny; j++)
    for(int i = 0; i < temp.nx; i++){
      if( fabs(temp.pixel[i + j * temp.nx] - val) > 0.5 )
	if( temp.hasNeighbour(i, j, val ) == SUCCESS )
	  pixel[i + j*nx] = val;
    }

  return SUCCESS;
}//dilate()

/****************************************************************************/
//
// FUNCTION    : nBoundaryPix8( )
// DESCRIPTION : Counts the number of boundary pixels with value=val to 
//               pixel i,j.
//               Uses all 8 boundary pixels
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
// i,j                   I                       pix coordinates
// val                   I                       pixel values
//
//  RETURNS    : number of boundary pixels
//
/****************************************************************************/
int Image::nBoundaryPix8( int i, int j, float val)
{
  if( nx <= 0 || ny <= 0 || i < 1 || i >= nx-1 || j < 1 || j >= ny-1 ){
    //    error("nBoundaryPix8(): No image or i,j on image boundary!");
    return 0;
  }


  if( fabs(pixel[i + j*nx] -val) > 0.1 )
    return 0;

  int nboundpix=0;
  for( int k = -1; k <= 1; k++)
    for( int l = -1; l <= 1; l++)
      if( fabs(pixel[(i+k) + (j+l)*nx] -val) < 0.1 )
	nboundpix++;


  return nboundpix-1; 
}//nBoundaryPix8()


/****************************************************************************/
//
// FUNCTION    : hasNeighbour( )
// DESCRIPTION : Looks for a neigbour to pixel i,j with value==val 
// 
//
// REMARKS     : 
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::hasNeighbour( int i, int j, float val)
{
  if( nx <= 0 || ny <= 0 || i < 1 || i >= nx-1 || j < 1 || j >= ny-1 ){
    //    error("hasNeighbour(): No image or i,j on image boundary!");
    return FAILURE;
  }
  
  for( int k = -1; k <= 1 ; k++)
    for( int l = -1; l <= 1; l++)
      if( !(k==0 && l==0) && (i+k)>=0 && (i+k) < nx &&
	  (j+l) >= 0 &&	(j+l) < ny &&
	  fabs(pixel[(i+k) + (j+l)*nx] -val) < 0.1 )
	return SUCCESS;

  
  return FAILURE;
}//hasNeighbour( 

/****************************************************************************/
//
// FUNCTION    : nBoundaryPix8( )
// DESCRIPTION : Counts the number of boundary pixels with value=val to 
//               pixel i,j.
//               Uses all 8 boundary pixels
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
// i,j                   I                       pix coordinates
// val                   I                       pixel values
//
//  RETURNS    : number of boundary pixels
//
/****************************************************************************/
int Image::nBoundaryPix4( int i, int j, float val)
{
  if( nx <= 0 || ny <= 0 || i < 1 || i >= nx-1 || j < 1 || j >= nx-1 ){
    setFunctionID("nBoundaryPix4");
    error("nBoundaryPix8(): No image or i,j on image boundary!");
    return 0;
  }

  int nboundpix = 0;
  if( fabs(pixel[i + j*nx] -val) < 0.1 ){
    int k, l;
    k = 0;
    l = 1;
    if( fabs(pixel[(i+k) + (j+l)*nx] -val) < 0.1 )
      nboundpix++;
    l = -1;
    if( fabs(pixel[(i+k) + (j+l)*nx] -val) < 0.1 )
      nboundpix++;
    k = 1;
    l = 0;
    if( fabs(pixel[(i+k) + (j+l)*nx] -val) < 0.1 )
      nboundpix++;
    k = -1;
    if( fabs(pixel[(i+k) + (j+l)*nx] -val) < 0.1 )
      nboundpix++;
  }//if
   
  return nboundpix;
}//nBoundaryPix4()

#if 1
/****************************************************************************/
//
// FUNCTION    : addgaussian()
// DESCRIPTION : adds a gaussian with a total integration of value "norm" 
//               and specified width to the image
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//    x,y               I                      pixel position
//    norm              I                      total integration
//   sigma_x,sigma_y    I                      x/y width
//   rot                I                      rotation (in degrees)
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::addgaussian(float x, float y, float norm,
			       float sigma_x, float sigma_y, float rot)
{
  if( sigma_x <= 0.0 || sigma_y <= 0.0 || x <= 0.0 || y <= 0 || 
      x >= nx || y >= ny){
    setFunctionID("addgaussian");
    //    printf("sigmax=%f sigmay=%f x=%f y=%f\n",sigma_x, sigma_y, x, y);
    return error("error in sigma_x, sigma_y input values");
  }//if

  float radius = 5.0 * MAX(sigma_x, sigma_y);
  int i1 = (int)(x - radius -0.5)-1,  j1=(int)(y - radius -0.5)-1;
  int i2 = (int)(x + radius + 0.5)-1, j2=(int)(y + radius + 0.5)-1;

  i1 = MAX(i1, 0);
  j1 = MAX(j1, 0);
  i2 = MIN(i2, nx-1);
  j2 = MIN(j2, nx-1);

  x -= 1.0; // FITS 2 C++ convention for pixels
  y -= 1.0; // FITS 2 C++ convention for pixels
  norm /= 2.0 * M_PI * sigma_x * sigma_y; // norm of gaussian
  norm *= 0.5*M_PI*sigma_x*sigma_y; // correction factor for error function
  rot *= M_PI/180.0; //convert to radians


  float x2, y2, x3, y3, sinrot=sin(rot), cosrot=cos(rot), pixval;
  float halfpx = 0.5*sqrt(0.5)/sigma_x;
  float halfpy = 0.5*sqrt(0.5)/sigma_y;

  int i;
  for( int j = j1 ; j <= j2 ; j++ )
    for( i = i1 ; i <= i2 ; i++ ){

      x2 = sqrt(0.5)*(i-x)/sigma_x;
      y2 = sqrt(0.5)*(j-y)/sigma_y;
      if( rot != 0.0 && sigma_x != sigma_y ){
	x3 = x2*cosrot - y2*sinrot;
	y3 = x2*sinrot + y2*cosrot;
	x2 = x3;
	y2 = y3;
      }
      pixval = norm*(erff(x2+halfpx) - erff(x2-halfpx))*
	(erff(y2+halfpy) - erff(y2-halfpy));        // not quite correct
      pixel[i + j*nx] += pixval;

    }//for

  return SUCCESS;
}// addgaussian()


/****************************************************************************/
//
// FUNCTION    : findbadpixels()
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::findBadPixels( const Image &detmap, int ccdid , 
			     float threshold )
{

  setFunctionID("findBadPixels");
#if 0
   char name[100];
   sprintf(name,"%s%i.img", "ccd_",ccdid);
   writefits(name);
#endif
  if( detmap.nx < 1 || detmap.ny <= 1 || 
      ccdid < 1 || threshold <= 0.0 || nbadpixbuffer==0 )
    return error("invalid object or input parameters");

  if( exposure <= 0.0 ){
    warning("exposure not defined, set to 10 ks");
    threshold *= exposure;
  }else
    threshold *= exposure;

  message("number threshold = ", threshold);
  //  message("nx=",nx);
  //  message("ny=",ny);
  message("buffersize=",nbadpixbuffer);

  int npixx, npixy;
  float xbgd, ybgd;
  for( int j = 0; j< detmap.ny; j++)
    for( int i = 0; i < detmap.nx; i++){

      // local background estimate
      xbgd  = ybgd = 0.0;
      npixx = npixy = 0;
      for( int l = -1; l <= 1 ; l++)
	for( int k = -1; k <= 1; k++){
	  if( i+k >= 0 && i+k < detmap.nx && j+l >=0 && j+l < detmap.ny){
	    if( k != 0 ){
	      xbgd += detmap.pixel[i+k + (j+l) * detmap.nx];
	      npixx++;
	    }
	    if( l != 0 ){
	      ybgd += detmap.pixel[i+k + (j+l) * detmap.nx];
	      npixy++;
	    }
	  }//if
	}//for
      if( npixx > 0 )
	xbgd/= npixx;
      if( npixx > 0 )
	ybgd/= npixy;
      // filter
      //
      if( detmap.pixel[i + j * detmap.nx] - xbgd >= threshold ||
	  detmap.pixel[i + j * detmap.nx] - ybgd >= threshold ){
	if( nbadpix < nbadpixbuffer ){
	  badpix[nbadpix].rawx = i+1;
	  badpix[nbadpix].rawy = j+1;
	  badpix[nbadpix].ccdnr = ccdid;
	  nbadpix++;
	}
	else
	  return error("not enough memory allocated");
      }//if
    }//for
  message("number of badpixels = ", nbadpix);

  return success();
}//findbadpixels

/****************************************************************************/
//
// FUNCTION    : markbadPNro()
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::markbadPNrow()
{
  if( nbadpixbuffer ==0 ||  nbadpix >= nbadpixbuffer || 
      strncasecmp(instrument,"EPN",3)!= 0 )
    return error("wrong instrument or no memory allocated!");

  for( unsigned long i= 13 ; i <= 200 && nbadpix < nbadpixbuffer ; i++){
    badpix[nbadpix].rawx = 11;
    badpix[nbadpix].rawy = i;
    badpix[nbadpix].ccdnr = 5;
    nbadpix++;
  }//for

  return SUCCESS;
}// markbadPNrow()


#endif




/****************************************************************************/
//
// FUNCTION    :      maskWindow(int *i1, int *j1, int *i2, int *j2, float threshold=0.5);
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  *i1, *j2, *i2, *j2
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::maskWindow(int *i1, int *j1, int *i2, int *j2, float threshold)
{
  setFunctionID("maskWindow");
  *i1=-1;
  *j1=-1;
  *i2=-1;
  *j2=-1;
  
  if( nx <=0 && ny <= 0){
    return error("Image is empty");
  }
  message("returning pixel coordinates min and max of pixels with values above ",threshold);

  //  bool threshold_found=false;

  // find smallest i with threshold
  *i1 = nx + 1;
  *j1 = ny + 1;
  *i2 = -1;
  *j2 = -1;
  for(int j=0; j<ny; j++){
    for(int i=0; i<nx; i++){
      if( pixel[i+j*nx] >= threshold ){
	if( *i1 > i ){
	  *i1 = i;
	}
	if( *j1 > j ){
	  *j1 = j;
	}
	if( *i2 < i ){
	  *i2 = i;
	}
	if( *j2 < j ){
	  *j2 = j;
	}
	
      }
    }
    
  }

  if( *i2 < -1 ){
    *i1 = -1;
    *j1 = -1;
    return error("no pixels found above threshold");
  }
  *i1 += 1; // difference between fits and C++ convention
  *j1 += 1;
  *i2 += 1;
  *j2 += 1;
  return success();
}




/****************************************************************************/
//
// FUNCTION    : meshrefinment
// DESCRIPTION : This uses mesh refinement to adaptively bin an image.
//               The original image is not changed, but instead a new (meshrefined)
//               image is produced plus an index map. This index map can be used
//               for PCA.
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   meshedimage        O                Mesh-refined image
//   index              O                Map with indices, a large block will have
//                                       same index
//   mnCnts             I                The minimum counts at which a mesh-refinement will
//                                       occur
//   levels             I                Mesh refinement levels. Minimum is 2
//                                       Maximum blocking factor will be 2^(levels-1).
//
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::meshrefinement(Image *meshedimage, Image *index, Image *binMap,
			     float minCnts, int levels)
{
  setFunctionID("meshrefinement");
  if( nx < 64 || ny < 64 ){
    return error("image too small");
  }
  
  if( !is_power_of_2( nx ) || !is_power_of_2( ny ) ){
    return error("image size is not a power of 2");
  }//if
  
  if( levels < 2 || levels > 10 ){
    return error("invalid number of levels ",levels);
  }

  message("Mesh refined executed; #levels = ", levels);

  //  string outputstr;
  int b=1;
  Image *binnedImages= new Image [levels];
  Image *indices     = new Image [levels];
  Image *binMaps     = new Image [levels];


  for(int i=0; i< levels; i++){
    binnedImages[i] = *this;
    if(b > 1 ){
      binnedImages[i].rebin( b );
    }

    indices[i]=binnedImages[i];
    if( i< levels-1){
      indices[i] *= 0.0;
    }else{
      indices[i].clip(0.5*minCnts, minCnts );
      indices[i] /= indices[i];
    }
    b *= 2; //bin factor
  }

  //  message("Rebinning at all levels done...");
  int indx=1, n;  // indx is a counter of how many final pixels over all levels are encountered
  for(int k=levels-1; k>= 1; k--){

    //    message("k = ", k);
    for( int j=0; j < indices[k].ny; j++){
      for( int i=0; i < indices[k].nx; i++){

	n=i + j*indices[k].nx;
	//	if( (k==levels -1) && binnedImages[k].pixel[n] < 0.25*minCnts ){
	//	  indices[k] = 0.0;
	//	}
	
	if( ( (binnedImages[k].pixel[n] < minCnts) ) //|| k==0)
	    && indices[k].pixel[n]>0.5 ){
	  indices[k].pixel[n] = indx;
	  indx++;
	}else{
	  if( k!=0 ){ // k==0 done below
	    if( binnedImages[k].pixel[n] >=  minCnts ){
	      indices[k].pixel[n] = 0.0;
	      binnedImages[k].pixel[n] = 0.0;
	      //	      binMaps[k].pixel[n] = 0.0;	      
	      int l0 = i*2;
	      int m0 = j*2;
	      indices[k-1].pixel[l0 + m0*indices[k-1].nx]=1.0;
	      indices[k-1].pixel[(l0+1) + m0*indices[k-1].nx]=1.0;
	      indices[k-1].pixel[ l0  + (m0+1)*indices[k-1].nx]=1.0;
	      indices[k-1].pixel[ (l0+1) + (m0+1)*indices[k-1].nx]=1.0;
	    }
	  }
	}
      }//for
    }//for


        
  }//for k

  //  message("Putting indices at unbinned level...");
  // This is only for unbinned level:
  for(int j=0; j<ny; j++){
      for(int i=0; i<nx; i++){
	if( indices[0].pixel[i+j*nx] > 0.5 ){
	  indices[0].pixel[i+j*nx] = indx;
	  indx++;
	}
      }
  }

  Image mask;
  b=1;


  //  message("Wrapping up....");
  for(int k=0; k<levels; k++){
    if( b>1){
      indices[k].zoom(b);
      binnedImages[k].zoom(b);
    }      

    mask = indices[k];
    mask /= indices[k];
    binnedImages[k] *= mask;
    binMaps[k] = mask;
    binMaps[k] *= b*b;

    
    if( k==0){
      *meshedimage = binnedImages[k];
      *index = indices[k];
      *binMap = binMaps[k];      
    }else{
      *meshedimage += binnedImages[k];
      *index += indices[k];
      *binMap += binMaps[k];            
    }

    //    string    outputstr = "testmesh_l" + to_string(k) + ".img.gz";
    //    binnedImages[k].save( outputstr);
    //    outputstr = "testmesh_l" + to_string(k) + "_index.img.gz";
    //    indices[k].save( outputstr);

    
    b *= 2;
  }

  *meshedimage/= *binMap;

  delete [] binnedImages;
  delete [] indices;
  delete [] binMaps;
  
  return success();
}


/****************************************************************************/
//
// FUNCTION    : meshedIndexBinning()
// DESCRIPTION : Uses the index image created by meshrefinement to
//               bin an image with the same mesh-refinement scheme.
//               It checkes whether the image sizes are compatible.
//               It does the rebinning in place, i.e. *this is changed!
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status Image::meshedIndexBinning(const Image &indexmap, const Image &binMap, double *mean )
{
  setFunctionID("meshedIndexBinning");

  if( nx == 0 || ny == 0)
    return error("no image present");
  if( (indexmap.nx != nx) || (indexmap.ny != ny) || (binMap.nx != nx) || (binMap.ny != ny) ){
    return error("unequal image sizes");
  }


  
  int maxindex=indexmap.maximum();
  if( maxindex <= 1 )
    return error("incorrect index map");


  message("starting index binning, max index = ", maxindex);
  int index;
  double *binnedvalues=new double [maxindex]();
  
  for(int j=0; j <ny; j++){
    for(int i=0; i <nx; i++){
      index =  (int)(indexmap.pixel[i + j*nx])-1;
      if( index >= 0 ){
	binnedvalues[index] += pixel[i + j*nx];
      }
    }
  }

  
  *mean=0.0;
  for(int j=0; j <ny; j++){
    for(int i=0; i <nx; i++){
      index =  (int)(indexmap.pixel[i + j*nx])-1;
      if( index >= 0 ){
	pixel[i + j*nx] = (float)binnedvalues[index];
	*mean += binnedvalues[index];
      }
      else
	pixel[i + j*nx] = 0.0;	
    }
  }
  
  *mean/= maxindex;


  *this /= binMap;


  delete [] binnedvalues;
  return success();
}


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
bool is_power_of_2( int n )
{
  if( n < 1 ){
    return false;
  }//if

  while( n > 2 ){
    if( n  % 2 == 1 ){
      return false;
    }
    n /= 2; 
  }//while

  return true;
}//  is_power_of_2( int n )





