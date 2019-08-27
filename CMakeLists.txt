cmake_minimum_required(VERSION 3.10)
project(lab3)
enable_language(CXX)

add_executable(${PROJECT_NAME} src/lab02_ex3_6.cu)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-arch=sm_30>)
