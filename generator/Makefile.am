AM_CXXFLAGS = -Wall $(all_includes) -I$(srcdir) -I$(srcdir)/MTRand \
	      -I$(srcdir)/../mtl

bin_PROGRAMS = grainofsalt
grainofsalt_SOURCES =  bestbits.cpp  dataelement.cpp  debug.cpp  \
	equationstosat.cpp main.cpp  output_maker.cpp  variables.cpp \
	cipher.cpp  dataelementrange.cpp equation.cpp  extendedmonomial.cpp \
       	monomial.cpp  polynomial.cpp xor_computer.cpp cipherdesc.cpp \
	data_holder.cpp  equationholder.cpp  grain-of-salt.cpp  mystack.cpp \
	solverattrib.cpp

grainofsalt_LDADD = ../espresso/libespresso.la -lgmp -lboost_program_options -lboost_filesystem
