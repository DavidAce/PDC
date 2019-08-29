cmake_minimum_required(VERSION 3.10)
project(mpi-lab3)
enable_language(C CXX)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake-modules)

message("Using microarchitechture: haswell")
set(COMMON_OPTIONS -g -Wall -march=haswell -mtune=haswell)
set(DEBUG_OPTIONS            -Wpedantic -Wextra -O0 -g3 -fstack-protector -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2)
set(RELWITHDEBINFO_OPTIONS   -Wpedantic -Wextra -O1 -g3 -fstack-protector -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2)
set(RELEASE_OPTIONS          -DNDEBUG -O3 )
set(PROFILE_OPTIONS          -DNDEBUG -O3 -fopt-info -fopt-info-missed -lprofiler -g  -ftime-report)


add_executable(${PROJECT_NAME} src/mpi_derived_types_parallel.cpp)

target_link_libraries(${PROJECT_NAME}  PRIVATE -Wl,--allow-multiple-definition,--undefined=malloc,--undefined=_ZdaPv /pdc/vol/allinea-forge/19.1.1/lib/64/libdmallocxx.a)
###########################################
###  Apply RELEASE/DEBUG compile flags  ###
###########################################
target_compile_options (${PROJECT_NAME} PRIVATE ${COMMON_OPTIONS})                                          ### common options
target_compile_options (${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>")                      ### debug build options
target_compile_options (${PROJECT_NAME} PRIVATE "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>")                  ### Release build options
target_compile_options (${PROJECT_NAME} PRIVATE "$<$<CONFIG:RELWITHDEBINFO>:${RELWITHDEBINFO_OPTIONS}>")    ### Release build with debug info options
target_compile_options (${PROJECT_NAME} PRIVATE "$<$<CONFIG:PROFILE>:${PROFILE_OPTIONS}>")                  ### Profiling build options



####################
###  Link Eigen  ###
####################
set(BUILD_DIRECTORY   ${PROJECT_SOURCE_DIR}/cmake-build-libs)
set(INSTALL_DIRECTORY ${PROJECT_SOURCE_DIR}/libs)
option(DOWNLOAD_EIGEN3    "Download and install Eigen3 if not found"            ON)
include(cmake-modules/Fetch_Eigen3.cmake)                   # Eigen library
target_link_libraries(${PROJECT_NAME} PRIVATE Eigen3::Eigen)


##################
###  Link MPI  ###
##################
#set(MPI_CXX_COMPILER CC)
find_package(MPI REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${MPI_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}")
target_include_directories(${PROJECT_NAME} PRIVATE ${MPI_INCLUDE_PATH})





###############################
###  Require c++17 support  ###
###############################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
set_target_properties(${PROJECT_NAME}
        PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)

