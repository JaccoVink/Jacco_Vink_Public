Before compiling the code make sure you have the NASA "heasoft" package
installed. See https://heasarc.gsfc.nasa.gov/docs/software/heasoft/

This package ensures that the pgplot and cfitsio libaries are installed.

Check the Makefile_dirs.in file for the correctness of the paths used.
Specifically JVTOOLS should point to your instalation directory. Make sure
the environment variable $HEADAS is set (pointing to the heasoft
installation directory).

If this is done compile the code by the command:
make all

You will find the compiled software under the top directory in ./bin.
The libraries compiled are in ./lib.


