#!/bin/bash
#SBATCH --kill-on-invalid-dep=yes
#SBATCH --output=logs/cuda.out
#SBATCH --error=logs/cuda.err
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH -C Haswell
#SBATCH --gres=gpu:K420:1
#SBATCH --time=00:01:00 
#SBATCH -A edu19.summer
#SBATCH --reservation=summer-2019-08-26

srun ../build/lab3