#!/bin/bash
#SBATCH --job-name=lab2_mpi_4
#SBATCH --time=00:30:00
#SBATCH --nodes=1 --ntasks-per-node=4
#SBATCH --mem=4gb
#SBATCH --output=lab2_mpi_4_%j.out

export I_MPI_LIBRARY=/usr/lib64/slurm/mpi_pmi2.so
srun --mpi=pmi2 ./lab2_mpi
