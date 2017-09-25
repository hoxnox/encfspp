# @author hoxnox <hoxnox@gmail.com> # @date 20160719 12:09:57
# libencfs cmake build script

cmake_minimum_required(VERSION 3.1.0)

########################################################################
# options

option(WITH_TESTS               "Build tests."  OFF)
option(WITH_DOCS                "Generate docs" OFF)

########################################################################
# general

project(libencfs)

set(libencfs_VERSION_MAJOR 0)
set(libencfs_VERSION_MINOR 0)
set(libencfs_VERSION_PATCH 1)
set(libencfs_TEST_SAMPLES_DIR "${PROJECT_SOURCE_DIR}/test/samples")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# configuration header
configure_file(
	"${PROJECT_SOURCE_DIR}/src/libencfs_config.cfg"
	"${PROJECT_SOURCE_DIR}/src/libencfs_config.h"
	ESCAPE_QUOTES
)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}"
                      "${PROJECT_SOURCE_DIR}/cmake/Modules")

include_directories("${PROJECT_SOURCE_DIR}/src")
set(STAGING_DIR "${CMAKE_BINARY_DIR}/staging")

include(sanitizers)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Threads REQUIRED)
list(APPEND LIBRARIES ${CONAN_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

########################################################################
# pluggable logging

# replace logging.hpp
if (NOT WITH_LOG)
	set(WITH_LOG "${PROJECT_BINARY_DIR}/log")
	configure_file("${PROJECT_SOURCE_DIR}/src/Logging.hpp" "${PROJECT_BINARY_DIR}/log/logging.hpp" COPYONLY)
	include_directories(BEFORE "${PROJECT_BINARY_DIR}/log")
else()
	include_directories(BEFORE "${WITH_LOG}")
endif()

########################################################################
# libencfs

add_library(readpassphrase src/readpassphrase.cpp)
list(APPEND LIBRARIES readpassphrase)

add_library(encfspp src/EncfsMounter.cpp)
add_dependencies(encfspp readpassphrase)
include_directories("${STAGING_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
target_link_libraries(encfspp ${LIBRARIES})
set_property(TARGET encfspp PROPERTY COMPILE_DEFINITIONS "_FILE_OFFSET_BITS=64")

########################################################################
# tests and docs

if(WITH_DOCS)
	add_subdirectory(doc)
endif()

if(WITH_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()

########################################################################
# installation

set(CPACK_SET_DESTDIR ON)

#INSTALL(TARGETS
#	targetname
#	DESTINATION bin)
#INSTALL(FILES filepath DESTINATION bin
#	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
#	            GROUP_EXECUTE GROUP_READ
#	            WORLD_READ WORLD_EXECUTE)
#INSTALL(DIRECTORY domedir DESTINATION share/${PROJECT_NAME})
SET(CPACK_PACKAGE_NAME libencfs)
#if (WITH_SYSTEM_SOMELIB)
#	SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libsomelib (>=1.1.1)")
#	SET(CPACK_RPM_PACKAGE_REQUIRES "libsomelib >= 1.1.1")
#endif()
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "")
SET(CPACK_PACKAGE_VENDOR "$uername$ <hoxnox@gmail.com>")
SET(CPACK_PACKAGE_CONTACT ${CPACK_PACKAGE_VENDOR})
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
SET(CPACK_PACKAGE_VERSION_MAJOR ${libencfs_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${libencfs_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${libencfs_VERSION_PATCH})
SET(CPACK_DEBIAN_PACKAGE_SECTION "misc")
SET(CPACK_RPM_PACKAGE_GROUP "Applications/System")
SET(CPACK_RPM_PACKAGE_LICENSE "MIT")
SET(CPACK_STRIP_FILES TRUE)
INCLUDE(CPack)

