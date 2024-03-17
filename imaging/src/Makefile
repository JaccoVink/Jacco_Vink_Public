##############################################################################
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

include ../../Makefile_dirs.in




IMAGE_LIB_OBJ = image.o image_io.o image2.o image3.o region.o region_io.o





###############################################################################
# 
# Make the astro_x library 
#
###############################################################################




imagelib: $(IMAGE_LIB_OBJ)
	$(AR) $(LIB_DIR1)/libjvtools_image.a $(IMAGE_LIB_OBJ)
	$(AR2) $(LIB_DIR1)/libjvtools_image.a

imagelibsh:
	g++ -shared -Wl,-soname,libjvtools_image.so -o libjvtools_image.so\
	$(IMAGE_LIB_OBJ)
	mv libjvtools_image.so $(LIB_DIR2)


###############################################################################
# 
# 
#
###############################################################################

clean:
	$(RM) *.o

imagetools: addim clipim markim multiplyim rebinim regionim subtractim




################################################################################
#
# 	In the next block executables are made:
#
################################################################################

#
#
#
addim: addim.cc 
	$(CPP) $(CCFLAGS) -o $(BIN)/addim addim.cc  $(LIBS)

#
#
#
clipim: clipim.cc  
	$(CPP) $(CCFLAGS) -o $(BIN)/clipim clipim.cc $(LIBS)


#
#
#
ixpesmooth: ixpesmooth.cc ixpesmooth_stokes.cc
	$(CPP) $(CCFLAGS) -o  $(BIN)/ixpesmooth ixpesmooth.cc ixpesmooth_stokes.cc $(LIBS) 

#
#
#
markim: markim.cc
	$(CPP) $(CCFLAGS) -o $(BIN)/markim markim.cc $(LIBS)

#
#
#
multiplyim: multiplyim.cc
	$(CPP) $(CCFLAGS) -o $(BIN)/multiplyim multiplyim.cc $(LIBS)


#
#
#
regionim: regionim.cc
	$(CPP) $(CCFLAGS) -o $(BIN)/regionim regionim.cc $(LIBS)


#
#
#
rebinim: rebinim.cc 
	$(CPP) $(CCFLAGS) -o $(BIN)/rebinim rebinim.cc $(LIBS)

#
#
#
sqrtim: sqrtim.cc
	$(CPP) $(CCFLAGS) -o $(BIN)/sqrtim sqrtim.cc $(LIBS)
#
#
#
subtractim:  subtractim.cc 
	$(CPP) $(CCFLAGS) -o $(BIN)/subtractim subtractim.cc $(LIBS)



################################################################################

#
#
#
image.o: image.cc
	$(CPP) $(CCFLAGS) -c image.cc

image_io.o: image_io.cc
	$(CPP) $(CCFLAGS) -c image_io.cc

image2.o: image2.cc
	$(CPP) $(CCFLAGS) -c image2.cc

image3.o: image3.cc
	$(CPP) $(CCFLAGS) -c image3.cc

#
#
#
region.o: region.cc
	$(CPP) $(CCFLAGS) -c region.cc

region_io.o: region_io.cc
	$(CPP) $(CCFLAGS) -c region_io.cc
