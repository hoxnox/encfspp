# @author hoxnox <hoxnox@gmail.com>
# @date 20160719 12:09:57
# libencfs cmake build script

cmake_minimum_required(VERSION 3.1.0)

########################################################################
# options

option(WITH_TESTS               "Build tests."  OFF)
option(WITH_DOCS                "Generate docs" OFF)
option(WITH_SYSTEM_FUSE         "Don't build libfuse" OFF)
option(WITH_SYSTEN_ENCFS        "Don't build encfs" OFF)
option(WITH_SYSTEM_OPENSSSL     "Don't build openssl" OFF)
option(ANDROID_TOOLCHAIN        "Path to Android toolchain" "")
option(ANDROID_TOOLCHAIN_PREFIX "Prefix for utilites"       "")
option(VENDOR_DIR               "Local directory with dependencies" "")

########################################################################
# general

project(libencfs)

set(libencfs_VERSION_MAJOR 0)
set(libencfs_VERSION_MINOR 0)
set(libencfs_VERSION_PATCH 1)
set(libencfs_TEST_SAMPLES_DIR "${PROJECT_SOURCE_DIR}/test/samples/")

set(CMAKE_CXX_STANDARD 11)
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

if (NOT VENDOR_DIR)
	if (DEFINED ENV{VENDOR_DIR})
		set(VENDOR_DIR $ENV{VENDOR_DIR})
		message(STATUS "VENDOR_DIR: ${VENDOR_DIR}")
	endif()
else()
		message(STATUS "VENDOR_DIR: ${VENDOR_DIR}")
endif()

########################################################################
# openssl

if (WITH_SYSTEM_OPENSSL)
	if(NOT OPENSSL_ROOT)
		set(OPENSSL_ROOT $ENV{OPENSSL_ROOT})
	endif()
	find_package(OpenSSL REQUIRED)
else()
	if (VENDOR_DIR)
		set(OPENSSL_URL "file:///${VENDOR_DIR}/openssl/openssl/openssl-1.0.1t.tar.gz")
	else ()
		set(OPENSSL_URL "https://www.openssl.org/source/openssl-1.0.1t.tar.gz")
	endif ()
	include(ExternalProject)
	ExternalProject_Add(openssl
		URL ${OPENSSL_URL}
		URL_HASH SHA256=4a6ee491a2fdb22e519c76fdc2a628bb3cec12762cd456861d207996c8a07088
		CONFIGURE_COMMAND ""
		BUILD_COMMAND "${PROJECT_SOURCE_DIR}/cmake/files/openssl-build.sh" "${STAGING_DIR}" "${ANDROID_TOOLCHAIN}" 
		INSTALL_COMMAND ""
		BUILD_IN_SOURCE 1
	)
	set(OPENSSL_LIBRARIES
		${STAGING_DIR}/lib/libcrypto.a
		${STAGING_DIR}/lib/libssl.a)
endif()

########################################################################
# fuse

if (WITH_SYSTEM_FUSE)
	if(NOT FUSE_ROOT)
		set(FUSE_ROOT $ENV{FUSE_ROOT})
	endif()
	find_package(FUSE REQUIRED)
else()
	if (VENDOR_DIR)
		set(FUSE_URL "file:///${VENDOR_DIR}/libfuse/libfuse/fuse-2.9.7.tar.gz")
	else ()
		set(FUSE_URL "https://github.com/libfuse/libfuse/releases/download/fuse-2.9.7/fuse-2.9.7.tar.gz")
	endif ()
	
	include(ExternalProject)
	if (ANDROID_TOOLCHAIN)
		if (NOT ANDROID_TOOLCHAIN_PREFIX)
			execute_process(COMMAND /bin/bash -c "basename ${ANDROID_TOOLCHAIN}/bin/*gcc"
				OUTPUT_VARIABLE GCC_NAME_FROM_TOOLCHAIN)
			string(FIND ${GCC_NAME_FROM_TOOLCHAIN} "-" LAST_HYPEN REVERSE)
			string(SUBSTRING ${GCC_NAME_FROM_TOOLCHAIN} 0 ${LAST_HYPEN} ANDROID_TOOLCHAIN_PREFIX_CALCULATED)
			message(STATUS "Caclulated toolchain prefix: ${ANDROID_TOOLCHAIN_PREFIX_CALCULATED}")
			set(ANDROID_TOOLCHAIN_PREFIX ${ANDROID_TOOLCHAIN_PREFIX_CALCULATED})
		endif ()
		ExternalProject_Add(fuse
			URL ${FUSE_URL}
			URL_HASH SHA256=832432d1ad4f833c20e13b57cf40ce5277a9d33e483205fc63c78111b3358874
			PATCH_COMMAND patch -p1 < "${PROJECT_SOURCE_DIR}/cmake/files/fuse-2.9.7.patch"
			CONFIGURE_COMMAND ./configure CFLAGS=-D__ANDROID__ --disable-shared
				--enable-static --host=${ANDROID_TOOLCHAIN_PREFIX}
				--with-sysroot=${ANDROID_TOOLCHAIN}/sysroot --disable-example
				--disable-util --prefix=<INSTALL_DIR>
			INSTALL_DIR ${STAGING_DIR}
			BUILD_IN_SOURCE 1
		)
	else()
		ExternalProject_Add(fuse
			URL ${FUSE_URL}
			URL_HASH SHA256=832432d1ad4f833c20e13b57cf40ce5277a9d33e483205fc63c78111b3358874
			CONFIGURE_COMMAND ./configure --disable-shared --enable-static
				--disable-shared --enable-static --disable-example
				--disable-util --prefix=<INSTALL_DIR>
			INSTALL_DIR ${STAGING_DIR}
			BUILD_IN_SOURCE 1
		)
	endif()
	set(FUSE_LIBRARIES
		${STAGING_DIR}/lib/libfuse.a)
endif()
add_definitions("-D_FILE_OFFSET_BITS=64")

########################################################################
# encfs

if (WITH_SYSTEM_ENCFS)
	if(NOT ENCFS_ROOT)
		set(ENCFS_ROOT $ENV{ENCFS_ROOT})
	endif()
	find_package(ENCFS REQUIRED)
else()
	if (VENDOR_DIR)
		set(ENCFS_URL "file:///${VENDOR_DIR}/vgough/encfs/encfs-1.9-rc1.tar.gz")
	else ()
		set(ENCFS_URL "https://github.com/vgough/encfs/archive/v1.9-rc1.tar.gz")
	endif ()
	include(ExternalProject)
	if (ANDROID_TOOLCHAIN)
		ExternalProject_Add(encfs-original
			URL ${ENCFS_URL}
			URL_HASH SHA256=ff16b824fdf613d69bb252fbe024a2ebcc9046e36fc639790182e19877067a44
			PATCH_COMMAND cat "${PROJECT_SOURCE_DIR}/cmake/files/encfs-1.9-rc1-*" | patch -p1
			CONFIGURE_COMMAND cmake -DCMAKE_C_FLAGS="-D__ANDROID__ -DBUILD_NLS=0 -fPIE -fPIC"
				-DCMAKE_EXE_LINKER_FLAGS="-pie" -DFUSE_USE_STATIC_LIBS=True
				-DCMAKE_C_COMPILER=${ANDROID_TOOLCHAIN}/bin/${ANDROID_TOOLCHAIN_PREFIX}-gcc
				-DCMAKE_CXX_COMPILER=${ANDROID_TOOLCHAIN}/bin/${ANDROID_TOOLCHAIN_PREFIX}-g++
				-DCMAKE_FIND_ROOT_PATH=${STAGING_DIR}
				-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER
				-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY
				-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY
				-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> <SOURCE_DIR>
			INSTALL_COMMAND make install && cp -r <SOURCE_DIR>/encfs <INSTALL_DIR>/include && touch <INSTALL_DIR>/include/config.h
			DEPENDS openssl fuse
			INSTALL_DIR ${STAGING_DIR}
		)
	else()
		ExternalProject_Add(encfs-original
			URL ${ENCFS_URL}
			URL_HASH SHA256=ff16b824fdf613d69bb252fbe024a2ebcc9046e36fc639790182e19877067a44
			PATCH_COMMAND cat
				"${PROJECT_SOURCE_DIR}/cmake/files/encfs-1.9-rc1-no_executables.patch"
				"${PROJECT_SOURCE_DIR}/cmake/files/encfs-1.9-rc1-staticlibs.patch"
				| patch -p1
			CONFIGURE_COMMAND cmake -DCMAKE_C_FLAGS="-DBUILD_NLS=0"
				-DFUSE_USE_STATIC_LIBS=True
				-DCMAKE_FIND_ROOT_PATH=${STAGING_DIR}
				-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER
				-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY
				-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY
				-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> <SOURCE_DIR>
			INSTALL_COMMAND make install && cp -r <SOURCE_DIR>/encfs <SOURCE_DIR>/internal <INSTALL_DIR>/include && touch <INSTALL_DIR>/include/config.h
			DEPENDS openssl fuse
			INSTALL_DIR ${STAGING_DIR}
		)
	endif()
	set(ENCFS_LIBRARIES
		${STAGING_DIR}/lib/libencfs.a)
endif()
list(APPEND LIBRARIES
	${ENCFS_LIBRARIES}
	${STAGING_DIR}/lib64/libtinyxml2.a
	${OPENSSL_LIBRARIES}
	${FUSE_LIBRARIES} 
	dl)

########################################################################
# libencfs

add_library(encfs src/EncfsMounter.cpp)
add_dependencies(encfs encfs-original)
include_directories("${STAGING_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
target_link_libraries(encfs ${LIBRARIES})

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

