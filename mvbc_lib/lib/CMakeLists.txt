cmake_minimum_required(VERSION 2.8)

project(mvbc_lib)

set(LIBMVBC_VERSION_MAJOR 1)
set(LIBMVBC_VERSION_MINOR 0)
set(LIBMVBC_VERSION_PATCH 0)
set(LIBMVBC_VERSION_STRING ${LIBMVBC_VERSION_MAJOR}.${LIBMVBC_VERSION_MINOR}.${LIBMVBC_VERSION_PATCH})

# Include paths
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../include/
	${CMAKE_CURRENT_SOURCE_DIR}/../parson/
	${CMAKE_CURRENT_SOURCE_DIR}/../../../mvbc/src/
	${CMAKE_CURRENT_SOURCE_DIR}/../../../target-powerpc64_e5500_glibc/linux-cygwp_t1042/mvbc/
)

# Library target
add_library(mvbc_lib
			lib_main.c
			json_parser.c
			../parson/parson.c ../parson/parson.h ../include/mvbc_lib.h
	)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLIBMVBC_VERSION_MAJOR=${LIBMVBC_VERSION_MAJOR}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLIBMVBC_VERSION_MINOR=${LIBMVBC_VERSION_MINOR}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLIBMVBC_VERSION_PATCH=${LIBMVBC_VERSION_PATCH}")

# Library version
set_target_properties(mvbc_lib PROPERTIES
  VERSION ${LIBMVBC_VERSION_STRING}
  SOVERSION ${LIBMVBC_VERSION_MAJOR})
