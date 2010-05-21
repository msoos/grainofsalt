cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

#SET(MY_BUILD_TYPE RelWithDebInfo CACHE STRING "Used instead of CMAKE_BUILD_TYPE")
#SET(CMAKE_BUILD_TYPE ${MY_BUILD_TYPE} CACHE STRING "..." FORCE)
#MARK_AS_ADVANCED(CMAKE_BUILD_TYPE )

PROJECT(grainofsalt)
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -Wall -ggdb -mtune=native")
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -O0 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -DNDEBUG -mtune=native -fomit-frame-pointer")
SET(CMAKE_EXE_LINKER_FLAGS "-static")

#FIND_PACKAGE(Threads REQUIRED)
#set(thread_library "${CMAKE_THREAD_LIBS_INIT}")

find_package( Boost 1.34 COMPONENTS system filesystem program_options REQUIRED)
link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( ${Boost_INCLUDE_DIRS} )

find_library(GMP_LIBRARIES NAMES gmp libgmp)

find_package( ZLIB )
link_directories( ${ZLIB_LIBRARY} )
include_directories( ${ZLIB_INCLUDE_DIR} )

add_subdirectory(espresso)
add_subdirectory(generator)

add_custom_target(copy ALL
    COMMENT "Copying binaries from subdirs to build directory")

add_custom_command(
    TARGET copy
    COMMAND ${CMAKE_COMMAND} -E copy generator/grainofsalt .
    COMMAND ${CMAKE_COMMAND} -E copy espresso/espresso_exec .
)

add_dependencies(copy espresso grainofsalt)
