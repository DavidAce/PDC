#!/bin/bash

module load cuda/7.0

cmake -E make_directory build
cd build
cmake -DCMAKE_BUILD_TYPE=$build  -G "CodeBlocks - Unix Makefiles" ../
cmake --build .  -- -j 4