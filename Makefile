###############################################################################
#
#
# FILE   : Makefile
# AUTHOR : Jacco Vink
#
# 
###############################################################################


###############################################################################
#  
# 	Some variables :
#
###############################################################################

include Makefile_dirs.in


TODAY = `date +"%Y_%m_%d"`

###############################################################################
#
#
#
###############################################################################

all: libs tools pca



clean:
	cd lib && rm -f *.a;
	cd base/src &&	make clean;
	cd pgplot/src &&	make clean;
	cd imaging/src && make clean;
	cd pca_eigen/src && make clean;
	cd bin && rm -f *;


libs:	
	make clean
	cd base/src &&	make basiclib;
	cd imaging/src && make imagelib;
	cd pgplot/src && make jvtools_pgplotlib;


tools:
	cd imaging/src && make imagetools;


pca:
	cd pca_eigen/src && make pcaim;





