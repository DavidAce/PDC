#!/bin/bash -l
#SBATCH -J cuda-lab03
#SBATCH --kill-on-invalid-dep=yes
#SBATCH --output=slurm/logs/cuda.out
#SBATCH --error=slurm/logs/cuda.err
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH -C Haswell
#SBATCH --gres=gpu:K420:1
#SBATCH --time=00:01:00 
#SBATCH -A edu19.summer
#SBATCH --reservation=summer-2019-08-26
#SBATCH --chdir=/afs/pdc.kth.se/home/a/aceituno/klemming/PDC/
srun  ./build/lab3 images/lab02.bmp