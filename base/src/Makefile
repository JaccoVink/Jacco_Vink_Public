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




BASE_LIB_SRC = basicdata.o basicdata_io.o jvtools_messages.o history.o \
	jvtools_gti.o  extraction.o io_tools.o jvtools_cfitsio.o \
	polarization.o polarization_ixpe.o




###############################################################################
# 
# Make the astro_x library 
#
###############################################################################


all: basiclib numreclib

basiclib: $(BASE_LIB_SRC)
	$(AR) $(LIB_DIR1)/libjvtools_basic.a $(BASE_LIB_SRC)
	$(AR2) $(LIB_DIR1)/libjvtools_basic.a


basiclibsh:  $(BASE_LIB_SRC)
	g++ -shared -Wl,-soname,libjvtools_basic.so -o libjvtools_basic.so\
	$(BASE_LIB_SRC)
	mv libjvtools_basic.so $(LIB_DIR2)


numreclib: $(NUMREC_OLD_OBJ)
	$(AR) $(LIB_DIR)libnumrec.a $(NUMREC_OLD_OBJ)
	$(AR) $(LIB_DIR)libnumrec.a
	g++ -shared -Wl,-soname,libnumrec.so -o libnumrec.so \
	$(NUMREC_OLD_OBJ)
	mv libnumrec.so $(LIB_DIR2)




clean:
	$(RM) *.o

################################################################################
#
# 	In the next block executables are made:
#
################################################################################

#
#
#
badpixim: badpixim.cc
	$(CPP)  $(CCFLAGS) -o $(BIN)/badpixim badpixim.cc $(LIBS)



################################################################################
#
# 	In the next block object files are made of several C++ files.
#	In all cases the files contain the implementation of the classes
#	with the same name as the file.
#
################################################################################


#
#
#
io_tools.o: io_tools.cc
	$(CPP) $(CCFLAGS) -c io_tools.cc


#
#
#
jvtools_messages.o: jvtools_messages.cc
	$(CPP) $(CCFLAGS) -c jvtools_messages.cc
#
#
history.o: history.cc
	$(CPP) $(CCFLAGS) -c history.cc
#
#
#
jvtools_gti.o: jvtools_gti.cc
	$(CPP) $(CCFLAGS) -c jvtools_gti.cc


#
#
#
backgroundfilter.o: backgroundfilter.cc
	$(CPP) $(CCFLAGS) -c backgroundfilter.cc

#
#
#
basicdata.o: basicdata.cc
	$(CPP) $(CCFLAGS) -c basicdata.cc

basicdata_io.o: basicdata_io.cc
	$(CPP) $(CCFLAGS) -c basicdata_io.cc


basicdata_applyPSF.o: basicdata_applyPSF.cc
	$(CPP) $(CCFLAGS) -c basicdata_applyPSF.cc

#
#
#
extraction.o: extraction.cc
	$(CPP) $(CCFLAGS) -c extraction.cc


#
#
#
jvtools_cfitsio.o: jvtools_cfitsio.cc
	$(CPP) $(CCFLAGS) -c jvtools_cfitsio.cc
#
#
#
polarization.o: polarization.cc
	$(CPP) $(CCFLAGS) -c polarization.cc

polarization_ixpe.o: polarization_ixpe.cc
	$(CPP) $(CCFLAGS) -c polarization_ixpe.cc

