add_executable(test_unit_${PROJECT_NAME} test.cpp)
target_include_directories(test_unit_${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(test_unit_${PROJECT_NAME} encfspp ${LIBRARIES})
add_dependencies(test_unit_${PROJECT_NAME} encfspp)

