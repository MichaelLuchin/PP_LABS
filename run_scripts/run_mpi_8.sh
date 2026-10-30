#!/bin/bash
#SBATCH --job-name=lab2_mpi_8
#SBATCH --time=00:30:00
#SBATCH --nodes=1 --ntasks-per-node=8
#SBATCH --mem=4gb
#SBATCH --output=lab2_mpi_8_%j.out

export I_MPI_LIBRARY=/usr/lib64/slurm/mpi_pmi2.so
srun --mpi=pmi2 ../lab2_mpi
