cmake_minimum_required(VERSION 3.10)
project(mpi-lab1)
enable_language(CXX)

add_executable(${PROJECT_NAME} src/parallel_search-parallel.cpp src/class_tic_toc.cpp)



##################
###  Link MPI  ###
##################
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

