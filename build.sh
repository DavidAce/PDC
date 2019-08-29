#/bin/bash
#module purge
module load PrgEnv-cray/6.0.5
module load gcc/6.2.0
module load cray-mpich
module load cmake/3.11.4

export CC=gcc
export CXX=g++
cmake -E make_directory build
cd build
cmake -DCMAKE_BUILD_TYPE=Release  -G "CodeBlocks - Unix Makefiles" ../
cmake --build .  -- -j 4