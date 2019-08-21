
cmake_minimum_required(VERSION 3.10)
#project(hello)
add_executable(TEST-hello hello.cpp)

set(CMAKE_VERBOSE_MAKEFILE ON)

target_include_directories(TEST-hello PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(TEST-hello PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

################################################
###  Force cmake to find .a library suffixes ###
################################################
if(NOT BUILD_SHARED_LIBS)
    target_link_libraries  (TEST-hello PRIVATE -static)                                             ### Static linkage
endif()
target_link_libraries(TEST-hello PRIVATE Eigen3::Eigen lapacke  spdlog::spdlog)
target_link_libraries(TEST-hello PRIVATE ${COMPILER_LFLAGS})



set_target_properties  (TEST-hello PROPERTIES CXX_STANDARD_REQUIRED 17)
target_compile_features(TEST-hello PRIVATE cxx_std_17)
target_compile_options (TEST-hello PRIVATE ${COMMON_OPTIONS})                                   ### Common options
target_compile_options (TEST-hello PRIVATE "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>")               ### Debug build options
target_compile_options (TEST-hello PRIVATE "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>")           ### Release build options
target_compile_options (TEST-hello PRIVATE "$<$<CONFIG:RELWITHDEBINFO>:${RELWITHDEBINFO_OPTIONS}>")    ### Release build with debug info options
add_test(NAME hello_test COMMAND TEST-hello)

add_dependencies(TEST-hello Eigen3::Eigen spdlog::spdlog gfortran )