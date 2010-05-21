include_directories(${grainofsalt_SOURCE_DIR}/generator/MTRand)

link_directories(${grainofsalt_BINARY_DIR}/espresso)

add_executable(grainofsalt
	bestbits.cpp
	cipher.cpp
	cipherdesc.cpp
	data_holder.cpp
	dataelement.cpp
	dataelementrange.cpp
	debug.cpp equation.cpp
	equationholder.cpp
	equationstosat.cpp
	extendedmonomial.cpp
	main.cpp
	grain-of-salt.cpp
	monomial.cpp
	mystack.cpp
	output_maker.cpp
	polynomial.cpp
	variables.cpp
	xor_computer.cpp
	solverattrib.cpp
)

target_link_libraries(grainofsalt
	MV
	${Boost_LIBRARIES}
	${ZLIB_LIBRARY}
	${GMP_LIBRARIES}
)
