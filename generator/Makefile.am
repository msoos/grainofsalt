AM_CXXFLAGS = -Wall $(all_includes) -I$(srcdir) -I$(srcdir)/MTRand \
	      -I$(srcdir)/../mtl

pkgincludesubdir = $(includedir)/generator
pkgincludesub_HEADERS = bestbits.h     dataelementrange.h  equation.h \
	filereader.h     output_maker.h  time_mem.h \
	cipherdesc.h   data_holder.h       equationholder.h \
	grain-of-salt.h  polynomial.h    timer.h \
	cipher.h       debug.h             equationstosat.h \
	monomial.h       solverattrib.h  variables.h \
	dataelement.h  defines.h           extendedmonomial.h \
	mystack.h        SolverTypes.h   xor_computer.h \
	MTRand/MersenneTwister.h


bin_PROGRAMS = grainofsalt
grainofsalt_SOURCES =  bestbits.cpp  dataelement.cpp  debug.cpp  \
	equationstosat.cpp main.cpp  output_maker.cpp  variables.cpp \
	cipher.cpp  dataelementrange.cpp equation.cpp  extendedmonomial.cpp \
       	monomial.cpp  polynomial.cpp xor_computer.cpp cipherdesc.cpp \
	data_holder.cpp  equationholder.cpp  grain-of-salt.cpp  mystack.cpp \
	solverattrib.cpp

grainofsalt_LDADD = ../espresso/libespresso.la -lgmp -lboost_program_options -lboost_filesystem
