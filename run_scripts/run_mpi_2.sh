#!/bin/bash
#SBATCH --job-name=lab3_mpi_2
#SBATCH --time=01:00:00
#SBATCH --nodes=1 --ntasks-per-node=2
#SBATCH --mem=4gb
#SBATCH --output=lab3_mpi_2_%j.out

export I_MPI_LIBRARY=/usr/lib64/slurm/mpi_pmi2.so
srun --mpi=pmi2 ../lab3_mpi
