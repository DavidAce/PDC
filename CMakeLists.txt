cmake_minimum_required(VERSION 3.10)
project(lab2)
enable_language(CUDA)

add_executable(${PROJECT_NAME} src/lab01_ex2.cu)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-arch=sm_30>)

