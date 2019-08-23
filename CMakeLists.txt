cmake_minimum_required(VERSION 3.10)
project(lab1)
enable_language(CUDA)

add_executable(${PROJECT_NAME} src/lab01_ex1.cu)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-arch=sm_30>)

#add_executable(lab2 src/lab01_ex2.cu)
