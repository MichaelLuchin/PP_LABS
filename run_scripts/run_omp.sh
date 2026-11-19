#!/bin/bash
#SBATCH --job-name=lab3_omp
#SBATCH --time=01:00:00
#SBATCH --nodes=1 --ntasks-per-node=8
#SBATCH --mem=2gb
#SBATCH --output=lab3_omp_%j.out

../lab3_omp
