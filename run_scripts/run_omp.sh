#!/bin/bash
#SBATCH --job-name=lab2_omp
#SBATCH --time=00:30:00
#SBATCH --nodes=1 --ntasks-per-node=8
#SBATCH --mem=1gb
#SBATCH --output=lab2_omp_%j.out

../lab2_omp
