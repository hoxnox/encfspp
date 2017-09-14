# @author hoxnox <hoxnox@gmail.com>
# @date 20160810 15:03:46
# libencfs cmake tests build script

find_package(Threads)
list(APPEND LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

function(add_gtests gtname sources)
    foreach(source ${sources})
        file(READ "${source}" contents)
        string(REGEX MATCHALL "TEST_?F?\\(([A-Za-z_0-9 ,]+)\\)"
               found_tests ${contents})
        foreach(hit ${found_tests})
            string(REGEX REPLACE
                   ".*\\( *([A-Za-z_0-9]+), *([A-Za-z_0-9]+) *\\).*" "\\1.\\2"
                   test_name ${hit})
            add_test("${gtname}-${test_name}"
                     test_${gtname}_${PROJECT_NAME}
                     --gtest_filter=${test_name})
        endforeach()
    endforeach()
endfunction()

if (NOT USE_SYSTEM_GTEST)
    include(ExternalProject)
    sources_url(GTEST
        google/googletest/googletest-release-1.8.0.tar.gz
        https://github.com/google/googletest/archive/release-1.7.0.tar.gz)
    ExternalProject_Add(
        gtestlib
        URL ${GTEST_URL}
        PREFIX "${CMAKE_CURRENT_BINARY_DIR}/gtest"
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS=False
            -DCMAKE_CXX_COMPILER:STRING='${CMAKE_CXX_COMPILER}'
            -DCMAKE_CXX_FLAGS:STRING='${CMAKE_CXX_FLAGS}'
        INSTALL_DIR "${STAGING_DIR}"
        BUILD_IN_SOURCE 1
        LOG_DOWNLOAD 1
        LOG_UPDATE 1
        LOG_CONFIGURE 1
        LOG_BUILD 1
        LOG_TEST 1
        LOG_INSTALL 1
    )
    set(GTEST_INCLUDE_DIRS "${STAGING_DIR}/include")
    set(GTEST_LIBRARIES "${STAGING_DIR}/lib/libgtest.a")
else ()
    add_custom_target(gtestlib)
    find_package(GTest REQUIRED)
endif()
include_directories(${GTEST_INCLUDE_DIRS})
list(APPEND LIBRARIES ${GTEST_LIBRARIES})

include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories(${GTEST_INCLUDE_DIRS})
set(TEST_UNIT_SRC test.cpp)
add_executable(test_unit_${PROJECT_NAME} ${TEST_UNIT_SRC})
add_dependencies(test_unit_${PROJECT_NAME} gtestlib)
target_link_libraries(test_unit_${PROJECT_NAME} encfs ${LIBRARIES})
add_gtests("unit" "${TEST_UNIT_SRC}")

