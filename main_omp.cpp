//
// Created by m_luc on 02.10.2026.
//
#include <iostream>
#include <cstdlib>
#include <omp.h>

int main(int argc, char* argv[]) {
    int threads_count = 4;

    if (argc > 1) {
        threads_count = std::atoi(argv[1]);
        if (threads_count <= 0) {
            std::cerr << "Некорректное число потоков. Используется значение по умолчанию: 4\n";
            threads_count = 4;
        }
    }

    const int RUNS = 12;
    double total_time = 0.0;

    for (int r = 0; r < RUNS; ++r) {
        std::cout << "Run num: " << r << std::endl;

        omp_set_num_threads(threads_count);
        double start = omp_get_wtime();

        #pragma omp parallel
        {
            int nThreads = omp_get_num_threads();
            int threadNum = omp_get_thread_num();

            if (r == 0) {
                #pragma omp critical
                {
                    std::cout << "      OpenMP thread " << threadNum
                              << " from " << nThreads << " threads\n";
                }
            }
        }

        double end = omp_get_wtime();
        total_time += (end - start);
    }

    std::cout << "\nAverage time over " << RUNS << " runs ("
              << threads_count << " threads): " << (total_time / RUNS) << " sec\n";

    return 0;
}