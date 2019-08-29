#/bin/bash
module purge
module load cdt/19.06
module load cmake/3.14.5
module load PrgEnv-cray/6.0.5

#export CC=cc
export CXX=CC
cmake -E make_directory build
cd build
cmake -DCMAKE_BUILD_TYPE=Release  -G "CodeBlocks - Unix Makefiles" ../
cmake --build .  -- -j 4