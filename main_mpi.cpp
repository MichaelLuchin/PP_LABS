//
// Created by m_luc on 02.10.2026.
//
#include <iostream>
#include <mpi.h>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int RUNS = 12;
    double total_time = 0.0;

    for (int r = 0; r < RUNS; ++r) {
        std::cout << "Run num: " << r << std::endl;
        MPI_Barrier(MPI_COMM_WORLD);
        double start = MPI_Wtime();

        if (r == 0) {
            std::cout << "Hello world from process " << rank
                      << " of " << size << "\n";
        }

        MPI_Barrier(MPI_COMM_WORLD);
        double end = MPI_Wtime();

        if (rank == 0) {
            total_time += (end - start);
        }
    }

    if (rank == 0) {
        std::cout << "\nAverage time over " << RUNS << " runs ("
                  << size << " processes): " << (total_time / RUNS) << " sec\n";
    }

    MPI_Finalize();
    return 0;
}