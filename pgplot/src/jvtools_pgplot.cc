/*****************************************************************************/
/*
*  File        :  jvtools_pgplot.cc
*  AUTHOR      :  Jacco Vink 
*  DESCRIPTION :  C++ pgplot wrapper
*
*
*  COPYRIGHT   : Jacco Vink
*/
/*****************************************************************************/
/* MODIFICATIONS :                                                           
*
*
*/
/*****************************************************************************/

#include "jvtools_pgplot.h"
#include "jvtools_export.h"


using namespace::jvtools;
using JVMessages::error;
using JVMessages::success;
using JVMessages::message;
using JVMessages::warning;
using JVMessages::setFunctionID;



#define STRL 1024


Status calculateTR(const Image *image, float tr[6],
		   float *x1, float *x2, float *y1, float *y2);

//
// constructor
// 
PGPlot::PGPlot()
{
  setDefaults();
  opened = false;
  interactive=false;
  xpanel = ypanel = 1;
  nx = ny = 1;
  histplot=false;
  invertticks=false;
  axisType=PG_BOX_WITH_TWO_AXIS;

  extraXoptions="";
  extraYoptions="";
  
};

PGPlot::~PGPlot()
{
  //  setFunctionID("~PGPlot");
  //  cpgsci( PG_COLOR_DEFAULT );

  close();

  //  success();
}//destructor


//
Status PGPlot::setDefaults()
{
  xmin  = 0.0;
  xmax  = 1.0;
  ymin  = 0.0;
  ymax  = 1.0;
  nx = 1;
  ny = 1;


  devID=0;
  xpanel = ypanel = 1;

  xvp1=0.0;
  xvp2=1.0;
  yvp1=0.0;
  yvp2=1.0;

  xvp1=0.15;
  xvp2=0.85;
  yvp1=0.2;
  yvp2=0.85;

  maxColorIndex = 255;
  useColorMap = 1;
  adjustScales = 0;
  color = PG_COLOR_DEFAULT;
  linestyle = PG_LSTYLE_SOLID;
  axisType = PG_BOXONLY;//_WITH_COORD;
  //  axisType = PG_BOX_WITH_GRID;
  lweight = 5;
  fontSize = 1.4;
  symbolSize = 1.4;
  //  symbolNr =  PGPLOT_SOLIDRHOMBUS;//Ruit
  symbolNr =  PGPLOT_SOLIDCIRCLE;

  overplot = false;
  xlogplot = false;
  ylogplot = false;


  xscaling = 1.0;
  yscaling = 1.0;
  xshift = 0.0;
  yshift = 0.0;
  xLabel = "x";
  yLabel = "y";
  topLabel = "";
  outputFile = "?";

  return SUCCESS;
}// setDefaults()


/****************************************************************************/
//
// FUNCTION    : open()
// DESCRIPTION : opens a new pgplot window and, if present, closes the old one
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
Status PGPlot::open( bool overPlot,
		     bool roundViewPort,
		     float xtick, float ytick, int nxsub, int nysub,
		     bool square)
{

  setFunctionID("open[PGPlot]");

  message("opening PGPlot device");

  if( opened && !overPlot ){

    close();
    //    devID = cpgopen(outputFile.c_str());
  //  cpgbeg (0, outputFile.c_str(), nx, ny);
  }
  else if ( !opened ){
    message("path name length = ",  outputFile.length() );
    if( outputFile.length() > 100 )
      warning("path length probably too long!");
    
    //    devID = cpgopen(outputFile.c_str());

    //    cpgscr(PG_COLOR_LIGHT_GREY,0.9,0.9,0.9);
    //    cpgscr(PG_COLOR_MEDIUM_GREY,0.5,0.5,0.5);
    //    cpgscr(PG_COLOR_DARK_GREY,0.3,0.3,0.3);

  }
  message("Device name = ", outputFile.c_str());
  message("Device ID = ", devID);

  // cpgslw( lweight );
  // cpgsch( fontSize);


  if( roundViewPort && 
      ylogplot && log10(ymax)-log10(ymin) > 1.7){//round to whole decades
    ymax = exp10( ceil(log10(ymax)) );
    ymin = exp10( floor(log10(ymin)) );
  }//if


  //  // cpgsvp(xvp1, xvp2, yvp1, yvp2);
  //  // cpgsvp(0.0,0.2,0.0,0.2);
  //  // cpgqvp (0, &xvp1, &xvp2, &yvp1, &yvp2);
  //  printf("---------> %f %f %f %f\n", xvp1, xvp2,yvp1, yvp2);
  if( ylogplot && (log10(ymax) -  log10(ymin)) > 10){
    // cpgsch( fontSize *0.8);
  }

  if( xlogplot && (log10(xmax) -  log10(xmin)) > 20){
    // cpgsch( fontSize *0.65);
  }else if( xlogplot && (log10(xmax) -  log10(xmin)) > 15){
    // cpgsch( fontSize *0.8);
  }


  if( (nx > 1 || ny > 1)  ){
    message("multiple panels");
    // cpgsubp( nx, ny );
    // cpgpanl(xpanel, ypanel);
    //     cpgpap(10,1.0);
    //     cpgsvp(0.0,0.8,0.0,2.);
    //     cpgsvp(0.0,0.8,0.0,4.);
    //     cpgswin (xmin, xmax*0.9, ymin, 0.7*ymax);
  }

  //// cpgsvp(xvp1, xvp2, yvp1, yvp2);
  if( square==true ){
    // cpgpap(0.0,1.0);
  }
  drawBox(xtick, ytick, nxsub, nysub);


  opened = true;
  return success();
}//
Status PGPlot::close()
{
  if( opened ){
    // cpgend();
    setDefaults();
    opened = false;
  }//
  return SUCCESS;
}



Status PGPlot::drawBox(float xtick, float ytick, int nxsub, int nysub)
{
  setFunctionID("drawBox");

  //  printf("axis type = %i\n",axisType);
  // cpgsvp(xvp1, xvp2, yvp1, yvp2);
  printf("%f %f %f %f\n", xvp1, xvp2, yvp1, yvp2);
  //    // cpgsvp(0.0,0.2,0.0,0.2);
  string xoptions, yoptions;



  switch (axisType){
  case PG_BOX_WITH_COORD:
    if( !xlogplot && !ylogplot){
      //      // cpgenv( xmin, xmax, ymin, ymax, adjustScales, -2);
      // cpgswin(xmin, xmax,  ymin, ymax);
      xoptions="BCTSN";
      yoptions="BCTSN";
      //      // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
      //      // cpgbox("BCTSN", xtick,nxsub, "BCTSN", ytick, nysub);
    }
    else if( !ylogplot && xlogplot ){
      //      // cpgenv( log10(xmin), log10(xmax), ymin, ymax, adjustScales, -2);
      // cpgswin(log10(xmin), log10(xmax),  ymin, ymax);
      xoptions="LBTSN";
      yoptions="BTSN";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
      //      // cpgbox("LBTSN", xtick, nxsub, "BTSN", ytick, nysub);
    }else if( !xlogplot && ylogplot){
      //      // cpgenv( xmin, xmax,log10(ymin), log10(ymax), adjustScales, -2);
      // cpgswin((xmin), (xmax), log10( ymin), log10(ymax));
      xoptions="BTSN";
      yoptions="LBTSN";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
      //      // cpgbox("BTSN", xtick,nxsub, "LBTSN", ytick, nysub);
    }
    else{
      //      // cpgenv( log10(xmin), log10(xmax), log10(ymin), log10(ymax), adjustScales, -2);
      // cpgswin(log10(xmin), log10(xmax), log10( ymin), log10(ymax));
      xoptions="LBTSN";
      yoptions="LBTSN";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
      //      // cpgbox("LBTSN", xtick, nxsub, "LBTSN", ytick, nysub);
    }
    break;
  case PG_BOX_WITH_TWO_AXIS:
    if( !xlogplot && !ylogplot){
      //      // cpgenv( xmin, xmax, ymin, ymax, adjustScales, -2);
      // cpgswin(xmin, xmax,  ymin, ymax);
      //      // cpgbox("BTSN", xtick,nxsub, "BTSN", ytick, nysub);
      xoptions="BTSN";
      yoptions="BTSN";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
    }
    else if( !ylogplot && xlogplot ){
      //      // cpgenv( log10(xmin), log10(xmax), ymin, ymax, adjustScales, -2);
      // cpgswin(log10(xmin), log10(xmax),  ymin, ymax);
      xoptions="LBTSN";
      yoptions="BTSN";
      //      // cpgbox("LBTSN", xtick, nxsub, "BTSN", ytick, nysub);
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
    }else if( !xlogplot && ylogplot){
      //      // cpgenv( xmin, xmax,log10(ymin), log10(ymax), adjustScales, -2);
      // cpgswin((xmin), (xmax), log10( ymin), log10(ymax));
      xoptions="BTSN";
      yoptions="LBTSN";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
      //      // cpgbox("BTSN", xtick,nxsub, "LBTSN", ytick, nysub);
    }
    else{
      //      // cpgenv( log10(xmin), log10(xmax), log10(ymin), log10(ymax), adjustScales, -2);
      // cpgswin(log10(xmin), log10(xmax), log10( ymin), log10(ymax));
      xoptions="LBTSN";
      yoptions="LBTSN";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
      //      // cpgbox("LBTSN", xtick, nxsub, "LBTSN", ytick, nysub);
    }
    break;
  case PG_BOX_WITH_XY_AXIS:
    if( !xlogplot && !ylogplot){
      //// cpgenv( xmin, xmax, ymin, ymax, adjustScales, -2);
      // cpgbox("ATSN", 0.0,0, "ATSN", 0.0, 0);
      // cpgswin(xmin, xmax,  ymin, ymax);
      // cpglab( xLabel.c_str(), yLabel.c_str(), topLabel.c_str());      
      //// cpgbox("ATS", xtick, nxsub, "ATS", ytick, nysub);
      xoptions="ATS";
      yoptions="ATS";
      //// cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
    }
    else if( !ylogplot && xlogplot ){
      //      // cpgenv( log10(xmin), log10(xmax), ymin, ymax, adjustScales, -2);
      // cpgswin( log10(xmin), log10(xmax), ymin, ymax);
      //      // cpgbox("LATSN", xtick, nxsub, "ATSN", ytick, nysub);
      xoptions="LATS";
      yoptions="ATS";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
    }else if( !xlogplot && ylogplot){
      //      // cpgenv( xmin, xmax,log10(ymin), log10(ymax), adjustScales, -2);
      // cpgswin( xmin, xmax,log10(ymin), log10(ymax));
      //      // cpgbox("ATSN", xtick, nxsub, "LATSN", ytick, nysub);
      xoptions="ATS";
      yoptions="LATS";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
    }
    else{
      //      // cpgenv( log10(xmin), log10(xmax), log10(ymin), log10(ymax), adjustScales, -2);
      // cpgswin( log10(xmin), log10(xmax), log10(ymin), log10(ymax));
      //      // cpgbox("LATSN", xtick, nxsub,"LATSN", ytick, nysub);
      xoptions="LATS";
      yoptions="LATS";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
    }

    break;
  case PG_BOX_WITH_X_AXIS:
    if( !xlogplot && !ylogplot){
      //      // cpgenv( xmin, xmax, ymin, ymax, adjustScales, -2);
      //      // cpgbox("ATSN", 0.0,0, "ATSN", 0.0, 0);
      //      // cpgbox("ATS", xtick, nxsub, "BTS", ytick, nysub);
      xoptions="ATS";
      yoptions="BTS";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
    }
    else if( !ylogplot && xlogplot ){
      //      // cpgenv( log10(xmin), log10(xmax), ymin, ymax, adjustScales, -2);
      //      // cpgbox("LATSN", 0.0,0, "BTSN", 0.0, 0);
      xoptions="LATSN";
      yoptions="BTSN";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
    }else if( !xlogplot && ylogplot){
      //      // cpgenv( xmin, xmax,log10(ymin), log10(ymax), adjustScales, -2);
      //      // cpgbox("ATSN", xtick,nxsub, "LBTSN", ytick, nysub);
      xoptions="LATSN";
      yoptions="LBTSN";
      //  cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
    }
    else{
      //       cpgenv( log10(xmin), log10(xmax), log10(ymin), log10(ymax), adjustScales, -2);
      //      cpgbox("LATSN", xtick, nxsub, "LBTSN", ytick, nysub);
      xoptions="LATS";
      yoptions="LBTSN";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
    }
    break;
  case PG_BOX_WITH_Y_AXIS:
    if( !xlogplot && !ylogplot){
      // cpgenv( xmin, xmax, ymin, ymax, adjustScales, -2);
      //      // cpgbox("ATSN", 0.0,0, "ATSN", 0.0, 0);
      //      // cpgbox("ATS", xtick, nxsub, "BTS", ytick, nysub);
      xoptions="";
      yoptions="BTSN";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
    }
    else if( !ylogplot && xlogplot ){
      //      // cpgenv( log10(xmin), log10(xmax), ymin, ymax, adjustScales, -2);
      //      // cpgbox("LATSN", 0.0,0, "BTSN", 0.0, 0);
      xoptions="";
      yoptions="BTSN";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
    }else if( !xlogplot && ylogplot){
      //      // cpgenv( xmin, xmax,log10(ymin), log10(ymax), adjustScales, -2);
      //      // cpgbox("ATSN", xtick,nxsub, "LBTSN", ytick, nysub);
      xoptions="";
      yoptions="LBTSN";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
    }
    else{
      //      // cpgenv( log10(xmin), log10(xmax), log10(ymin), log10(ymax), adjustScales, -2);
      //      // cpgbox("LATSN", xtick, nxsub, "LBTSN", ytick, nysub);
      xoptions="LATS";
      yoptions="LBTSN";
      // // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
    }
    break;
  case PG_BOXONLY:
    if( xlogplot && ylogplot ){
      ;
        // cpgenv( log10(xmin), log10(xmax), log10(ymin), log10(ymax), adjustScales, 		30 );//axisType);
    }else if( xlogplot )
      ;
      // cpgenv( log10(xmin), log10(xmax), ymin, ymax, adjustScales, 10);//axisType);
    else if( ylogplot)
      ;
      // cpgenv( xmin, xmax, log10(ymin), log10(ymax), adjustScales, 20);//axisType);
    else{// temporary solution!
      if( nx > 1 || ny > 1){
	// cpgswin(xmin,xmax,ymin,ymax);
	if( xpanel==1 ){
	  //	  // cpgbox("ABCTSN",xtick,nxsub,"ABCTSN", ytick, nysub);
	  xoptions="ABCTSN";
	  yoptions="ABCTSN";
	}else{
	  //	  // cpgbox("ABCTSN", xtick, nxsub,"ABCTSN",ytick, nysub);
	  xoptions="ABCTSN";
	  yoptions="ABCTSN";
	  //	}else{
	  //	  // cpgbox("ABCTSN",xtick,nxsub,"ABCTSN", ytick, nysub);
	  //	  xoptions="ABCTSN";
	  //	  yoptions="ABCTSN";
	}
	// // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
      }//else
    }
  case PG_NO_BOX:
  default:
    {
      if( !xlogplot && !ylogplot ){
	// cpgswin(xmin,xmax,ymin,ymax);
      }else if( xlogplot && ylogplot ){
	// cpgswin(log10(xmin),log10(xmax),log10(ymin),log10(ymax));
      }
      else if( xlogplot && !ylogplot ){
	// cpgswin(log10(xmin),log10(xmax),ymin,ymax);
      }else if( !xlogplot && ylogplot ){
      // cpgswin(xmin,xmax,log10(ymin),log10(ymax));
      }
      return success();
    }
  }//switch
  if( nx > 1 || ny > 1){
	if( xpanel == 1 )
	  ;
	  // cpglab( xLabel.c_str(), yLabel.c_str(), topLabel.c_str());
	else
	  ;
	  // cpglab( xLabel.c_str(), "", topLabel.c_str());
  }
  else
    ;
    // cpglab( xLabel.c_str(), yLabel.c_str(), topLabel.c_str());
  
  if( xlogplot && (log10(xmax) -  log10(xmin)) > 15)
    ;
    // cpgsch( fontSize );
  if( ylogplot && (log10(ymax) -  log10(ymin)) > 12)
    ;
    // cpgsch( fontSize );
  
  //switch
  if(invertticks){
    xoptions += "I";
    yoptions += "I";
  }

  xoptions += extraXoptions;
  yoptions += extraYoptions;
      
  // cpgbox(xoptions.c_str(), xtick,nxsub, yoptions.c_str(), ytick, nysub);
  return success();
    
}

//
//  setBoundaries
//  plot boundaries
//
Status PGPlot::setBoundaries(float x1, float x2, float y1, float y2)
{
  setFunctionID("setBoundaries");


  if(  x1 == x2 || y1 == y2  ){
    message("xmin = ", x1);
    message("xmax = ", x1);
    message("ymin = ", y1);
    message("ymax = ", y2);
    return error("error in plot boundaries");
  }
  if( (x1 <= 0 && xlogplot) || (y1 <= 0 && ylogplot))
    return error("negative boundaries cannot be combined with log plots");


  xmin = x1;
  xmax = x2;
  ymin = y1;
  ymax = y2;

  return success();
}//setBoundaries()


Status PGPlot::setViewPort(float xleft, float xright, float ybot, float ytop)
{
  setFunctionID("setViewPort");


  // cpgsvp(xleft,xright, ybot, ytop);
  xvp1=xleft;
  xvp2=xright;
  yvp1=ybot;
  yvp2=ytop;

  return success();
}

Status PGPlot::setViewPortCM(float xleft, float xright, float ybot, float ytop)
{
  setFunctionID("setViewPort");
  xleft/=2.54;
  xright/=2.54;
  ybot/=2.54;
  ytop/=2.54;
  // cpgvsiz(xleft,xright, ybot, ytop);
  // cpgqvp(0, &xvp1, &xvp2, &yvp1, &yvp2);
  message("xvp1= ", xvp1);
  message("xvp2= ", xvp2);
  message("yvp1= ", yvp1);
  message("yvp2= ", yvp2);

  return success();
}


//
// setNPanels()
//
Status PGPlot::setNPanels( int m, int n)
{
  setFunctionID("setNPanels");
  if( m < 0 || n <0 || m > 10 || n > 8 )
    return error("value for nx or ny not allowed");

  nx = m;
  ny = n;
  //  // cpgsubp( nx, ny );
  return success();
}//setNPanels

//
// selectPanel
//
Status PGPlot::selectPanel(int i, int j)
{

  setFunctionID("selectPanel");

  if(  i < 1 || i > nx || j < 1 || j > ny )
    return error("panel number not allowed");

  xpanel = i;
  ypanel = j;
  // cpgpanl(xpanel, ypanel);

  message("panels set to ");
  message("           X = ", xpanel);
  message("           Y = ", ypanel);

  return success();
}// selectPanel();

/****************************************************************************/
//
// FUNCTION    : setXlog/setYlog
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
Status PGPlot::setXlog( bool log)
{
  setFunctionID("setXlog");
  if( log )
    message("setting x log plot on");
  else
    message("setting x log plot off");
  xlogplot = log;

  //  setAxisType();

  return SUCCESS;
}
Status PGPlot::setYlog( bool log)
{
  setFunctionID("setYlog");

  if( log )
    message("setting y log plot on");
  else
    message("setting y log plot off");

  ylogplot = log;

  //  setAxisType();

  return SUCCESS;
}//setYlog

Status PGPlot::setAxisType(int axisT)
{
  if(axisT< -2 || axisT > 30 )
    return FAILURE;
  axisType=axisT;
  message("axis type = ", axisType);
  return SUCCESS;
}

/****************************************************************************/
//
// FUNCTION    : setXscaling/yscaling
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
Status PGPlot::setXscaling( double scaling)
{
  setFunctionID("setXscaling");
  
  if( scaling == 0.0 )
    return error("a zero scaling is not allowed");

  xscaling = scaling;
  message("x scaling set to ", xscaling);

  return success();
}//setXscaling
Status PGPlot::setYscaling( double scaling)
{
  setFunctionID("setYscaling");
  
  if( scaling == 0.0 )
    return error("a zero scaling is not allowed");

  yscaling = scaling;
  message("y scaling set to ", yscaling);

  return success();
}//setYscaling


/****************************************************************************/
//
// FUNCTION    : setXshift/setYshift
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
Status PGPlot::setXshift( double shift)
{
  setFunctionID("setXshift");

  xshift=shift;
  message("introduced a x-shift: ", xshift);
  return success();
}
Status PGPlot::setYshift( double shift)
{
  setFunctionID("setXshift");

  yshift=shift;
  message("introduced a y-shift: ", yshift);
  return success();
}

/****************************************************************************/
//
// FUNCTION    : setXlabel/setYlabel/setTopLabel
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
Status PGPlot::setXlabel( const string &label)
{
  xLabel = label;
  return SUCCESS;
}
Status PGPlot::setYlabel( const string &label)
{
  yLabel = label;
  return SUCCESS;
}
Status PGPlot::setLabel( const string &label)
{
  topLabel = label;
  return SUCCESS;
}// 
Status PGPlot::setTopLabel( const string &label)
{
  topLabel = label;
  return SUCCESS;
}// 
Status PGPlot::writeLabel(double x, double y,const string &label )
{
  float xt, yt;
  if( xlogplot)
    xt=(float)log10(xscaling * x + xshift);
  else
    xt=(float)(xscaling * x + xshift);

  if( ylogplot)
    yt = (float)log10(yscaling * y + yshift);
  else
    yt= (float)(yscaling * y) + yshift;

  //  printf("%e %e %e\n", xt, yt, yscaling);

  // cpgsci( color );
  // cpgsch( fontSize );
  // cpgptxt( xt,  yt, 0.0, 0.0, label.c_str());
  // cpgsci( PG_COLOR_DEFAULT );

  return SUCCESS;
}

Status PGPlot::writeLabel(double x, double y,double angle, const string &label )
{
  float xt, yt;
  if( xlogplot)
    xt=(float)log10(xscaling * x + xshift);
  else
    xt=(float)(xscaling * x + xshift);

  if( ylogplot)
    yt = (float)log10(yscaling * y + yshift);
  else
    yt= (float)(yscaling * y + yshift);

  //  printf("%e %e %e\n", xt, yt, yscaling);

  // cpgsci( color );
  // cpgsch( fontSize );
  // cpgptxt( xt,  yt, float(angle), 0.0, label.c_str());
  // cpgsci( PG_COLOR_DEFAULT );

  return SUCCESS;
}

// in screen coordinate
Status PGPlot::writeInsideLabel(float x, float y,const string &label )
{
  // cpgsci( color );
  // cpgsch( fontSize );
  // cpgmtxt("lv", -x, y, 0.0,label.c_str());
  // cpgsci( PG_COLOR_DEFAULT );
  return SUCCESS;
}




// Uses the current line style to make a legend: a line with a label
Status PGPlot::makeLegend(double x1, double x2, double y1, const string & label)
{
  double x[3]={x1,0.5*(x1+x2), x2}, y[3]={y1,y1,y1}, xl;

  if( xlogplot ){
    double length= fabs(log10( x2 ) - log10( x1));
    xl= log10(MAX(x2,x1)) + 0.4 *length;
    //    printf("%f %f\n", log10(x2), xl);
    xl= pow(10.0, xl);
  }
  else{
    double length= fabs(x2  - x1);
    xl= x2 + 0.1 *length;
  }

  //  printf("-- %e %e\n -- %e %e\n -- %e\n", x[0],y[0],x[1],y[1], xl);
  if( x1 != x2 )
    drawCurve( x, y, 3);
  writeLabel( xl, y1, label);

  return SUCCESS;
}

Status PGPlot::makeLegendSymbol(double x, double y, const string & label)
{
  float xp[2], yp[2], xl, yl;
  xp[0]=xp[1]=(float)x;
  yp[0]=yp[1]=(float)y;

  if(xlogplot)
    xl= pow(10.0, log10(x) + 0.02*(log10(xmax)-log10(xmin)));
  else
    xl= x + 0.05*(xmax-xmin);
  yl=y;

  drawPoints( xp,yp, yp,2,false);
  writeLabel( xl, yl, label);

  return SUCCESS;
}

Status PGPlot::drawArrow(double x1, double y1, double x2, double y2, int size)
{

  int oldSize=fontSize;
  if( size != -1 )
    setFontSize(size);

  // cpgsci( color );
  // cpgarro( (float)x1, (float)y1, (float)x2, (float)y2);
  if( size != -1 )
    setFontSize(oldSize);


  return SUCCESS;
}

//
// setColor
//
Status PGPlot::setColor( int colorIndex )
{
  color = colorIndex;
  
  return SUCCESS;
}//setColor
int PGPlot::getColor( )
{
  return  color;
}//setColor

//
// setLineStyle
//
Status PGPlot::setLineStyle( int ls){
  linestyle = ls;
  //  message("line style set to ",linestyle);
  return SUCCESS;
}//setLineStyle
Status PGPlot::setOverPlot( bool op){
  overplot=op;

  return SUCCESS;
}//setLineStyle

/****************************************************************************/
//
// FUNCTION    : drawCurve various versions (automatic conversion to float)
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
Status PGPlot::drawCurve( const double *x, const double *y, int ndata)
{
  setFunctionID("drawCurve[d,d]");
  if( ndata <= 1 )
    return error("invalid number of data points");

  if( !opened )
    open();

  if(xlogplot)
    message("x log");
  if(xlogplot)
    message("y log");
  //  message("y log? = ", ylogplot);

  float *newx = new float [ndata];
  float *newy = new float [ndata];
  bool negNumbers=false;
  if( yscaling != 1.0)
    message("yscaling = ",yscaling);

  for( int i = 0; i < ndata; i++){
    if (xlogplot ){
      if( xscaling*x[i] > 1.0e-45 )
	newx[i] = (float)log10(xscaling * x[i] + xshift);
      else{
	newx[i] = -45.0;
	negNumbers=true;
      }
    }else
      newx[i] = (float)(xscaling * x[i] + xshift);
    


    if (ylogplot ){
      if( yscaling*y[i] + yshift> 0 && isfinite(y[i]*yscaling) )
	newy[i] = (float)log10(yscaling * y[i] + yshift);
      else{
	//	newy[i] = -45.0;
	newy[i] = log10(ymin)-2.0;
	negNumbers=true;
      }
    }else
      newy[i] = yscaling * y[i] + yshift;

  }//for

  // cpgslct(devID);
  // cpgsci( color );
  // cpgsls( linestyle );

  if( histplot )
    ;
    // cpgbin( ndata, newx, newy, 1);
  else
    ;
    // cpgline( ndata, newx, newy);
  // cpgsci( PG_COLOR_DEFAULT  );

  // cpgsls( PG_LSTYLE_SOLID );

  delete [] newx;
  delete [] newy;

  if( negNumbers)
    warning("logarithm of negative number encountered");


  return success();
}//drawCurve()
Status PGPlot::drawCurve( const float *x, const float *y, int ndata)
{
  setFunctionID("drawCurve[f,f]");
  if( ndata <= 0 )
    return error("invalid number of data points");

  float *newx = new float [ndata];
  float *newy = new float [ndata];
  bool negYnumbers=false;
  bool negXnumbers=false;


  //  message("x log? = ", xlogplot);
  //  message("y log? = ", ylogplot);

  for( int i = 0; i < ndata; i++){
    if (xlogplot ){
      if( xscaling*x[i] + xshift > 0 )
	newx[i] = log10(xscaling * x[i] + xshift);
      else{
	newx[i] = -36.0;
	negXnumbers=true;
      }
    }else
      newx[i] = xscaling * x[i];
    
    if (ylogplot ){
      if( yscaling*y[i] + yshift > 0 )
	newy[i] = (float)log10(yscaling * y[i] + yshift);
      else{
	newy[i] = -36.0;
	negYnumbers=true;
      }
    }else
      newy[i] = yscaling * y[i] + yshift;
  }//for

  // cpgslct(devID);
  // cpgsci( color );

  // cpgsls( linestyle );
  if( histplot )
    ;
    // cpgbin( ndata, newx, newy, 1);
  else
    ;
    // cpgline( ndata, newx, newy);

  // cpgsls( PG_LSTYLE_SOLID);
  // cpgsci( PG_COLOR_DEFAULT  );

  delete [] newx;
  delete [] newy;
  
  if( negXnumbers)
    warning("logarithm of negative number encountered in X");
  if( negYnumbers)
    warning("logarithm of negative number encountered in Y");

  return success();
}//drawCurve()

Status PGPlot::drawCurve( const float *x, const long *y, int ndata)
{
  setFunctionID("drawCurve[d,d]");
  if( ndata <= 0 )
    return error("invalid number of data points");

  if( !opened )
    open();
  
  
  float *newx = new float [ndata];
  float *newy = new float [ndata];
  bool negNumbers=false;

  for( int i = 0; i < ndata; i++){
    if (xlogplot ){
      if( xscaling*x[i] + xshift > 1.0e-45 )
	newx[i] = (float)log10(xscaling * x[i] + xshift);
      else{
	newx[i] = -45.0;
	negNumbers=true;
      }
    }else
      newx[i] = (float)(xscaling * x[i]);
    
    if (ylogplot ){
      if( yscaling*y[i] + yshift > 1.0e-45 )
	newy[i] = (float)log10(yscaling * y[i]+ yshift);
      else{
	newy[i] = -45.0;
	negNumbers=true;
      }
    }else
      newy[i] = (float)(yscaling * y[i] + yshift);
  }//for

  // cpgslct(devID);
  // cpgsci( color );

  // cpgsls( linestyle );
  // cpgline( ndata, newx, newy);
  // cpgsci( PG_COLOR_DEFAULT  );

  // cpgsls( PG_LSTYLE_SOLID );

  delete [] newx;
  delete [] newy;

  if( negNumbers)
    warning("logarithm of negative number encountered");


  return success();
}//drawCurve()



/****************************************************************************/
//
// FUNCTION    : drawPoints()
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
Status PGPlot::drawPoints(const double *x, 
			  const double *xerr_pos, const double *xerr_neg,
			  const double *y, 
			  const double *yerr_pos, const double *yerr_neg,
			  int ndata,
			  bool plotErrors){

  float *newx =  new float [ndata];
  float *newy =  new float [ndata];
  float *newxerrpos = new float [ndata];
  float *newxerrneg = new float [ndata];
  float *newerrpos = new float [ndata];
  float *newerrneg = new float [ndata];

  for(int i=0;i<ndata;i++){
    newx[i]=(float)x[i];
    newy[i]=(float)y[i];
    newxerrpos[i]=(float)xerr_pos[i];
    newxerrneg[i]=(float)xerr_neg[i];
    newerrpos[i]=(float)yerr_pos[i];
    newerrneg[i]=(float)yerr_neg[i];
  }
  drawPoints(newx, newxerrpos, newxerrneg,newy,newerrpos,newerrneg,ndata,plotErrors);

  delete [] newx;
  delete [] newy;
  delete [] newxerrneg;
  delete [] newxerrpos;
  delete [] newerrpos;
  delete [] newerrneg;

  return success();
  
}
Status PGPlot::drawPoints(const float *x, 
			  const float *xerr_pos, const float *xerr_neg,
			  const float *y, 
			  const float *yerr_pos, const float *yerr_neg,
			  int ndata,
			  bool plotErrors)
{
  setFunctionID("drawPoints");
  if( ndata <= 0 )
    return error("invalid number of data points");

  if( !opened )
    open();
  
  float *newx =  new float [ndata];
  float *newy =  new float [ndata];
  float *newxerrpos = new float [ndata];
  float *newxerrneg = new float [ndata];
  float *newerrpos = new float [ndata];
  float *newerrneg = new float [ndata];
  bool negNumbers=false;

  for( int i = 0; i < ndata; i++){
    if (xlogplot ){
      if( xscaling*x[i] + xshift > 0 ){
	newx[i] = (float)log10(xscaling * x[i] + xshift);
	newxerrpos[i] = (float)log10( xscaling * (x[i]+ xerr_pos[i]) + xshift);
	newxerrpos[i] -= (float)log10( xscaling * x[i] + xshift);

	newxerrneg[i] = (float)log10( xscaling * x[i] + xshift);
	newxerrneg[i] -= (float)log10( xscaling * (x[i]- xerr_neg[i]) + xshift);

      }else{
	newx[i] = -36.0;
	negNumbers=true;
      }
    }else{
      newx[i] = (float)(xscaling * x[i] + xshift);
      newxerrpos[i] = (float)(xscaling * xerr_pos[i]);
      newxerrneg[i] = (float)(xscaling * xerr_neg[i]);
    }
    
    if (ylogplot ){
      if( yscaling*y[i] + yshift > 0 ){
	newy[i] = (float)log10(yscaling * y[i]);
	newerrpos[i] = (float)log10( yscaling * (y[i]+ yerr_pos[i]) + yshift);
	newerrpos[i] -= (float)log10( yscaling * y[i] + yshift);

	newerrneg[i] = (float)log10( yscaling * y[i] + yshift);
	newerrneg[i] -= (float)log10( yscaling * (y[i]- yerr_neg[i]) + yshift);
      }
      else{
	newy[i]= yscaling*(MAX(y[i],0.0) + 2.0*yerr_pos[i]) + yshift;
	drawUpperLimit(xscaling*x[i], newy[i]);
	newy[i] = -36.0;
	newerrpos[i] = newerrneg[i] = 0.0;
	negNumbers=true;
      }
    }else{//not ylog
      newy[i] = (float)(yscaling * y[i] + yshift);
      newerrpos[i] = yscaling*yerr_pos[i];
      newerrneg[i] = yscaling*yerr_neg[i];
    }
  }//for

  // cpgslct(devID);
  // cpgsci( color );
  // cpgsch( symbolSize);
  message("symbol size = ", symbolSize);
  if( plotErrors )
    message("Drawing error bars");
  else
    message("No error bars");
  //  for( int i = 0; i < ndata ; i++){
    if( plotErrors ){
      // cpgerrb(1, ndata, newx, newy, newxerrpos, 0.0);
      // cpgerrb(3, ndata, newx, newy, newxerrneg, 0.0);
      // cpgerrb(2, ndata, newx, newy, newerrpos, 0.0);
      // cpgerrb(4, ndata, newx, newy, newerrneg, 0.0);
    }
    // cpgpt (ndata, newx, newy, symbolNr);
    //  }
  //PGERRB (DIR, N, X, Y, E, T)
  //  // cpgline( ndata, newx, newy);
  // cpgsci( PG_COLOR_DEFAULT  );
  // cpgsch( fontSize);

  delete [] newx;
  delete [] newy;
  delete [] newxerrneg;
  delete [] newxerrpos;
  delete [] newerrpos;
  delete [] newerrneg;

  if( negNumbers)
    warning("logarithm of negative number encountered");

  return success();
}
Status PGPlot::drawPoints(const float *x, const float *y, 
			  const float *yerror, int ndata,
			  bool plotErrors)
{
  setFunctionID("drawPoints");
  if( ndata <= 0 )
    return error("invalid number of data points");

  if( !opened )
    open();
  
  float *newx =  new float [ndata];
  float *newy =  new float [ndata];
  float *newerrpos = new float [ndata];
  float *newerrneg = new float [ndata];
  bool negNumbers=false;

  for( int i = 0; i < ndata; i++){
    if (xlogplot ){
      if( xscaling*x[i] + xshift > 0 )
	newx[i] = (float)log10(xscaling * x[i] + xshift);
      else{
	newx[i] = -36.0;
	negNumbers=true;
      }
    }else
      newx[i] = (float)(xscaling * x[i]);
    
    if (ylogplot ){
      if( yscaling*y[i] > 0 ){
	newy[i] = (float)log10(yscaling * y[i] + yshift);
	newerrpos[i] = (float)log10( yscaling * (y[i]+ yerror[i]) + yshift);
	newerrpos[i] -= (float)log10( yscaling * y[i] + yshift);

	newerrneg[i] = (float)log10( yscaling * y[i] + yshift);
	newerrneg[i] -= (float)log10( yscaling * (y[i]- yerror[i]) + yshift);
      }
      else{
	newy[i] = -36.0;
	newerrpos[i] = newerrneg[i] = 0.0;
	negNumbers=true;
      }
    }else{
      newy[i] = (float)(yscaling * y[i] + yshift);
      //      printf("newy=%e\n",newy[i]);
      newerrpos[i] = newerrneg[i] = yerror[i]*yscaling;
    }
  }//for

  // cpgslct(devID);
  // cpgsci( color );
  // cpgsch( symbolSize);
  message("symbol size = ", symbolSize);
  if( plotErrors )
    message("Drawing error bars");
  else
    message("No error bars");
  //  for( int i = 0; i < ndata ; i++){
    if( plotErrors ){
      // cpgerrb(2, ndata, newx, newy, newerrpos, 0.0);
      // cpgerrb(4, ndata, newx, newy, newerrneg, 0.0);
    }
    // cpgpt (ndata, newx, newy, symbolNr);
    //  }
  //PGERRB (DIR, N, X, Y, E, T)
  //  // cpgline( ndata, newx, newy);
  // cpgsci( PG_COLOR_DEFAULT  );
  // cpgsch( fontSize);

  delete [] newerrpos;
  delete [] newerrneg;

  if( negNumbers)
    warning("logarithm of negative number encountered");

  return success();
}
Status PGPlot::drawPoints( const double *x, const double *y, 
			   const double *yerror, int ndata,
			   bool plotErrors)
{
  setFunctionID("drawPoints");
  if( ndata <= 0 )
    return error("invalid number of data points");

  if( !opened )
    open();
  
  float *newx = new float [ndata];
  float *newy = new float [ndata];
  float *errors= new float [ndata];

  double xs=xscaling;
  double ys=yscaling;
  double xsh=xshift;
  double ysh=yshift;

  for( int i = 0; i < ndata; i++){
    newx[i]=(float)(x[i]*xscaling + xshift);
    newy[i]=(float)(y[i]*yscaling + yshift);
    errors[i]=(float)(yerror[i]*yscaling);
  }
  xscaling=1.0;
  yscaling=1.0;
  xshift=0.0;
  yshift=0.0;
  drawPoints(newx, newy,  errors, ndata, plotErrors);
  xscaling=xs;
  yscaling=ys;
  xshift=xsh;
  yshift=ysh;
  
  delete [] newx;
  delete [] newy;
  delete [] errors;

  return success();
}

/****************************************************************************/
//
// FUNCTION    : drawUpperLimit
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
Status PGPlot::drawUpperLimit( double x, double y)
{
  setFunctionID("drawUpperLimit");


  if( !opened )
    open();
  
  if (xlogplot ){
    if( x < 0 )
      return error("x < 0 , not allowed for log plot");
    x = log10(xscaling * x + xshift);
  }

    
  if (ylogplot ){
    if( y < 0 )
      return error("y < 0 , not allowed for log plot");
    y = log10( yscaling * y + yshift);
  }


  // cpgslct(devID);
  // cpgsci( color );
  //  // cpgsci( PG_COLOR_GREEN );
  // cpgsch( symbolSize) ;

  float x0=x, y0=y, size;
  if( ylogplot)
    size=0.2*(log10(ymax)-log10(ymin));
  else
    size=0.2*(ymax - ymin);

  // cpgpt (1, &x0, &y0, -4);

  // cpgsch( 0.75*symbolSize) ;

  // cpgsah(1, 45.0, 0.3);
  // cpgarro( x0, y0, x0, y0-size);
  // cpgsch( symbolSize) ;

  //PGERRB (DIR, N, X, Y, E, T)
  //  // cpgline( ndata, newx, newy);
  // cpgsci( PG_COLOR_DEFAULT  );
  // cpgsch( fontSize);


  return success();
}// 

/****************************************************************************/
//
// FUNCTION    : displayImage
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
Status PGPlot::displayImage(const Image *image, bool drawWedge, 
			    ImageScaling scaling,
			    int i, int j, int zoom)
{
  setFunctionID("displayImage");

  Image imageCopy= *image;
  if( scaling != SCAL_LIN){
    switch (scaling){
      case SCAL_SQRT:
	imageCopy.squareroot();
	break;
      case SCAL_LOG:
	imageCopy.logarithm();
	break;
      case SCAL_SQR:
	imageCopy *= imageCopy;
	break;
      default:;
      }//switch
  }//if

  //  int nx=imageCopy.nx;
  //  int ny=imageCopy.ny;
  int i0=1, i1=imageCopy.nx;
  int j0=1, j1=imageCopy.ny;
  int wedgeSep=2, wedgeSize=2;
  float min = imageCopy.minimum();
  float max = imageCopy.maximum();
  float tr[6];
  float x1, x2, y1, y2;

  
  calculateTR( &imageCopy, tr, &x1 ,&x2,&y1, &y2);

#if 0
  if( nx != ny){
    if( nx > ny )
      y2a = y1a + ny/nx * (x2a-x1a);
    else
      x2a =  x1a + nx/ny * (y2a-y1a);
  }

  //  // cpgsvp(x1a/XSIZE, x2a/XSIZE, y1a/YSIZE, y2a/YSIZE);    
  //  // cpgvsiz( x1a, x2a, y1a, y2a );

#endif
  message("nx = ", imageCopy.nx);
  message("ny = ", imageCopy.ny);
  if( zoom != 1 && zoom > 0 ){
    int newnx, newny;
    newnx=imageCopy.nx/zoom;
    newny=imageCopy.nx/zoom;
    i0= i-newnx/2;
    j0= i-newny/2;
    i1= i + newnx/2;
    j1= i + newnx/2;
    //    printf("%i %i %i %i\n",i0,i1,j0,j1);
  }
  //  else
    ///    

  setBoundaries( x1, x2, y1, y2);
// setBoundaries( (float)i0, (float)i1, (float)j0, (float)j1);
  //  adjustScales = 1;
  //  axisType = -1;
  //  setXlabel("");
  //  setYlabel("");

  //  setenv("PGPLOT_ENVOPT","I",1);

  if( !opened) 
    open();

  generateColorMap( useColorMap );

  // cpgwnad(x1,x2,y1,y2);
  // cpgimag (imageCopy.pixel, imageCopy.nx, imageCopy.ny, i0, 	   i1, j0, j1,  min, max, tr);
  

  if( drawWedge )
    ;
    // cpgwedg("LI", wedgeSep, wedgeSize, min, max, "");

  adjustScales = 0;

  return success();
}//displayImage()

/****************************************************************************/
//
// FUNCTION    : readCursor( )
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
char PGPlot::readCursor( float *x, float *y)
{
  setFunctionID("readCursor");
  char returnChar;

#if 0
  if( cpgcurs( x, y, &returnChar) == 0)
    error("an error occurred");
  else
    success();
#endif
  return returnChar;
}

/****************************************************************************/
//
// FUNCTION    : generateColorMap
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
Status PGPlot::generateColorMap( int colorMap)
{
  setFunctionID("generateColorIndex");
  

  int icilo, icihi;
  int firstIndex = 16, indexRange;

  // cpgqcir( &icilo, &icihi);
  //  printf("%i %i\n", icilo, icihi);
  maxColorIndex = icihi;

  message("generating color map ", colorMap);
  message("start  color  index = ", firstIndex);
  message("maximum color index = ", maxColorIndex);
  indexRange = maxColorIndex - firstIndex;

  float index, red=0.0, green=0.0, blue=0.0;

  if( colorMap == 1 ){

    for( int i = firstIndex; i <= maxColorIndex; i++){

      //      index = (float)i - 1.0;
      index = (float)i -  firstIndex;
      if(  index < (float)indexRange /3.0 )
	red = index * (3.0/indexRange);
      else{
	red = 1.0;
	if(  index < (float)indexRange * 2.0/3.0 )
	  green = (index - indexRange/3.0) * (3.0/indexRange);
	else{
	  green = 1.0;
	  blue = (index - 2.0 * indexRange/3.0) * (3.0/indexRange);
	}
      }//else

      red   = MAX(0.0, red);
      green = MAX(0.0, green);
      blue  = MAX(0.0, blue);
      red   = MIN(1.0, red);
      green = MIN(1.0, green);
      blue  = MIN(1.0, blue);

      // cpgscr( i, red, green, blue);
    }//for

  }else if( colorMap == 2 ){

    for( int i = firstIndex; i <= maxColorIndex; i++){
      //      index = (float)i - 1.0;
      index = (float)i - firstIndex;
      // red
      if( index <  0.25 * indexRange )
	red = 0.0;
      else if( index <  0.5 * indexRange )
	red = (index - 0.25 * indexRange)/(0.25*indexRange);
      else
	red = 1.0;

      //green
      if( index <  0.5 * indexRange )
	green = 0.0;
      else if( index <  0.75 * indexRange )
	green = (index - 0.5 * indexRange)/(0.25*indexRange);
      else
	green = 1.0;

      //blue
      if( index <  0.25 * indexRange )
	blue = (index  - 1.0)/(0.25*indexRange);
      else if( index <  0.5 * indexRange )
	blue = 1.0 - (index - 0.25 * indexRange)/(0.25*indexRange);
      else if( index <  0.75 * indexRange )
	blue = 0.0;
      else 
	blue = (index - 0.75 * indexRange)/(0.25*indexRange);


      red   = MAX(0.0, red);
      green = MAX(0.0, green);
      blue  = MAX(0.0, blue);
      red   = MIN(1.0, red);
      green = MIN(1.0, green);
      blue  = MIN(1.0, blue);
      //      printf("%i %f %f %f\n",i,red, green, blue);
      // cpgscr( i, red, green, blue);
    }//for

  }else if( colorMap == 3 ){

    for( int i = firstIndex; i <= maxColorIndex; i++){
      //      index = (float)i - 1.0;
      index = ((float)i - firstIndex)/indexRange;

      // red has period
      red = SQR(sin( 2.25 * index * 2.0 * M_PI) );

      //green
      green = SQR(sin( 1.25 * index * 2.0 * M_PI) );

      //blue has period 1/4
      blue = SQR( sin( 0.25 * index * 2.0 * M_PI) );


      red   = MAX(0.0, red);
      green = MAX(0.0, green);
      blue  = MAX(0.0, blue);
      red   = MIN(1.0, red);
      green = MIN(1.0, green);
      blue  = MIN(1.0, blue);
      //      printf("%i %f %f %f\n",i,red, green, blue);
      // cpgscr( i, red, green, blue);
    }//for
  }else if( colorMap == 4 ){

    for( int i = firstIndex; i <= maxColorIndex; i++){
      //      index = (float)i - 1.0;
      index = ((float)i - firstIndex)/indexRange;

      // red has period
      red = SQR( sin( 0.25 * index * 2.0 * M_PI) );

      //green
      green = SQR(sin( 1.25 * index * 2.0 * M_PI) );

      //blue has period 1/4
      blue = SQR(sin( 2.25 * index * 2.0 * M_PI) );


      red   = MAX(0.0, red);
      green = MAX(0.0, green);
      blue  = MAX(0.0, blue);
      red   = MIN(1.0, red);
      green = MIN(1.0, green);
      blue  = MIN(1.0, blue);
      //      printf("%i %f %f %f\n",i,red, green, blue);

      //      printf("%i %f %f %f %f\n",i, index, red, green, blue);
      // cpgscr( i, red, green, blue);
    }//for
  }else if( colorMap == 5 ){

    for( int i = firstIndex; i <= maxColorIndex; i++){
      //      index = (float)i - 1.0;
      index = ((float)i - firstIndex)/indexRange;

      // red has period
      red = SQR(sin( 1.25 * index * 2.0 * M_PI) );

      //green
      green = SQR( sin( 0.25 * index * 2.0 * M_PI) );

      //blue has period 1/4
      blue = SQR(sin( 2.25 * index * 2.0 * M_PI) );


      red   = MAX(0.0, red);
      green = MAX(0.0, green);
      blue  = MAX(0.0, blue);
      red   = MIN(1.0, red);
      green = MIN(1.0, green);
      blue  = MIN(1.0, blue);
      //      printf("%i %f %f %f\n",i,red, green, blue);

      //      printf("%i %f %f %f %f\n",i, index, red, green, blue);
      // cpgscr( i, red, green, blue);
    }//for
  }



  // cpgscir( firstIndex, indexRange );
  return success();
}//generateColorIndex()

/****************************************************************************/
//
// FUNCTION    : 
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
Status PGPlot::contourPlot( const Image *image, int nc, int scaling)
{ 
  setFunctionID("contourPlot");

  float min, max;
  min = image->minimum();
  max = 0.90 * image->maximum();
  //  message("mininmum = ", min);
  //  message("maximum = ", max);

  contourPlot( image,min,max, nc, scaling);

  return success();
}
// version with min max

Status PGPlot::contourPlot( const Image *image, float min, float max, int nc, 
			    int scaling, 
			    bool plotlabels, bool useImageBoundaries,
			    float *customLevels)
{ 
  setFunctionID("contourPlot");

  if( nc <= 0 )
    return error("error in number of contour levels, nc = ", nc);

  //  float min, max, dl;
  float dl;
  // float level;
  float *contourLevels = new float [nc];
  float tr[6];

  tr[2] = tr[4] = 0.0;
  tr[0] = tr[3] = 0.0;
  tr[1] = 1.0;
  tr[5] = 1.0;

  // only rectangular without rotation works

  float x1, x2, y1, y2;

#if 0
  int nx, ny;
  float crpix1, crpix2, crval1, crval2, cdelt1, cdelt2, crota2, equinox;
  image->getCoordinates( &crval1, &crval2, &crpix1, &crpix2, 
			 &cdelt1, &cdelt2, 
			 &crota2, &equinox);
  image->getnxny(&nx, &ny);
  if(nx <= 0 && ny <= 0)
    return error("not a valid image");


  tr[0]=crval1 - (crpix1-1.)*cdelt1;
  tr[1]=cdelt1;
  tr[2]=0.0;
  tr[3]=crval2 - (crpix2-1.)*cdelt2;
  tr[4]=0.0;
  tr[5]=cdelt2;

  x1=tr[0]+1*tr[1];
  x2=tr[0]+nx*tr[1];
  y1=tr[3]+1*tr[5];
  y2=tr[3]+ny*tr[5];
#endif

  message("mininmum = ", min);
  message("maximum = ", max);


  calculateTR( image, tr, &x1 ,&x2,&y1, &y2);

  message("x1 = ", x1);
  message("x2 = ", x2);

  message("y1 = ", y1);
  message("y2 = ", y2);

  switch (scaling){
  case PG_SCALE_SQRT:
    dl = sqrt(max - min)/nc;
    for( int i = 0; i < nc; i++)
      contourLevels[i] = min + SQR( (i) * dl );
    break;
  case PG_SCALE_LOG:
    dl = log(max - min)/nc;
    for( int i = 0; i < nc; i++)
      contourLevels[i] = min + exp( (i) * dl );
    break;
  case PG_SCALE_LIN:
    dl = (max - min)/nc;
    for( int i = 0; i < nc; i++)
      contourLevels[i] = min + (i) * dl;
    break;
  case PG_CUSTOM_CONT:
    for( int i = 0; i < nc; i++)
      contourLevels[i] = customLevels[i];
    break;
  default:
    dl = (max - min)/nc;
    for( int i = 0; i < nc; i++)
      contourLevels[i] = min + (i) * dl;
    break;
  };
  message("contour levels:");
  for(int i=0; i<nc ;i++)
    message("level ", contourLevels[i]);

  if( !opened){
    if( useImageBoundaries)
      setBoundaries(x1, x2, y1, y2);
    //    setBoundaries(1, nx, 1, ny);
    //    adjustScales = 1;
    //    axisType = -1;
    open();
  }

  //  // cpgsci ( color );  
  // cpgsci ( color);  
  // cpgcont( image->pixel, image->nx, image->ny, 1, image->nx, 1, image->ny,	   contourLevels, nc, tr );

  if( plotlabels ){
    char label[STRL];
    for(int i=0; i< nc; i++){
      if( (fabs(contourLevels[i]) > 9.9e3 ||  fabs(contourLevels[i]) < 1e-3 ) &&
	  contourLevels[i]!= 0.0)
	sprintf(label,"%.3e",contourLevels[i]);
      else
	sprintf(label,"%.3f",contourLevels[i]);
      message(label);
      int intval=40*image->nx/64;
      int minint=10*image->nx/64;
      // cpgconl( image->pixel, image->nx, image->ny, 1, image->nx, 1, image->ny,      contourLevels[i], tr, label, intval, minint  );
    }
  }
  // cpgsci ( PG_COLOR_DEFAULT  );


  delete [] contourLevels;
  
  return success();
}

/****************************************************************************/
//
// FUNCTION    : contourPlot
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
Status PGPlot::contourPlot( double *data, int nx, int ny, 
			    double x0, double y0,
			    double dx, double dy,
			    double *levels, int nc)
{
  float *data2= new float [nx*ny];
  float *levels2=new float [nc];
  int n=nx*ny;
  for( int i=0; i < n; i++){
    data2[i]=(float)data[i];
  }
  for(int i=0; i<nc;i++)
    levels2[i]=(float)levels[i];

  if( contourPlot( data2, nx, ny, (float)x0, (float)y0, 
		   (float)dx, (float)dy, levels2, nc)==FAILURE)
    return FAILURE;
  else
    return SUCCESS;
}//double version

Status PGPlot::contourPlot( float *data, int nx, int ny, 
			    float x0, float y0,
			    float dx, float dy,
			    float *levels, int nc)
{
  setFunctionID("contourPlot");

  float tr[6];

  tr[2] = tr[4] = 0.0; // 0 for rectangular coordinates
  tr[0] = x0 - dx;
  tr[3] = y0 - dy;
  tr[1] = dx;
  tr[5] = dy;


  // cpgslw( lweight );
  //#  // cpgscr(PG_COLOR_WHITE, 1.0, 1.0, 1.0);

  //  // cpgsci( color );
  // cpgsls( linestyle );
  
  // cpgcont( data, nx, ny, 1, nx, 1, ny,  levels, -nc, tr );

  // cpgsls( 0 );
  // cpgsci ( PG_COLOR_DEFAULT  );



  return success();
}

/****************************************************************************/
//
// FUNCTION    : 
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
char PGPlot::getCursor(float *x, float *y, int mode, bool showResult)
{
  setFunctionID("getCursor");
  
  int posn=0;
  char ch;
  // cpgband( mode, posn,  *x, *y, x, y, &ch);

  if( showResult ){
    printf("ch=%c, x=%e y=%e\n",ch, *x, *y);
  }
  success();

  return ch;
}



// world coordinate converter
Status calculateTR(const Image *image, float tr[6], float *x1 ,float *x2,
		   float *y1, float *y2)
{


  int nx, ny;
  float crpix1, crpix2, crval1, crval2, cdelt1, cdelt2, crota2, equinox;
  image->getCoordinates( &crval1, &crval2, &crpix1, &crpix2, 
			 &cdelt1, &cdelt2, 
			 &crota2, &equinox);

  tr[0]=crval1 - (crpix1-1.)*cdelt1;
  tr[1]=cdelt1;
  tr[2]=0.0;
  tr[3]=crval2 - (crpix2-1.)*cdelt2;
  tr[4]=0.0;
  tr[5]=cdelt2;

  image->getnxny(&nx, &ny);
  *x1=tr[0]+1*tr[1];
  *x2=tr[0]+nx*tr[1];
  *y1=tr[3]+1*tr[5];
  *y2=tr[3]+ny*tr[5];

  return SUCCESS;
}


Status PGPlot::resetXlabelsOptions()
{
  extraXoptions="";
  return SUCCESS;
}


Status PGPlot::resetYlabelsOptions()
{
  extraYoptions="";
  return SUCCESS;
}

Status PGPlot::setYlabelsHorizontal()
{
  extraYoptions+="V";
  return SUCCESS;
}
