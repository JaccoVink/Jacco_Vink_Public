#ifndef __JVTOOLS_PGPLOT_H__
#define __JVTOOLS_PGPLOT_H__

/*****************************************************************************/
/*
*  FILE        :  jvtools_pgplot.h
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

#include "jvtools_image.h"
#include "cpgplot.h"

#define PG_COLOR_DEFAULT   1
#define PG_COLOR_BLACK   0
#define PG_COLOR_BACKGROUND   0
#define PG_COLOR_WHITE   1
#define PG_COLOR_RED   2
#define PG_COLOR_GREEN 3
#define PG_COLOR_BLUE  4
#define PG_COLOR_CYAN  5
#define PG_COLOR_MAGENTA  6
#define PG_COLOR_YELLOW 7
#define PG_COLOR_ORANGE 8
#define PG_COLOR_LIGHTBLUE  11
#define PG_COLOR_PURPLE  12


#define PG_COLOR_LIGHT_GREY 16
#define PG_COLOR_MEDIUM_GREY 17
#define PG_COLOR_GREY 17
#define PG_COLOR_DARK_GREY 18

#define PGPLOT_COLOR_DEFAULT   1
#define PGPLOT_BLACK   0
#define PGPLOT_BACKGROUND   0
#define PGPLOT_WHITE   1
#define PGPLOT_RED   2
#define PGPLOT_GREEN 3
#define PGPLOT_BLUE  4
#define PGPLOT_CYAN  5
#define PGPLOT_MAGENTA  6
#define PGPLOT_LIGHTBLUE  11
#define PGPLOT_PURPLE  12

#define PGPLOT_YELLOW 7
#define PGPLOT_ORANGE 8
#define PGPLOT_XXX 12


#define PGPLOT_LIGHT_GREY 16
#define PGPLOT_MEDIUM_GREY 17
#define PGPLOT_DARK_GREY 18



#define PG_LSTYLE_SOLID     1
#define PG_LSTYLE_DASHED    2
#define PG_LSTYLE_DOTDASHED 3
#define PG_LSTYLE_DOTTED    4
#define PG_LSTYLE_DASHEDDOTDOTDOT 5
#define PGPLOT_SOLID     1
#define PGPLOT_DASHED    2
#define PGPLOT_DOTDASHED 3
#define PGPLOT_DOTTED    4
#define PGPLOT_DASHEDDOTDOTDOT 5


#define PGPLOT_SQUARE 0
#define PGPLOT_DOT 1
#define PGPLOT_PLUS 2
#define PGPLOT_STAR 3
#define PGPLOT_CIRCLE 4
#define PGPLOT_X 5
#define PGPLOT_TRIANGLE 7
#define PGPLOT_PLUSCIRCLE 8
#define PGPLOT_SOLIDTRIANGLE -3
#define PGPLOT_SOLIDRHOMBUS -4
#define PGPLOT_SOLIDSQUARE 16
#define PGPLOT_SOLIDCIRCLE 17



#define PG_NO_BOX -1
#define PG_BOXONLY 0
#define PG_BOX_WITH_COORD 1
#define PG_BOX_WITH_GRID  2
#define PG_BOX_WITH_TWO_AXIS 5
#define PG_BOX_WITH_XY_AXIS 6
#define PG_BOX_WITH_X_AXIS 7
#define PG_BOX_WITH_Y_AXIS 8
#define PG_BOX_WITH_XLOG  10
#define PG_BOX_WITH_YLOG  20
#define PG_BOX_WITH_XYLOG 30

#define PG_SCALE_LIN  1
#define PG_SCALE_SQRT 2
#define PG_SCALE_LOG  3
#define PG_CUSTOM_CONT 4


#define PG_COLORMAP_B 1
#define PG_COLORMAP_BB 2
#define PG_COLORMAP_RAINBOW 3
 
namespace jvtools{

  //  class Image;
  
  class PGPlot{
  public:
    PGPlot();
    ~PGPlot();
    Status setDefaults();
    Status open( bool overPlot = false, bool roundViewPort = true,
		 float xtick=0.0, float ytick=0.0,
		 int nxsub=0, int nysub=0, bool square=false);
    Status close();
    Status drawBox(float xtick=0.0, float ytick=0.0, int nxsub=0, int nysub=0);
    Status toggleInteractive(){ 
      if( interactive==true){
	cpgask(0);
	interactive=false;
      }
      else{
	cpgask(1);
	interactive=true;
      }
      return SUCCESS;
    };
    Status toggleInvertTicks(){
	invertticks= !invertticks;
	if( invertticks)
	  JVMessages::message("tick marks will be drawn outside the box/axes");
	return SUCCESS;
    }
    Status setNPanels( int m, int n);
    Status getNPanels( int *m, int *n){
      *m = nx;
      *n = ny;
      return SUCCESS;
    };
    Status selectPanel(int i, int j);
    Status setBoundaries(float x1, float x2, float y1, float y2);
    Status setViewPort(float xleft, float xright, float ybot, float ytop);
    Status setViewPortCM(float xleft, float xright, float ybot, float ytop);
    float getXmax(){return xmax;};
    float getYmax(){return ymax;};
    float getXmin(){return xmin;};
    float getYmin(){return ymin;};
    Status setXlabel( const string &label);
    Status setYlabel( const string &label);
    Status setAxisType(int axisT);
    int getDeviceID(){return devID;};
    int getAxisType(){return axisType;};

    Status drawXYaxis(){
      //      cpgenv( xmin, xmax, ymin, ymax, adjustScales, -2);
      cpgswin (xmin, xmax, ymin, ymax);
      cpgsls( PG_LSTYLE_SOLID );
      if( !xlogplot && !ylogplot)
	cpgbox("BCTSN", 0.0,0, "BCTSN", 0.0, 0);
      else if( !ylogplot )
	cpgbox("LBTSN", 0.0,0, "BTSN", 0.0, 0);
      else if( !xlogplot )
	cpgbox("BTSN", 0.0,0, "BLNTSN", 0.0, 0);
      else
	cpgbox("LBCTSN", 0.0,0, "LBCTSN", 0.0, 0);
      return SUCCESS;
    };// separate open does same)

    Status setLabel( const string &label);
    Status setTopLabel( const string &label);
    Status setXlog( bool log=true);
    Status setYlog( bool log=true);
    Status setXscaling( double scaling);
    Status setYscaling( double scaling);
    Status setXshift( double shift);
    Status setYshift( double shift);
    Status setColor( int colorIndex );
    int getColor( );
    Status setFontSize( float size ){
      fontSize = size;   
      cpgsch( fontSize);
      return SUCCESS;
    };
    Status setSymbolSize( float size ){symbolSize = size; return SUCCESS;};
    Status setSymbolNumber( int n ){symbolNr=n; return SUCCESS;};
    Status setLineWeight( int lw ){lweight = lw; return SUCCESS;};
    int getLineWeight( ){return lweight;};
    Status setLineStyle( int ls);
    int getLineStyle( ){return linestyle;};
    Status setOverPlot( bool op=true);
    Status toggleHistogram(){histplot= !histplot; return SUCCESS;};
    Status setHistogram( bool useHist=false){ histplot=useHist; 
    return SUCCESS;};
    Status setPlotFile( const char plotFileName[] ){ 
      outputFile = plotFileName; return SUCCESS;};
    Status drawCurve( const float *x, const float *y, int ndata);
    Status drawCurve( const float *x, const long *y, int ndata);
    Status drawCurve( const double *x, const double *y, int ndata);
    Status drawSpline( const double *x, const double *y, int ndata, int k=5);
    inline Status drawLine(float x1, float y1, float x2, float y2){
      float x[2]={x1,x2};
      float y[2]={y1,y2};
      return drawCurve(x,y,2);
    }

    Status drawPoints(const double *x, 
		      const double *xerr_pos, const double *xerr_neg,
		      const double *y, 
		      const double *yerr_pos, const double *yerr_neg,
		      int ndata,
		      bool plotErrors=true);
    Status drawPoints(const float *x, 
		      const float *xerr_pos, const float *xerr_neg,
		      const float *y, 
		      const float *yerr_pos, const float *yerr_neg,
		      int ndata,
		      bool plotErrors=true);
    Status drawPoints( const float *x, const float *y, 
		       const float *yerror, int ndata,
		       bool plotErrors=true);
    Status drawPoints( const double *x, const double *y, 
		       const double *yerror, int ndata,
		       bool plotErrors=true);
    Status drawUpperLimit( double x, double y);
    Status writeLabel(double x, double y,const string &label );
    Status writeLabel(double x, double y, double angle, const string &label );
    Status writeXlabel(){
      cpglab(xLabel.c_str(), "","");
      return SUCCESS;
    };
    Status writeYlabel(){
      cpglab("", yLabel.c_str(), "");
      return SUCCESS;
    };
    Status writeTopLabel(){
      cpglab("","",topLabel.c_str());
      return SUCCESS;
    };
    Status writeInsideLabel(float x, float y,const string &label );
    Status makeLegend(double x1, double x2, double y, const string &label);
    Status makeLegendSymbol(double x, double y, const string &label);
    Status drawArrow(double x1, double y1, double x2, double y2, int size=-1);

    bool isOpen(){ return opened;};
    
    Status contourPlot( float *data, int nx, int ny, 
			float x0, float y0,
			float dx, float dy,
			float *levels, int nc);
    Status contourPlot( double *data, int nx, int ny, 
			double x0, double y0,
			double dx, double dy,
			double *levels, int nc);

    // images:
    
    Status displayImage( const Image *image, bool drawWedge=true,
			 ImageScaling scaling=SCAL_LIN,
			 int i=-1, int j=-1, int zoom=1);
    
    char readCursor( float *x, float *y);
    Status contourPlot( const Image *image, int nc, 
			int scaling=PG_SCALE_SQRT);
    Status contourPlot( const Image *image, float min, float max, int nc=5, 
			int scaling=PG_SCALE_SQRT, bool plotlabels=true,
			bool useImageBoundaries=true,
			float *customLevels=0);
    Status generateColorMap( int colorMap=1);
    Status setColorMap( int colorMap){
      useColorMap= colorMap;
      return SUCCESS;};
    char getCursor(float *x, float *y, int mode=7, bool showResult=false );
    Status resetXlabelsOptions();
    Status resetYlabelsOptions();
    Status setYlabelsHorizontal();
  protected:
  private:
    string outputFile;
    string xLabel, yLabel, topLabel;
    string extraXoptions, extraYoptions;
    int nx, ny, xpanel, ypanel, devID, adjustScales;
    int color, linestyle, axisType, lweight, symbolNr;
    int maxColorIndex, useColorMap;
    float fontSize, symbolSize;
    bool opened, interactive;
    bool xlogplot, ylogplot, histplot, overplot, invertticks;
    double xscaling, yscaling, xshift, yshift;
    float xmin, xmax, ymin, ymax, xvp1, xvp2, yvp1, yvp2;


    void setAxisType(){
      if( axisType != 5 ){
	if( xlogplot && ylogplot )
	  axisType = PG_BOX_WITH_XYLOG;
	else 
	  if( xlogplot )
	    axisType = PG_BOX_WITH_XLOG;
	  else 
	    if( ylogplot )
	      axisType = PG_BOX_WITH_YLOG;
	    else
	      axisType = PG_BOXONLY;
      }
      return;
    };

  };
};
#endif
