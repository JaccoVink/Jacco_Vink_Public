Before compiling the code make sure you have the NASA "heasoft" package
installed. See https://heasarc.gsfc.nasa.gov/docs/software/heasoft/

This package ensures that the pgplot and cfitsio libaries are installed.

The principle component analysis (PCA) code makes use of the linear algebra C++
libray Eigen v3.4.0. The code is included in the current packaged, but can also
be downloaded from https://gitlab.com/libeigen/eigen/-/releases/3.4.0.

Check the Makefile_dirs.in file for the correctness of the paths used.
Specifically JVTOOLS should point to your instalation directory. Make sure
the environment variable $HEADAS is set (pointing to the heasoft
installation directory).

If this is done compile the code by the command:
make all

You will find the compiled software under the top directory in ./bin.
The libraries compiled are in ./lib.

For PCA code check out 10.5281/zenodo.10301088 for output results of the code
and a simple script to invoke the PCA code (pcaim) and use it.

