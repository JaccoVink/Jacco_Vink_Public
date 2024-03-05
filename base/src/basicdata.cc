/*****************************************************************************/
//
// FILE        :  basicdata.cc
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
#include "jvtools_basic.h" // includes all other necessary files
#include "coordinates.h"

using jvtools::BasicData;
//using JVMessages::error;
//using JVMessages::message;
//using JVMessages::setFunctionID;

using namespace::JVMessages;

char jvtools::fname[FLEN_STATUS];
char jvtools::defaultname[FLEN_STATUS];

/****************************************************************************/
//
// FUNCTION    : BasicData()
// DESCRIPTION : Constructure for the BasicData class
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
/****************************************************************************/
BasicData::BasicData()
{  
  setFunctionID("BasicData");

  if( strcmp(classid,"BASICDATA")== 0){
    error("Constructor illegaly called!");
    return;
  }
  strcpy(classid,"BASICDATA");

  n_events = 0;    // events in buffer
  ntot_events = 0; // total events in file, not in buffer
  buffer_size = 0;
  n_gti = 0;


  channel = pattern =  ccdnr = node_id = grade = 0;
  flag = 0;
  x = y = detx = dety = 0;
  time = 0;

  gti = 0;
  nGTI  = 0;

  nbadpix = nbadpixbuffer = 0;
  badpix = 0;
  reset_keywords();


  polarizationColumns=false;

  applyPSF=false;
  chatty = TRUE;

  success();

}// BasicData()  constructor

/****************************************************************************/
//
// FUNCTION    : BasicData()
// DESCRIPTION : copy constructor
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
/****************************************************************************/
BasicData::BasicData( const BasicData &obs )
{
  setFunctionID("BasicData(copy constructor)");
  //  warning("Entering BasicData() 2 :");

  if( this == &obs || strcmp(classid,"BASICDATA") == 0){
    error("constructor illegaly called!");
    return;
  }

  n_events = 0;
  ntot_events = 0; // total events in file, not in buffer
  buffer_size = 0;
  n_gti = 0;
  nbadpix = nbadpixbuffer = 0;
  badpix = 0;

  resetKeywords();

  chatty = TRUE;

  // copy event list data
  if( obs.buffer_size != 0 ){

    if( allocate( obs.buffer_size ) == FAILURE ){
      warning("allocation error");
    }
    else{
      copyKeywords( obs ); // copy the keywords from object
	
      int *chan_ptr1 = &channel[0], *chan_ptr2 = &obs.channel[0];
      int *pat_ptr1 = &pattern[0], *pat_ptr2 = &obs.pattern[0];
      int *ccd_ptr1 = &ccdnr[0], *ccd_ptr2 = &obs.ccdnr[0];
      int *node_ptr1 = &node_id[0], *node_ptr2 = &obs.node_id[0];
      long *fl_ptr1 = &flag[0], *fl_ptr2 = &obs.flag[0];
      float *x_ptr1 = &x[0], *x_ptr2 = &obs.x[0];
      float *y_ptr1 = &y[0], *y_ptr2 = &obs.y[0];
      float *dx_ptr1 = &detx[0], *dx_ptr2 = &obs.detx[0];
      float *dy_ptr1 = &dety[0], *dy_ptr2 = &obs.dety[0];
      double *time_ptr1 = &time[0], *time_ptr2 = &obs.time[0];
      
      for( unsigned long i = 0; i < buffer_size ; i++ ){
	*chan_ptr1++ = *chan_ptr2++;
	*pat_ptr1++ = *pat_ptr2++;
	*ccd_ptr1++ = *ccd_ptr2++;
	*node_ptr1++ = *node_ptr2++;
	*fl_ptr1++ = *fl_ptr2++;
	*x_ptr1++ = *x_ptr2++;
	*y_ptr1++ = *y_ptr2++;
	*dx_ptr1++ = *dx_ptr2++;
	*dy_ptr1++ = *dy_ptr2++;
	*time_ptr1++ = *time_ptr2++;
	// Polarization info is not copied, should it change?
      }// end for 
    }//else
  }//if

  // copy gti data
  copy_GTIs( obs);

  if( copyBadpix( obs) == FAILURE )
    warning("badpix copying error!");


  chatty = TRUE;    
  success();
}// end BasicData()

/****************************************************************************/
//
// FUNCTION    : ~BasicData
// DESCRIPTION : Destructor
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
/****************************************************************************/
BasicData::~BasicData()
{

  BasicData::deallocate();
  BasicData::deallocateGTI( );
  BasicData::deallocate_gti();
  BasicData::deallocate_badpix();

  strcpy(classid,"ENDED");

}// destructor ~BasicData()


/****************************************************************************/
//
// FUNCTION    : allocate()
// DESCRIPTION : Allocates memory for the BasicData().
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//       n               I                 number of rows
//       m               I                           columns
//
//  RETURNS    :  SUCCESS or FAILURE
//
/****************************************************************************/
Status BasicData::allocate( unsigned long n )
{

  setFunctionID("allocate[BasicData]");

  if( n <= 0 )
    return error("Error in the input parameters.");

  if( buffer_size != n ){
    BasicData::deallocate();

    // Allocate memory

    channel = new int [n];
    pattern = new int [n];
    ccdnr   = new int [n];
    node_id = new int [n];
    flag    = new long [n];
    grade   = new int [n];
    x       = new float [n];
    y       = new float [n];
    detx    = new float [n];
    dety    = new float [n];
    chanEnergy = new float [n];
    time    = new double [n];

    if( polarizationColumns ){
      message("Allocating polarizarition information arrays");
      polAngle = new float [n];
      polx= new float [n];
      poly= new float [n];
      modfactor= new float [n];
    }else{
      message("Not allocating polarizarition information arrays");
      polAngle=polx=poly=modfactor=0;
    }
    
    n_events = ntot_events = 0;
    buffer_size = n;    

    // Check if the memory is allocated?

    if( channel == 0 || pattern == 0 || grade == 0 || ccdnr == 0 || 
	node_id==0 || flag == 0 || x == 0 || y == 0 || detx == 0 || 
	dety == 0 ||  time == 0){
      BasicData::deallocate();
      return error("Allocation error!");
    }// end if

  }//if

  for(unsigned long i = 0; i < buffer_size ; i++ ){
    channel[i] = 0;
    chanEnergy[i] = 0.0;
    flag[i]    = -1;
    pattern[i] = 0;
    ccdnr[i]   = -1;
    node_id[i] = -1;
    time[i] = 0.0;
    x[i] = y[i] = detx[i] = dety[i] = 0.0;
    //polarization angle?
    
  }//end for j

  //  if( chatty == TRUE )
  //   warning("allocate(): Memory buffer allocated.");

  JVMessages::message("succesfully allocated a buffer size of ", buffer_size);
  return JVMessages::success();

}// end allocate()

/****************************************************************************/
//
// FUNCTION    : deallocate()
// DESCRIPTION : Deallocates the data eventlist arrays BasicData.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :  SUCCESS
//
/****************************************************************************/
Status BasicData::deallocate()
{
  setFunctionID("deallocate[BasicData]");


  if( buffer_size != 0 ){
    delete [] channel;
    delete [] pattern;
    delete [] ccdnr;
    delete [] node_id;
    delete [] flag;
    delete [] grade;
    delete [] x;
    delete [] y;
    delete [] detx;
    delete [] dety;
    delete [] chanEnergy;
    delete [] time;
    if( polAngle != 0){
      message("Deallocating polarization arrays");
      delete [] polAngle;
      delete [] polx;
      delete [] poly;
      delete [] modfactor;
      polAngle=polx=poly=0;
    }
    
    channel = pattern =  ccdnr = node_id = grade = 0;
    flag = 0;
    x = y = detx = dety = 0;
    time = 0;
    n_events = ntot_events = 0;
    buffer_size = 0;
    //    if( chatty == TRUE )
    //      warning("deallocate(): Memory buffer deallocated.");

  } // end if


  return success();
    
}// end deallocate()


/*****************************************************************************/
//
// FUNCTION    : allocateGTI()
// DESCRIPTION : Allocate the good time interval (gti) arrays.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//    n                 I                       Number of elements
//
//  RETURNS    : SUCCESS or FAILURE;
//
/*****************************************************************************/
Status BasicData::allocateGTI(int nccd )
{
  setFunctionID("allocateGTI");

  if( nccd <= 0 )
    return error("wrong parameter value");

  if( nGTI >= 1)
    deallocateGTI();


  if( nccd == 1 ){
    gti = new GoodTimeIntervals;
    nGTI = 1;
  }//
  else{
    gti  = new GoodTimeIntervals [nccd];
    nGTI = nccd;
  }//else

  return success();
}// allocateGTI()
Status BasicData::deallocateGTI( void )
{
  setFunctionID("deallocateGTI");

  if( nGTI == 1 ){
    delete gti;
    gti = 0;
  }
  else if( nGTI > 1 ){
    delete []  gti;
    gti = 0;
  }//for

  nGTI = 0;

  return success();
}//deallocateGTI


/*****************************************************************************/
//
// FUNCTION    : allocate_gti()
// DESCRIPTION : Allocate the good time interval (gti) arrays.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//    n                 I                       Number of elements
//
//  RETURNS    : SUCCESS or FAILURE;
//
/*****************************************************************************/
Status BasicData::allocate_gti(  unsigned long n )
{
  //  warning("Entering allocate_gti() :");
  setFunctionID("allocate_gti");

  if( n <= 0 )
    return error("error in the input parameters");

  if( n_gti != n ){
    deallocate_gti();

    // Allocate memory
    start_gti = new double [n];
    stop_gti = new double [n];
    
  }//if

  n_gti = n;

  // Check if the memory is allocated?
  if( start_gti == 0 || stop_gti == 0 ){ 
    deallocate_gti();
    n_gti = 0;
    warning("allocate_gti(): Allocation error!");
    return FAILURE;
  }// end if

  for(unsigned long i = 0; i < n_gti ; i++ ){
    start_gti[i] = 0;
    stop_gti[i] = 0.0;
  }//end for j


  return success();
}//  allocate_gti()

/****************************************************************************/
//
// FUNCTION    : allocate_badpix()
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
Status BasicData::allocate_badpix( unsigned long n)
{
  setFunctionID("allocate_badpix");

  if( n <= 0 )
    return error("invalid input parameter!");

  if( nbadpix > 0 ){
    warning("destroying old badpix list!");
    deallocate_badpix();
  }//if

  badpix = new Badpixels [n];
  if( badpix != 0 ){
    nbadpixbuffer = n;
    nbadpix = 0;
    for( unsigned long i = 0 ; i < nbadpix ; i++ ){
      badpix[i].rawx = badpix[i].rawy = 0;
      badpix[i].type = badpix[i].flag = badpix[i].ccdnr = 0;
      badpix[i].yextent = 1;
    }//for
  }
  else
    return error("allocate_badpix(): error allocating memory!");

  return success();
}//allocate_badpix()


/****************************************************************************/
//
// FUNCTION    : operator=
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
BasicData & BasicData::operator=(const BasicData &events )
{
  setFunctionID("operator=");
  
  BasicData::deallocate();
  BasicData::deallocateGTI( );
  BasicData::deallocate_gti();
  BasicData::deallocate_badpix();
  


  allocate(events.n_events);

  for(unsigned long i=0; i < events.n_events; i++){
    channel[i] = events.channel[i];
    pattern[i] = events.pattern[i];
    ccdnr[i]   = events.ccdnr[i];
    node_id[i] = events.node_id[i];
    flag[i]    = events.flag[i];
    grade[i]   = events.grade[i];
    x[i]       = events.x[i];
    y[i]       = events.y[i];
    polAngle[i]= events.polAngle[i];
    detx[i]    = events.detx[i];
    dety[i]    = events.dety[i];
    time[i]    = events.time[i];
  }//for

  n_events = events.n_events;
  copyKeywords( events);

  success();
  return *this;

}//operator=

/****************************************************************************/
//
// FUNCTION    : operator+=
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
BasicData & BasicData::operator+=(const BasicData &events )
{
  setFunctionID("operator+=");
  
  if( n_events <= 0 && events.n_events <= 0 ){
    error("nothing to add");
    return *this;
  }//if

  BasicData temp;


  message("merging two event lists, 1: #events = ", n_events);
  message("merging two event lists, 2: #events = ", events.n_events);
  message("start time 1: ", start_time);
  message("start time 2: ", events.start_time);
  message("stop time  1: ", stop_time);
  message("stop time  2: ", events.stop_time);

  temp.copyKeywords( *this);
  temp.start_time = MIN( start_time, events.start_time);
  temp.stop_time = MAX( stop_time, events.stop_time);


  temp.allocate(n_events + events.n_events);

  for(unsigned long i=0; i < n_events; i++){
    temp.channel[i] = channel[i];
    temp.pattern[i] = pattern[i];
    temp.ccdnr[i]   = ccdnr[i];
    temp.node_id[i] = node_id[i];
    temp.flag[i]    = flag[i];
    temp.grade[i]   = grade[i];
    temp.x[i]       = x[i];
    temp.y[i]       = y[i];
    temp.polAngle[i]       = polAngle[i];
    temp.detx[i]    = detx[i];
    temp.dety[i]    = dety[i];
    temp.time[i]    = time[i];
  }//for

  double alpha, delta, newx, newy;
  for(unsigned long i=0; i < events.n_events; i++){
    temp.channel[i+n_events] = events.channel[i];
    temp.pattern[i+n_events] = events.pattern[i];
    temp.ccdnr[i+n_events]   = events.ccdnr[i];
    temp.node_id[i+n_events] = events.node_id[i];
    temp.flag[i+n_events]    = events.flag[i];
    temp.grade[i+n_events]   = events.grade[i];

    events.getradec(events.x[i], events.y[i], &alpha, &delta);
    temp.getxy(alpha, delta, &newx, &newy);
    temp.x[i+n_events]       = (float)newx;
    temp.y[i+n_events]       = (float)newy;
    temp.polAngle[i+n_events]= events.polAngle[i];
    temp.detx[i+n_events]    = events.detx[i];
    temp.dety[i+n_events]    = events.dety[i];
    temp.time[i+n_events]    = events.time[i];
  }//for

  temp.n_events = n_events + events.n_events;

  //  *this.copyKeywords( temp);

  *this = temp;

  message("merged two event lists, #events = ", n_events);
  message("start time : ", start_time);
  message("stop time  : ", stop_time);

  success();
  return *this;
}//operator+=



/*****************************************************************************/
//
// FUNCTION    : constrain_time()
// DESCRIPTION : Applies extra time filter to gti. tstart and tstop with 
//               respect to gti[0].
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//     tstart           I                         
//     tstop            I
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status BasicData::constrain_time(double tstart, double tstop )
{
  setFunctionID("constrain_time");

  if( n_gti <= 0 || tstart >= tstop || tstart < 0.0 )
    return error("Error in input parameters!");

  double tst, t0, tn;
  unsigned long new_ngti = 0;

  //  print(stdout);

  tst = start_gti[0];
  for(unsigned long i = 0; i < n_gti ; i++ ){

    t0 = start_gti[i] - tst;
    tn = stop_gti[i] -  tst;
    if( (t0 >= tstart && t0 <= tstop) ||  (tn >= tstart && tn <= tstop) ||
	(t0 <= tstart && tn >= tstop ) ){
      if( t0 < tstart )
	start_gti[new_ngti] = tst + tstart;
      else
	start_gti[new_ngti] = t0 + tst;
      if( tn > tstop )
	stop_gti[new_ngti] = tst + tstop;
      else	
	stop_gti[new_ngti] = tn + tst;
      new_ngti++;
    }//if

  }//end for i
  if( new_ngti < n_gti ){
    message("number of gti intervals changed from ",n_gti);
    message("to ",new_ngti);
    n_gti = new_ngti;
  }//if

  start_time = start_gti[0];
  stop_time = stop_gti[n_gti-1];

  //  print(stdout);

  return success();

}//constrain_time()


/****************************************************************************/
//
// FUNCTION    : excludeInterval()
// DESCRIPTION : 
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
/****************************************************************************/
Status BasicData::excludeInterval( double tbegin, double tend)
{
  setFunctionID("excludeInterval");

  message("start time = ", start_time);  
  message("stop time = ",  stop_time);  

  if( nGTI == 0 )
    return error("no GTIs available");

  message("Changing GTI");

  if( nGTI >= 1)
    message("Old exposure", gti[0].totalExposure());

  for( int i = 0; i < nGTI; i++)
    gti[i].excludeInterval(tbegin, tend);
  
  if( nGTI >= 1)
    message("New exposure", gti[0].totalExposure());

  return success();
}//excludeInterval()

/****************************************************************************/
//
// FUNCTION    : exclude_time()
// DESCRIPTION : 
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
/****************************************************************************/
Status BasicData::exclude_time( double tbegin, double tend)
{
  setFunctionID("exclude_time");

  if( n_gti == 0 || tend <= tbegin)
    return error("No GTIs present or input error!");


  BasicData newgti;

  if( newgti.allocate_gti( n_gti+2) == FAILURE )
    return error("allocation error");

  // step 1 adapt start and stop times to tbegin and tend
  unsigned long n=0;
  for(unsigned long i=0; i < n_gti ; i++){

    if( tbegin > stop_gti[i] || tend < start_gti[i]){
      newgti.start_gti[n] = start_gti[i];
      newgti.stop_gti[n] = stop_gti[i];
      if( newgti.stop_gti[n] > newgti.start_gti[n]) n++;
    }
    else if( tbegin >= start_gti[i] && tbegin < stop_gti[i] && 
	     tend >= stop_gti[i] && tbegin != start_gti[i]){
      newgti.start_gti[n] = start_gti[i];
      newgti.stop_gti[n] = tbegin;
      if( newgti.stop_gti[n] > newgti.start_gti[n]) n++;
    }
    else if( tbegin > start_gti[i] && tbegin < stop_gti[i] && 
	     tend < stop_gti[i] ){
      newgti.start_gti[n] = start_gti[i];
      newgti.stop_gti[n] = tbegin;
      if( newgti.stop_gti[n] > newgti.start_gti[n]) n++;
      newgti.start_gti[n] = tend;
      newgti.stop_gti[n]  = stop_gti[i];
      if( newgti.stop_gti[n] > newgti.start_gti[n]) n++;
    }
    else if( tend > start_gti[i] && tend <= stop_gti[i] ){
      newgti.start_gti[n] = tend;
      newgti.stop_gti[n] = stop_gti[i];
      if( newgti.stop_gti[n] > newgti.start_gti[n]) n++;
    }//if
  }//for

  newgti.n_gti = n;

  copy_GTIs( newgti );
  calc_exposure();

  message("Excluded the following interval:");
  message("  tstart    = ", tbegin);
  message("  tend      = ", tend);
  message("  duration  = ", tend-tbegin);
  message("  n_gti     = ", n_gti);
  message("  exposure  = ", exposure);
  //  for( unsigned long i = 0 ; i < n_gti; i++)
  //    printf("      %15.0f %15.0f\n", start_gti[i], stop_gti[i]);

  return success();

}//exclude_time()



/*****************************************************************************/
//
// FUNCTION    : deallocate_gti()
// DESCRIPTION : 
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
Status BasicData::deallocate_gti( )
{

  //  warning("Entering deallocate_gti() :");

  if( n_gti != 0 ){
    delete [] start_gti;
    delete [] stop_gti;
    n_gti = 0;
    start_gti = 0;
    stop_gti = 0;
  } // end if
 
  return success();

}//deallocate_gti( )

/****************************************************************************/
//
// FUNCTION    : deallocate_badpix()
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS or FAILURE;
//
/****************************************************************************/
Status BasicData::deallocate_badpix( )
{

  if( nbadpix > 0 ){
    delete [] badpix;
    badpix = 0;
    nbadpix = nbadpixbuffer = 0;
  }//if

  return SUCCESS;
}//deallocate_badpix();

/****************************************************************************/
//
// FUNCTION    : reset_keywords() 
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS
//
/****************************************************************************/
Status BasicData::reset_keywords(){
  return resetKeywords();
}
Status BasicData::resetKeywords()
{

  //  warning("Entering reset_keywords()");

  strncpy(telescope, "UNKNOWN", FLEN_VALUE );
  strncpy(instrument,"UNKNOWN", FLEN_VALUE);
  strncpy(detector,"UNKNOWN", FLEN_VALUE);
  strncpy(filter, "none", FLEN_VALUE);
  strncpy(grating, "none", FLEN_VALUE);
  strncpy(object, "UNKNOWN", FLEN_VALUE);
  strncpy(obsdate, "UNKNOWN", FLEN_VALUE);
  strncpy(enddate, "UNKNOWN", FLEN_VALUE);
  strncpy(chan_type, "PI", FLEN_VALUE);  
  strncpy(ctype1, "UNKNOWN", FLEN_VALUE);
  strncpy(ctype2, "UNKNOWN", FLEN_VALUE);

  strncpy(ctype1, "RA---TAN", FLEN_VALUE);
  strncpy(ctype2, "DEC--TAN", FLEN_VALUE);
  //  strncpy(ctype2, "UNKNOWN", FLEN_VALUE);
  
  strncpy(radecsys, "UNKNOWN", FLEN_VALUE);
  strncpy(radecsys, "FK5", FLEN_VALUE);
  strncpy(observer, "UNKNOWN", FLEN_VALUE);
  strncpy(creator, classid, FLEN_VALUE);
  
  dec_nom   = 0.0;
  ra_nom    = 0.0;
  roll_nom  = 0.0;
  opticx    = 0.0;
  opticy    = 0.0;
  bscale    = 1.0;
  bzero     = 0.0;
  crval1    = crval2 = 0.0;
  crpix1    = crpix2 = 0.0;
  cdelt1    = cdelt2 = 0.0;
  crota2    = 0.0;
  pc001001  = 1.0;
  pc002002  = 1.0;
  pc001002  = 0.0;
  pc002001  = 0.0;
  longpole  = 180.0;
  drpix1    = drpix2  = 0.0;
  drdelt1   = drdelt2 = 0.0;
  drval1    = drval2  = 0.0;
  blank     = 0.0;
  //  blankFloat = MINFLOAT;
  blankFloat = -31515.0;
  equinox   = 0.0;
  exposure  = 0.0;
  deadtime_corr = 1.0;
  ontime   = 0.0;
  start_time = 0.0;
  stop_time = 0.0;
  mjd_obs   = 0.0;
  mjdrefi  = 0.0;
  mjdreff  = 0.0;
  mjdmean   = 0.0;
  chanMin = 0;
  chanMax = 2047;
  projectiontype = UNKNOWN;
  xy_type = DET_XY;
  instrumentID = UNDEF;

  beta_ref = 0.0;
  beta_wid = 0.0;
  xdsp_ref = 0.0;
  xdsp_wid = 0.0;
  alphanom = 0.0;
  rgsalpha = RGSALPHA;
  line_sep = DGROOVE;

  rflorder = 0;
  nhistory =0;

  return SUCCESS;
}// reset_keywords( )

/****************************************************************************/
//
// FUNCTION    : copy_keywords() 
// DESCRIPTION : 
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS
//
/****************************************************************************/
Status BasicData::copy_keywords( BasicData const &obs){
  return copyKeywords( obs);
}
Status BasicData::copyKeywords( BasicData const &obs)
{
  setFunctionID("copyKeywords");

  if( this != &obs ){
    strncpy(instrument, obs.instrument, FLEN_VALUE-1);
    strncpy(telescope, obs.telescope, FLEN_VALUE-1);
    strncpy(filter, obs.filter, FLEN_VALUE-1);
    strncpy(object, obs.object, FLEN_VALUE-1);
    strncpy(obsdate, obs.obsdate, FLEN_VALUE-1);
    strncpy(chan_type, obs.chan_type, FLEN_VALUE-1);
    strncpy(ctype1, obs.ctype1, FLEN_VALUE-1);
    strncpy(ctype2, obs.ctype2, FLEN_VALUE-1);
    strncpy(radecsys, obs.radecsys, FLEN_VALUE-1);
    strncpy(observer, obs.observer, FLEN_VALUE-1);
    strncpy(creator, obs.creator, FLEN_VALUE-1);
    
    dec_nom   = obs.dec_nom;
    ra_nom    = obs.ra_nom;
    crval1    = obs.crval1;
    crval2    = obs.crval2;
    crpix1    = obs.crpix1; 
    crpix2    = obs.crpix2;
    cdelt1    = obs.cdelt1;
    cdelt2    = obs.cdelt2;
    crota2    = obs.crota2;
    pc001001  = obs.pc001001;
    pc002002  = obs.pc002002;
    pc001002  = obs.pc001002;
    pc002001  = obs.pc002001;
    longpole  = obs.longpole;
    drpix1    = obs.drpix1;
    drpix2    = obs.drpix2;
    drdelt1   = obs.drdelt1;
    drdelt2   = obs.drdelt2;
    drval1    = obs.drval1;
    drval2    = obs.drval2;
    roll_nom  = obs.roll_nom;
    opticx    = obs.opticx;
    opticy    = obs.opticy;
    blank     = obs.blank;
    bscale    = obs.bscale;
    bzero     = obs.bzero;
    equinox   = obs.equinox;
    exposure  = obs.exposure;
    deadtime_corr = obs.deadtime_corr;
    start_time = obs.start_time;
    stop_time  = obs.stop_time;
    ontime    = obs.ontime;
    mjd_obs   = obs.mjd_obs;
    chanMin  = obs.chanMin;
    chanMax  = obs.chanMax;
    beta_ref  = obs.beta_ref;
    beta_wid  = obs.beta_wid;
    xdsp_ref  = obs.xdsp_ref;
    xdsp_wid  = obs.xdsp_wid;
    rflorder  = obs.rflorder;
    alphanom  = obs.alphanom;
    rgsalpha  = obs.rgsalpha;
    line_sep  = obs.line_sep;
    instrumentID   = obs.instrumentID; 
    projectiontype = obs.projectiontype;
    setprojection();
    //    message("projection = ",projectiontype);
  }
  else
    return error("cannot copy to the same object");
  //  message("copied fits keywords");

  return success();
}// copyKeywords( )

/****************************************************************************/
//
// FUNCTION    : copy_GTIs()
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		 I/O			DESCRIPTION
//    obs                I                      object to copy GTIs from
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status BasicData::copy_GTIs( const BasicData &obs)
{
#if 0
  // copy gti data
  if( obs.n_gti != 0 ){
    if( allocate_gti( obs.n_gti ) == FAILURE )
      warning("BasicData()/copy: GTI allocation error!");
    for( unsigned long i = 0 ; i < n_gti ; i++ ){
      start_gti[i] = obs.start_gti[i];
      stop_gti[i] = obs.stop_gti[i];
    }//for
  }//if
  
#endif

  if( obs.nGTI != 0 ){
    
    deallocateGTI();
    if( allocateGTI( obs.nGTI ) == FAILURE )
      return JVMessages::error("GTI allocation error");
    for( int i = 0; i < nGTI; i++)
      gti[i] = obs.gti[i];

  }//if

  calc_exposure();

  return success();
}//copy_GTIs


/****************************************************************************/
//
// FUNCTION    : copyBadpix()
// DESCRIPTION : Copies the badpixel part to another object.
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
Status BasicData::copy_badpix( const BasicData &obs)
{
  return copyBadpix(obs);
}//
Status BasicData::copyBadpix( const BasicData &obs)
{
  setFunctionID("copyBadpix");

  // copy badpixel data
  if( obs.nbadpix != 0 ){
    if( allocate_badpix( obs.nbadpixbuffer ) == FAILURE )
      return error("badpix allocation error!");
    else{
      for( unsigned long i = 0 ; i < obs.nbadpix; i++){
	badpix[i].rawx = obs.badpix[i].rawx;
	badpix[i].rawy = obs.badpix[i].rawy;
	badpix[i].ccdnr = obs.badpix[i].ccdnr;
	badpix[i].yextent = obs.badpix[i].yextent;
	badpix[i].flag = obs.badpix[i].flag;
      }//for
      nbadpix = obs.nbadpix;
    }
  }//if

  message("copied badpix section, nbadpix = ",nbadpix);

  return success();
}//copyBadpix()

/*****************************************************************************/
//
// FUNCTION    : getExposure()
// DESCRIPTION : Returns the exposure time.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :  exposure time
//
/*****************************************************************************/
double BasicData::getExposure( void ) const
{
  setFunctionID("getExposure");

  if( exposure <= 0.0 )
    error("possibly an invalid exposure time");
  
  success();
  return exposure;

}//end getExposure()

/*****************************************************************************/
//
// FUNCTION    : gettstart(0/ getTstop()
// DESCRIPTION : Returns the start/stop time.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :  exposure time
//
/*****************************************************************************/
double BasicData::getTstart( void ) const
{
  return start_time;
}
double BasicData::getTstop( void ) const
{
  return stop_time;
}
double BasicData::getMjdObs( void ) const
{
  return mjd_obs;
}
double BasicData::getMjdMean( void ) const
{
  setFunctionID("getMjdMean");
  double mjdm=0.0;
  if( mjdmean <= 0.0){
    warning("MJDMEAN not defined, using MJD-OBS instead");
    mjdm=mjd_obs;
    message("MJD-OBS=",mjdm);
  }
  else
    mjdm=mjdmean;
  
  success();
  return mjdm;
}

/****************************************************************************/
//
// FUNCTION    : getInstrument()
// DESCRIPTION : returns the instrument code
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
/****************************************************************************/
Instrument BasicData::get_instrument( void ){
  return getInstrument();
}
Instrument BasicData::getInstrumentConst( void ) const
{
  return instrumentID;
}
Instrument BasicData::getInstrument( void )
{
  setFunctionID("getInstrument");

  if( strncasecmp(telescope,"XMM",3)==0 ){
    if( strncasecmp(instrument,"EPN",3)==0 )
      instrumentID = XMM_PN;
    else if(  strncasecmp(instrument,"EM1",3)==0 ||  
	      strncasecmp(instrument,"EMOS1",5)==0 )
      instrumentID = XMM_MOS1;
    else if(  strncasecmp(instrument,"EM2",3)==0 ||
	      strncasecmp(instrument,"EMOS2",5)==0 )
      instrumentID = XMM_MOS2;
    if( strncasecmp(instrument,"RGS1",4)==0 )
      instrumentID = XMM_RGS1;
    if( strncasecmp(instrument,"RGS2",4)==0 )
      instrumentID = XMM_RGS2;
    if( strncasecmp(instrument,"OM",2)==0 )
      instrumentID = XMM_OM;
  }
  else if( strncasecmp(telescope,"CHANDRA",7)==0 ){
    if( strncasecmp(instrument,"ACIS",4)==0 ){
      if( strncasecmp(grating,"LETG",4)==0 )
	instrumentID = CHANDRA_LETG_ACIS;
      else if( strncasecmp(grating,"METG",4)==0 )
	instrumentID = CHANDRA_METG_ACIS;
      else  if( strncasecmp(grating,"HETG",4)==0 )
       	instrumentID = CHANDRA_HETG_ACIS;
      else
       	instrumentID = CHANDRA_ACIS;
    }else if( strncasecmp(instrument,"HRC",3)==0 ){
      if( strncasecmp(grating,"LETG",4)==0 )
	instrumentID = CHANDRA_LETG_HRC;
      else
	instrumentID = CHANDRA_HRC;
    }
  }
  else if(  strncasecmp(telescope,"ASCA",4)==0 ){
    if( strncasecmp(instrument,"SIS0",4)==0 )
      instrumentID = ASCASIS0;
    else if( strncasecmp(instrument,"SIS1",4)==0 )
      instrumentID = ASCASIS1;
    else if( strncasecmp(instrument,"GIS",3)==0 )
      instrumentID = ASCAGIS;
  }//if
  else if( strncasecmp(telescope,"ROSAT",5)==0 ){
    if( strncasecmp(instrument,"HRI",3)==0 )
      instrumentID = ROSAT_HRI;
    else if( strncasecmp(instrument,"PSPC",4)==0 )
      instrumentID = ROSAT_PSPC;
    else
      JVMessages::warning("Unknown instrument");
  }
  else if(  strncasecmp(telescope,"EINSTEIN",4)==0 ){
    if( strncasecmp(instrument,"HRI",3)==0 )
      instrumentID = EINSTEIN_HRI;
    else
       JVMessages::warning("Unknown instrument");
  }
  else if(  strncasecmp(telescope,"SAX",3)==0 || 
	    strncasecmp(telescope,"BEPPOSAX",7)==0 ){
    if( strncasecmp(instrument,"PDS",3)==0 )
      instrumentID = SAX_PDS;
    else
       JVMessages::warning("Unknown instrument",instrument);
  }
  else if(  strncasecmp(telescope,"CGRO",4)==0  || 
	    strncasecmp(telescope,"GRO",3)==0 ){
    if( strncasecmp(instrument,"OSSE",3)==0 )
      instrumentID = CGRO_OSSE;
    else
       JVMessages::warning("Unknown instrument",instrument);
  }
  else if(  strncasecmp(telescope,"INTEGRAL",8)==0 ){
    if( strncasecmp(instrument,"IBIS",4)==0 )
      instrumentID = INTEGRAL_IBIS;
    else if(strncasecmp(instrument,"SPI",3)==0  ) 
            instrumentID = INTEGRAL_SPI;
    else
       JVMessages::warning("Unknown instrument",instrument);
  }
  else if( strncasecmp(telescope,"SUZAKU",6)==0 ){
    if( strncasecmp(instrument,"XIS0",4)==0 )
      instrumentID = SUZAKU_XIS0;
    else if( strncasecmp(instrument,"XIS1",4)==0 )
      instrumentID = SUZAKU_XIS1;
    else if( strncasecmp(instrument,"XIS2",4)==0 )
      instrumentID = SUZAKU_XIS2;
    else if( strncasecmp(instrument,"XIS3",4)==0 )
      instrumentID = SUZAKU_XIS3;
  }
  else if( strncasecmp(telescope,"SWIFT",6)==0 && strncasecmp(instrument,"XRT",3)==0 ){
    instrumentID = SWIFT_XRT;
  }
  else if( strncasecmp(telescope,"XIPE",4)==0 &&
	   strncasecmp(instrument,"EXP2",4)==0 ){
    instrumentID = XIPE_GPD;
  }
  else if( strncasecmp(telescope,"IXPE",4)==0 &&
	   strncasecmp(instrument,"GPD",4)==0 ){
    instrumentID = IXPE_GPD;
  }
  else if(  strncasecmp(telescope,"Spitzer",7)==0 ){
    if(  strncasecmp(instrument,"IRAC",4)==0 ){
      instrumentID = SPITZER_IRAC;
    }else if( strncasecmp(instrument,"MIPS",4)==0 ){
      instrumentID = SPITZER_MIPS;
    }else if( strncasecmp(instrument,"IRS",4)==0 ){
      instrumentID = SPITZER_IRS;
    }
  }else if(  strncasecmp(telescope,"PACS",4)==0 ){
    instrumentID= HERSCHEL_PACS;
  }else if(  strncasecmp(telescope,"GLAST",5)==0 &&  
	     strncasecmp(instrument,"LAT",3)==0 ){
    instrumentID = FERMI_LAT;
 
  }else if(  strncasecmp(telescope,"HST",3)==0 &&  
	     strncasecmp(instrument,"ACS",3)==0 ){
    instrumentID = HST_ACS;
  }
  else
     JVMessages::warning("Unknown telescope ",telescope);


  if( chatty == true){
     JVMessages::message("instrument code = ",(long)instrumentID);
     switch(instrumentID){
     case ASCASIS0:
       JVMessages::message("instrument ","ASCA SIS0");
       break;
     case ASCASIS1:
       JVMessages::message("instrument ","ASCA SIS1");
       break;
     case ASCAGIS:
       JVMessages::message("instrument ","ASCA GIS12");
       break;
     case ROSAT_PSPC:
       JVMessages::message("instrument ","ROSAT PSPC");
       break;
     case ROSAT_HRI:
       JVMessages::message("instrument ","ROSAT HRI");
       break;
     case SAX_LE:
       JVMessages::message("instrument ","SAX LE");
       break;
     case SAX_ME:
       JVMessages::message("instrument ","SAX ME");
       break;
     case SAX_HP:
       JVMessages::message("instrument ","SAX HPGPSC");
       break;
     case SAX_PDS:
       JVMessages::message("instrument ","SAX PDS");
       break;
     case CGRO_OSSE:
       JVMessages::message("instrument ","CGRO OSSE");
       break;
     case XTE:
       JVMessages::message("instrument ","RXTE");
       break;
     case EXOSAT_LE:
       JVMessages::message("instrument ","EXOSAT LE");
       break;
     case EXOSAT_ME:
       JVMessages::message("instrument ","EXOSAT ME");
       break;
     case EINSTEIN_SSS:
       JVMessages::message("instrument ","EINSTEIN SSS");
       break;
     case EINSTEIN_HRI:
       JVMessages::message("instrument ","EINSTEIN HRI");
       break;
     case XMM_PN:
       JVMessages::message("instrument ","XMM-NEWTON PN");
       break;
     case XMM_MOS1:
       JVMessages::message("instrument ","XMM-NEWTON MOS1");
       break;
     case XMM_MOS2:
       JVMessages::message("instrument ","XMM-NEWTON MOS2");
       break;
     case XMM_RGS1:
       JVMessages::message("instrument ","XMM-NEWTON RGS1");
       break;
     case XMM_RGS2:
       JVMessages::message("instrument ","XMM-NEWTON RGS2");
       break;
     case XMM_OM:
       JVMessages::message("instrument ","XMM-NEWTON OM");
       break;
     case CHANDRA_ACIS:
       JVMessages::message("instrument ","CHANDRA ACIS");
       break;
     case CHANDRA_HRC:
       JVMessages::message("instrument ","CHANDRA HRC");
       break;
     case CHANDRA_LETG_HRC:
       JVMessages::message("instrument ","CHANDRA LETGS HRC");
       break;
     case INTEGRAL_IBIS:
       JVMessages::message("instrument ","INTEGRAL IBIS");
       break;
     case INTEGRAL_SPI:
       JVMessages::message("instrument ","INTEGRAL SPI");
       break;
     case FERMI_LAT:
       JVMessages::message("instrument ","FERMI_LAT");
       break;
     default:
       JVMessages::message("instrument ","UNKNOWN");
     };
  }
  JVMessages::success();
  return instrumentID;
}//getInstrument()


/****************************************************************************/
//
// FUNCTION    : get_chipsize()
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
// xsize,ysize           O                 chipsize, x y direction
//
//  RETURNS    : void
//
/****************************************************************************/
void BasicData::get_chipsize( Instrument instr,
				int *xsize, int *ysize, int *nchip) const
{
  setFunctionID("get_chipsize");

  switch( instr ){
  case XMM_PN:
    *xsize = 64;
    *ysize = 200;
    *nchip = 12;
    break;
  case XMM_MOS1:
  case XMM_MOS2:
    *xsize = 600;
    *ysize = 600;
    *nchip = 7;
    break;
  case XMM_RGS1:
  case XMM_RGS2:
    *xsize = 342;
    *ysize = 128;
    *nchip = 8;
    break;
  case CHANDRA_ACIS:
    *xsize = 1024;
    *ysize = 1024;
    *nchip = 8;
    break;
  default:
    *xsize = 512;
    *ysize = 512;
  }
  message("Chip width  = ", *xsize);
  message("Chip length = ", *ysize);

  success();
}//get_chipsize()



/****************************************************************************/
//
// FUNCTION    : getFilter()
// DESCRIPTION : 
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
/****************************************************************************/
Status BasicData::getFilter( char *filtername) const
{
  strncpy(filtername,filter,FLEN_VALUE);
  return SUCCESS;
}//

/****************************************************************************/
//
// FUNCTION    : setprojection()
// DESCRIPTION : Sets the projectiontype. Using a enum is faster than 
//               constantly evaluating the ctype keywords.
// 
//
// SIDE EFFECTS: in progress
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : SUCCESS
//
/****************************************************************************/
Status BasicData::setprojection( void )
{
  setFunctionID("setprojection");
  
  projectiontype =  UNKNOWN;
  if(strncmp(ctype1,"RA---TAN",8 )== 0 && strncmp(ctype2,"DEC--TAN",8 )== 0 ){
    projectiontype = RADECTAN;
  }else if( strncmp(ctype1,"RA---SIN",8 )== 0 &&
	   strncmp(ctype2,"DEC--SIN",8 )== 0 ){
    projectiontype = RADECSIN;
  }else if( strncmp(ctype1,"RA---NCP",8 )== 0 &&
	   strncmp(ctype2,"DEC--NCP",8 )== 0 ){
    projectiontype = RADECSIN;
  }
  else if( strncmp(ctype1,"RA---ARC",8 )== 0 &&
	   strncmp(ctype2,"DEC--ARC",8 )== 0 ){
    projectiontype = RADECARC;
  }//if
  else if( (strncmp(ctype1,"DET----X",8 )== 0 &&
	    strncmp(ctype2,"DET----Y",8 )== 0) ||
	   (strncmp(ctype1,"DETX",8 )== 0 &&
	    strncmp(ctype2,"DETY",8 )== 0)){
    projectiontype = DETECTOR;
  }else{
    message("ctype 1 = ", ctype1);
    message("ctype 2 = ", ctype2);
    warning("Could not assess te projection type.");
  }


  return success();
}//setprojection()


/****************************************************************************/
//
// FUNCTION    : getCoordinateType()
// DESCRIPTION : returns coordinate code
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
/****************************************************************************/
Coordinate_type BasicData::getxytype() const
{
  //  setCoordinateType();
  setFunctionID("getxytype");
  warning("xy type = ", xy_type);
  success();
  return xy_type;
}// getCoordinateType()
Coordinate_type BasicData::setCoordinateType( void )
{
  //  setFunctionID("getCoordinateType()");
  xy_type = UNDEF_XY;
  
  if((strncmp(ctype1,"RA---TAN",8 )== 0 && strncmp(ctype2,"DEC--TAN",8)== 0))
    xy_type = SKY_XY;
  else if( (strncmp(ctype1,"RA---SIN",8 )== 0 && 
	    strncmp(ctype2,"DEC--SIN",8)== 0) )
    xy_type = SKY_XY;
  else if( (strncmp(ctype1,"RA---NCP",8 )== 0 && 
	    strncmp(ctype2,"DEC--NCP",8)== 0) ) // no quite correct!!
    xy_type = SKY_XY;
  else if( (strncmp(ctype1,"RA---ARC",8 )== 0 && 
	    strncmp(ctype2,"DEC--ARC",8)== 0) )
    xy_type = SKY_XY;
  else if((strncmp(ctype1,"DET----X",8 )== 0 && 
           strncmp(ctype2,"DET----Y",8 )== 0) ||  
          (strncmp(ctype1,"DETX",8 )== 0 && 
           strncmp(ctype2,"DETY",8 )== 0))
    xy_type = DET_XY;
  else if((strncmp(ctype1,"CHIPX",8 )== 0 && 
           strncmp(ctype2,"CHIPY",8 )== 0) ||  
          (strncmp(ctype1,"RAWX",8 )== 0 && 
           strncmp(ctype2,"RAWY",8 )== 0))
    xy_type = RAW_XY;
  else if((strncmp(ctype1,"RGS--DSP",8 )== 0 && 
           strncmp(ctype2,"RGS-XDSP",8 )== 0) )
    xy_type = XMM_RGS_XDSP;
  else if((strncmp(ctype1,"RGS--DSP",8 )== 0 && 
           strncmp(ctype2,"RGS---PI",8 )== 0) )
    xy_type = XMM_RGS_PI;
  else if((strncmp(ctype1,"RGS-DSPR",8 )== 0 && 
           strncmp(ctype2,"RGS---PI",8 )== 0) )
    xy_type = XMM_RGS_PI_RAD;
  else if((strncmp(ctype1,"RGS-DSPR",8 )== 0 && 
           strncmp(ctype2,"RGSXDSPR",8 )== 0) )
    xy_type = XMM_RGS_XDSP_RAD;

  //  message("XY TYPE = ", xy_type);
  setprojection();
  return xy_type;

}//getxytype();


/****************************************************************************/
//
// FUNCTION    :  getopticxy()
// DESCRIPTION : 
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
// opticx,opticy        O
//
//  RETURNS    : SUCCESS
//
/****************************************************************************/
Status BasicData::getopticxy( double *optx, double *opty) const
{
  *optx = opticx;
  *opty = opticy;

  return SUCCESS;

}//getopticxy()

/*****************************************************************************/
//
// FUNCTION    : get_nevents()
// DESCRIPTION : Return n_events 
//               (the number of events selected from the event list)
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : n_events
//
/*****************************************************************************/
unsigned long BasicData::get_nevents() const
{
  return n_events;

}// get_nevents()

/*****************************************************************************/
//
// FUNCTION    : calcExposure()
// DESCRIPTION : Calculates the exposure from the good time intervals (GTI)
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :  exposure time
//
/*****************************************************************************/
double BasicData::calc_exposure()
{
  return calcExposure();
}
double BasicData::calcExposure(int detector)
{
  setFunctionID("calcExposure");

  if( nGTI == 0 ){
    warning("No GTIs available");
    return exposure;
  }//if

  double onTime = 0.0;
  if( detector < 0 ){
    for( int i = 0; i < nGTI; i++){
      onTime = MAX( onTime, gti[i].totalExposure());
      start_time = MIN(start_time, gti[i].startGTI[0]);
      stop_time = MIN(stop_time, gti[i].stopGTI[ gti[i].nGTI-1]);
    }
  }
  else{
    if( detector > nGTI ){
      warning("wrong exposure, detector number > nGTI; using 0");
      detector = 0;
    }
    onTime = gti[detector].totalExposure();
    start_time = MIN(start_time, gti[detector].startGTI[0]);
    stop_time = MIN(stop_time, gti[detector].stopGTI[ gti[detector].nGTI-1]);
  }

  exposure = onTime;
  message("Exposure (s)  = ",exposure);
  message("Updated Start Time = ", start_time);
  message("Updated Stop Time = ", stop_time);

  success();
  return exposure;
}// calc_exposure()


/*****************************************************************************/
//
// FUNCTION    : get_timespan()
// DESCRIPTION : Calculates the timespan (=tstart - stop).
//               Assume the GTIs are time ordered!
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :  time span
//
/*****************************************************************************/
double BasicData::get_timespan() const
{
  setFunctionID("get_timespan()");
  if( n_gti == 0 && start_time == 0.0 && stop_time == 0.0){
    error("No GTIs or timing information available!");
    return 0.0;
  }//if

  double timespan;

  if(n_gti != 0 )
    timespan = stop_gti[n_gti-1] - start_gti[0];
  else{
    warning("No GTI availabe. Using start/stop time instead!");
    timespan = stop_time - start_time;
  }//if

  success();
  return timespan;

}// get_timespan()


/*****************************************************************************/
//
// FUNCTION    : get_timespan()
// DESCRIPTION : Calculates the timespan (=tstart - stop).
//               Assume the GTIs are time ordered!
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    :  time span
//
/*****************************************************************************/
double BasicData::getTimespan() const
{
  setFunctionID("getTimespan");
  if( nGTI == 0 && start_time == 0.0 && stop_time == 0.0){
    error("No GTIs or timing information available!");
    return 0.0;
  }//if

  double timespan;

  if(nGTI == 0 ){
    warning("No GTI availabe. Using start/stop time instead!");
    timespan = stop_time - start_time;
    message("old start_time = ", start_time);
    message("old stop_time  = ", stop_time);
    return timespan;
  }//if

  message("old start_time = ", start_time);
  message("old stop_time  = ", stop_time);
  timespan = stop_time - start_time;

  message("old time span  = ", timespan);

  double startTime = gti[0].startGTI[0];
  double stopTime =  gti[0].stopGTI[0];
  for( int i= 0 ; i < nGTI; i++){
    startTime = MIN( startTime, gti[i].startGTI[0] );
    stopTime = MAX( stopTime, gti[i].startGTI[gti[i].nGTI-1] );
  }

  message("new start_time = ", startTime);
  message("new stop_time  = ", stopTime);
  timespan = stopTime - startTime;

  message("new time span  = ", timespan);

  success();
  return timespan;

}// getTimespan()

/*****************************************************************************/
//
// FUNCTION    : setCreator()
// DESCRIPTION : Changes the creator keyword
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
Status BasicData::setCreator( const char creatorName[])
{
  setFunctionID("setCreator");

  if( strlen(creatorName) <= 1 )
    return error("incorrect input parameter");

  strncpy( creator, creatorName, FLEN_VALUE-1);

  return success();
}// setCreator()

/*****************************************************************************/
//
// FUNCTION    : set_instrument()
// DESCRIPTION : Changes the instrument
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
Status BasicData::set_instrument( char instrname[])
{
  setFunctionID("set_instrument");

  if( strlen(instrname) <= 1 )
    return error("Incorrect input parameter. ");

  message("Changing instrument name.");
  message("From ",instrument);
  strncpy( instrument, instrname, FLEN_VALUE-1);
  message("To   ",instrument);

  get_instrument();

  return success();
}// set_instrument()creator()
Status BasicData::set_filter( char filtername[])
{
  setFunctionID("set_filter");

  if( strlen(filtername) <= 1 )
    return error("Incorrect input parameter. ");

  message("Changing filter name.");
  message("From ",filter);
  strncpy( filter, filtername, FLEN_VALUE-1);
  message("To   ",filter);

  //  get_instrument();

  return success();
}// set_instrument()creator()

/*****************************************************************************/
//
// FUNCTION    : set_telescope()
// DESCRIPTION : Changes the telescop keyword
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
Status BasicData::set_telescope( char telescopename[])
{
  setFunctionID("set_telescope");

  if( strlen(telescopename) <= 1 )
    return error("incorrect input parameter");

  message("Changing telescope name.");
  message("From ",telescope);
  strncpy( telescope, telescopename, FLEN_VALUE-1);
  message("To   ",telescope);

  get_instrument();
  return success();
}// set_telescope

/*****************************************************************************/
//
// FUNCTION    : set_object()
// DESCRIPTION : Changes the object keyword
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
Status BasicData::set_object( char objectname[])
{
  setFunctionID("set_object");

  if( strlen(objectname) <= 1 )
    return error("Incorrect input parameter. ");
  strncpy(object, objectname, FLEN_VALUE-1);

  return success();
}// set_object



/****************************************************************************/
//
// FUNCTION    : getradec()
// DESCRIPTION : Return the RA and DEC of pixel x,y
//               Only use message if chatty is set, as this function is
//               sometimes often called.
//
// SIDE EFFECTS: Note the first pixel is 1, 1 (0,0 in C++)
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   x, y,                I
//   alpha, delta         O                 position in degrees.
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
Status BasicData::getradec(double x, double y, 
			   double *alpha, double *delta) const
{
  setFunctionID("getradec");

  if( cdelt1 == 0.0 || cdelt2 == 0.0)
    return error("no physical coordinates specified");

  double phi, theta, rt, x0, y0;
  double ap = crval1*DEG2RAD, deltp = crval2 * DEG2RAD;
  double phip = longpole*DEG2RAD; 
  // pole of coordinate system

  //  *alpha = 0.0;
  //  *delta = 0.0; // omit for speed reasons

  // first calculate phi, theta (i,j)
  //
  // (note that the pixels start by convention on 1 instead 0)

  // Inverse rotation:
  x -= crpix1;
  y -= crpix2;
  x0 = pc001001 * x + pc001002 * y;
  y0 = pc002001 * x + pc002002 * y;
  x0 *= cdelt1;
  y0 *= cdelt2;
  rt  = sqrt( x0*x0 + y0*y0 );               // Rtheta in GC
  phi = atan2( x0, -y0);                     // argument of x and y

  if( rt != 0.0 ){
    if( projectiontype == RADECTAN ){
      theta = atan( RAD2DEG/rt );
    }
    else{
      if(  projectiontype == RADECSIN ) 
	  theta = acos( rt*DEG2RAD );
      else{
	//setChatty( true);
	message("ctype1 = ",ctype1);
	message("ctype2 = ",ctype2);
	message("projection type  = ",projectiontype);
	return error("no ctype keywords specified or not implemented");
      }
    }//else

    // calculate alpha and delta
    ch_coord(phi, theta, phip, ap, deltp, alpha, delta);
    *alpha *= RAD2DEG;
    *delta *= RAD2DEG;
  }
  else{
    *alpha = crval1;
    *delta = crval2; 
    //    setChatty( true);
    return error("theta not specified!");
  }

  return success();
  //  return SUCCESS;
}// getradec()

/*****************************************************************************/
//
// FUNCTION    : getxy()
// DESCRIPTION : Return the X,Y for a certain RA DEC
// 
//
// SIDE EFFECTS: Note that it is not checked if x and y are within the limits.
//               So the validity of the result is not checked.
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//   alpha, delta         I                 position in degrees.
//   x, y,                O
//
//  RETURNS    : SUCCESS or FAILURE
//
/*****************************************************************************/
Status BasicData::getxy(double alpha, double delta, 
			double *x, double *y) const
{

  //  setFunctionID("getxy");
  if( cdelt1 == 0.0 || cdelt2 == 0.0 )
    return error("physical coordinates not specified");

  double phi, theta, rt=0.0, x0, y0;
  double ap = crval1*DEG2RAD, deltp = crval2 * DEG2RAD;
  double phip = longpole*DEG2RAD; 
  double ipc001001, ipc001002, ipc002001, ipc002002, det;

  //  *x = 0.0;
  //  *y = 0.0; // omit for speed reasons

  // make the inverted pc matrix:
  det = pc001001*pc002002 - pc002001*pc001002; // determinant of matrix
  if(det == 0.0 )
    return error("error in the PC matrix");

  ipc001001 =  pc002002/det; // inverted PC matrix
  ipc001002 = -pc001002/det;
  ipc002001 = -pc002001/det;
  ipc002002 =  pc001001/det;

  ch_coord( alpha*DEG2RAD, delta*DEG2RAD, ap, phip, deltp, &phi , &theta );

  if( theta != 0.0 ){
    if( projectiontype == RADECTAN ){
      rt = RAD2DEG/tan( theta );
    }
    else{
      if( projectiontype == RADECSIN ){
	rt = RAD2DEG * cos(theta);
      }
      else{
	return error("no ctype keywords specified or not implemented.");
      }

    }//else
  }
  else
    warning("division by zero error!");

  x0 =  rt * sin( phi )/cdelt1;
  y0 = -rt * cos( phi )/cdelt2;

  // Note that the FORTRAN convention is used for pixel numbers
  // Invert to pixel coordinates
  *x = ipc001001 * x0 + ipc001002 * y0 + crpix1;
  *y = ipc002001 * x0 + ipc002002 * y0 + crpix2;

  //  return success();
  return SUCCESS;
}// getxy()


/****************************************************************************/
//
// FUNCTION    : convert_detxy
// DESCRIPTION : Converts an x/y of the WCS of obs to that of the input image.
//               It uses the keyword imgbin for that.
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  obs                I                      initial WCS
//  im                 I                      final   WCS
// *x,*y               I/O                    changes the contents of x/y
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
#define MINDELT 1.0E-7
Status BasicData::convert_detxy(const BasicData &obs, 
				 double *x, double *y) const
{
  setFunctionID("convert_detxy");

  if( getxytype() != DET_XY && 
      (fabs(cdelt1) < MINDELT     || fabs(cdelt2) < MINDELT ||
       fabs(obs.cdelt1) < MINDELT || fabs(obs.cdelt2) < MINDELT ||
       fabs(cdelt1) != fabs(cdelt2) || fabs(obs.cdelt1) != fabs(obs.cdelt2) ) )
    return error("Wrong image coordinate system!");

  double imgbin = (int)(fabs(cdelt1/obs.cdelt1) + 0.5);

  *x = (*x - obs.crpix1)/imgbin + crpix1;
  *y = (*y - obs.crpix2)/imgbin + crpix2;

  return success();
}//




/****************************************************************************/
//
// FUNCTION    : convert_chipxy
// DESCRIPTION : Converts an x/y of the WCS of obs to that of the input image.
//               It uses the keyword imgbin for that.
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  obs                I                      initial WCS
//  im                 I                      final   WCS
// *x,*y               I/O                    changes the contents of x/y
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
#define MINDELT 1.0E-7
Status BasicData::convert_chipxy(const BasicData &obs, const int &ccdnr,
				 const int &xsize, double *x) const
{
  setFunctionID("convert_detxy");

  if( getxytype() != RAW_XY || ccdnr < 0 )
    return error("Wrong image coordinate system!");

    *x = *x + (ccdnr-1)*xsize;

  return success();
}//




/****************************************************************************/
//
// FUNCTION    : convertRGS_XY
// DESCRIPTION : Converts an x/y of the WCS of obs to that of the input image.
//               It uses the keyword imgbin for that.
// 
//
// REMARKS     :
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  obs                I                      initial WCS
//  im                 I                      final   WCS
// *x,*y               I/O                    changes the contents of x/y
//
//  RETURNS    : SUCCESS or FAILURE
//
/****************************************************************************/
#define MINDELT 1.0E-7
Status BasicData::convertRGS_XY( const BasicData &obs,
				 double *x, double *y) const
{
  setFunctionID("convertRGS_XY");

  if( (getxytype() != XMM_RGS_XDSP && getxytype() != XMM_RGS_PI) ||
      cdelt1 == 0.0 || cdelt2 == 0.0)
    return error("wrong image coordinate system!");

  *x = (*x - obs.crpix1)*obs.cdelt1 + obs.crval1; 
  *x = (*x - crval1)/cdelt1 + crpix1;
  if( getxytype() == obs.getxytype() ){
    *y = (*y - obs.crpix2)*obs.cdelt2 + obs.crval2; 
    *y = (*y - crval2)/cdelt2 + crpix2;
  }
  else
    *y = (*y - crval2)/cdelt2 + crpix2;

  return success();
}//

/****************************************************************************/
//
// FUNCTION    : mergeGTI()
// DESCRIPTION : 
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
/****************************************************************************/
Status BasicData::mergeGTI( void ) 
{
  setFunctionID("mergeGTI");

  if( (instrumentID == XMM_RGS1) || (instrumentID == XMM_RGS2 && n_gti > 2) ){
    double *new_start_gti = new double [n_gti];
    double *new_stop_gti  = new double [n_gti];
    float mindiff= 1.0e-2;
    unsigned long new_n_gti=0;

    message("old #gti = ", n_gti);

    new_start_gti[new_n_gti]  = start_gti[0];
    new_stop_gti[new_n_gti++] = stop_gti[0];

    for( unsigned long i = 1; i < n_gti; i++){
      if( start_gti[i] - stop_gti[i-1] < mindiff ){
	new_stop_gti[new_n_gti-1] = stop_gti[i];
      }
      else{
	new_start_gti[new_n_gti]  = start_gti[i];
	new_stop_gti[new_n_gti++] = stop_gti[i];
      }//else
    }//for

    if( new_n_gti == n_gti ){
      delete [] new_start_gti;
      delete [] new_stop_gti;
    }
    else{
      delete [] start_gti;
      delete [] stop_gti;
      start_gti = new_start_gti;
      stop_gti  = new_stop_gti;
      n_gti = new_n_gti;
    }
    message("new #gti = ", n_gti);
  }//if

  return success();

}//mergGTI






/*****************************************************************************/
//
// FUNCTION    : date2juliandate()
// DESCRIPTION : Given the year month day, this function returns the julian 
//               date. It is also possible to give years with decimals.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//  year, month, day    I
//
//
//  RETURNS    :  the julian data
//
/*****************************************************************************/
double date2juliandate( double year, double month, double day)
{

  int leapyear = 0;
  double diffyear, ndays, yr, fracyr;
  double jd = 0.0;

  diffyear = year - REF_YEAR;

  fracyr = year - (int)year;
  if( fracyr != 0.0 && month == 0.0 && day == 0.0 ){
    month = 1.0;
    day   = 1.0;
  }
  else{
    if( fracyr != 0.0 ){
      puts("Invalid year!");
      return 0.0;
    }//if
  }//

  ndays = 0.0;
  ndays += day-1.0;

  if( (int)year % 4 == 0 && 
      !( (int)year % 100 == 0 &&  (int)year % 400 != 0) ){
    leapyear = 1;
    ndays += fracyr * 366.0;
    //printf("%.0f is a leap year.\n",year);
  }
  else{
    ndays += fracyr * 365.0;
    //    printf("%.0f is not a leap year.\n",year);
  }//if else
  
  if( day >= 32.0 || day < 1.0 || month >= 13.0 || month < 1.0 || 
      (
       ( month == 2.0 && ( (day >= 30.0) || (day >= 29.0 && !leapyear)) ) ||
       ( (month == 4.0 || month == 6.0 || month == 9.0 || month == 11.0) && 
	 day >= 31.0) )
      ){
    puts("Invalid day or month!!");
    return 0.0;
  }//if

  for( int i= 1 ; i < month; i++){
    if( i == 1 || i == 3 || i == 5 || i == 7|| i == 8 || i == 10 )
      ndays += 31.0;
    else{
      if( i == 2 )
	ndays += 28.0;
      else
	ndays += 30.0;
    }//if
  }//for
  if(leapyear && month >= 3.0 ) ndays+= 1.0;

  //  printf("%.0f %0.f is the %.0fth day of the year\n",day, month, ndays);

  if( year < REF_YEAR ){
    yr = year;

    while( yr < REF_YEAR ){
      if( (int)yr % 4 == 0 && 
	  !( (int)yr % 100 == 0 &&  (int)yr % 400 != 0) ){
	ndays -= 366.0;
      }
      else{
	ndays -= 365.0;
      }//if
      yr += 1.0;
    }//while

  }
  else{
    yr = REF_YEAR;

    while( yr < year ){
      if( (int)yr % 4 == 0 && 
	  !( (int)yr % 100 == 0 &&  (int)yr % 400 != 0) ){
	ndays += 366.0;
      }
      else{
	ndays += 365.0;
      }//if
      yr += 1.0;
    }//while
  }//if

  jd = ndays + REF_JD + 0.5;


  return jd;

}// date2juliandate()


/*****************************************************************************/
//
// FUNCTION    : jd2date( )
// DESCRIPTION : Converts julian date to a date.
// 
//
// SIDE EFFECTS:
//
// PARAMETERS  :
// 	NAME		I/O			DESCRIPTION
//
//
//  RETURNS    : void
//
/*****************************************************************************/
void jd2date( double jd, double *year, double *month, double *day)
{
  int leapyear=0;
  double nyrs=0.0, ndays=0.0;

  nyrs = (jd-REF_JD)/365.2522; // approx. number of years since REF_YEAR

  *year = (int)(REF_YEAR + nyrs);

  if( (int) *year % 4 == 0 && 
      !( (int) *year % 100 == 0 &&  (int) *year % 400 != 0) ){
    leapyear = 1;
  }//if

  ndays = jd - date2juliandate( *year, 1.0, 1.0) + 1.0;

  *month = 1.0;
  for( int i = 1; i < 12 && ndays >= 29.0 && i == (int)*month; i++){
    if( (i == 1 || i == 3 || i == 5 || i == 7|| i == 8 || i == 10) ){
      if( ndays >= 32.0 ){
	ndays -= 31.0;
	*month += 1.0;
      }//if
    }
    else{
      if( i != 2 ){
	if( ndays >= 31.0){
	  ndays -= 30.0;
	  *month += 1.0;
	}//if
      }
      else{
	if( leapyear ){
	  if( ndays >= 30.0 && leapyear){
	    ndays -= 29.0;
	    *month += 1.0;
	  }//if
	}
	else{
	  if( ndays >= 29.0 ){
	    ndays -= 28.0;
	    *month += 1.0;
	  }//
	}
      }//else
    }//if
  }//for

  *day = ndays;

  if( *month == 12.0 && *day == 32.0 ){
    *year += 1.0;
    *month = 1.0;
    *day = 1.0;
  }else
    if( ndays < 1.0 && *month == 1.0 ){
      *year -= 1.0;
      *month = 12.0;
      *day = 31.0;
    }//if

  return;

}// jd2date




