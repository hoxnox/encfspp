# @author hoxnox <hoxnox@gmail.com>
# @date 20160719 12:09:57
# encfspp cmake build script

cmake_minimum_required(VERSION 3.1.0)

########################################################################
# options

option(WITH_TESTS "Build tests."        OFF)
option(WITH_DOCS  "Generate docs"       OFF)
option(WITH_LOG   "Logging replacement" "")

########################################################################
# general

project(encfspp)

set(encfspp_VERSION_MAJOR 0)
set(encfspp_VERSION_MINOR 0)
set(encfspp_VERSION_PATCH 2)
set(encfspp_TEST_SAMPLES_DIR "${PROJECT_SOURCE_DIR}/test/samples")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# configuration header
configure_file(
	"${PROJECT_SOURCE_DIR}/src/encfspp_config.cfg"
	"${PROJECT_SOURCE_DIR}/src/encfspp_config.h"
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wformat=2 -Wnon-virtual-dtor -Wcast-qual")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused -Wno-unused-parameter -Wno-parentheses")
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
# encfspp

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

INSTALL(TARGETS encfspp readpassphrase DESTINATION lib)
INSTALL(FILES include/EncfsMounter.hpp DESTINATION include)
SET(CPACK_PACKAGE_NAME encfspp)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "")
SET(CPACK_PACKAGE_VENDOR "Merder Kim <hoxnox@gmail.com>")
SET(CPACK_PACKAGE_CONTACT ${CPACK_PACKAGE_VENDOR})
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
SET(CPACK_PACKAGE_VERSION_MAJOR ${encfspp_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${encfspp_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${encfspp_VERSION_PATCH})
SET(CPACK_DEBIAN_PACKAGE_SECTION "misc")
SET(CPACK_RPM_PACKAGE_GROUP "Applications/System")
SET(CPACK_RPM_PACKAGE_LICENSE "MIT")
SET(CPACK_STRIP_FILES TRUE)
INCLUDE(CPack)

