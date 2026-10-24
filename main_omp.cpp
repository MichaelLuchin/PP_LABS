//
// Created by m_luc on 23.10.2026.
//
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <omp.h>

const int N = 8400000;
const int K = 5;
const int Q = 20;
const int RUNS = 20;
const int THREAD_COUNTS[] = {2, 4, 8};

int* createVector()
{
    int* v = new int[N];
    for (int i = 0; i < N; ++i)
        v[i] = std::rand() % 201 - 100;
    return v;
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(0)));

    int* v0 = createVector();
    int* v1 = createVector();
    int* v2 = createVector();
    int* v3 = createVector();
    int* v4 = createVector();

    std::cout << "Лабораторная работа 2: OpenMP\n"
              << "Тип элементов: int\nK = " << K
              << "\nN = " << N << "\nQ = " << Q
              << "\nКоличество потоков: [2, 4, 8]"
              << "\nЧисло запусков усреднения: " << RUNS << "\n\n";

    for (int cfg = 0; cfg < 3; ++cfg)
    {
        const int threads = THREAD_COUNTS[cfg];
        omp_set_num_threads(threads);

        double tSeq = 0.0, tInit = 0.0;
        double tCritical = 0.0, tAtomic = 0.0, tReduction = 0.0;
        long double seqSum = 0, criticalSum = 0, atomicSum = 0, reductionSum = 0;

        for (int run = 0; run < RUNS; ++run)
        {
            long double sum = 0;
            double start = omp_get_wtime();
            for (int i = 0; i < N; ++i)
                for (int q = 0; q < Q; ++q)
                    sum += v0[i] + v1[i] + v2[i] + v3[i] + v4[i];
            tSeq += omp_get_wtime() - start;
            seqSum = sum;

            start = omp_get_wtime();
            #pragma omp parallel
            {
            }
            tInit += omp_get_wtime() - start;

            long double sumCritical = 0;
            start = omp_get_wtime();
            #pragma omp parallel for
            for (int i = 0; i < N; ++i)
            {
                for (int q = 0; q < Q; ++q)
                {
                    #pragma omp critical
                    sumCritical += v0[i] + v1[i] + v2[i] + v3[i] + v4[i];
                }
            }
            tCritical += omp_get_wtime() - start;
            criticalSum = sumCritical;

            long double sumAtomic = 0;
            start = omp_get_wtime();
            #pragma omp parallel for
            for (int i = 0; i < N; ++i)
            {
                for (int q = 0; q < Q; ++q)
                {
                    #pragma omp atomic
                    sumAtomic += v0[i] + v1[i] + v2[i] + v3[i] + v4[i];
                }
            }
            tAtomic += omp_get_wtime() - start;
            atomicSum = sumAtomic;

            long double sumReduction = 0;
            start = omp_get_wtime();
            #pragma omp parallel for reduction(+:sumReduction)
            for (int i = 0; i < N; ++i)
            {
                for (int q = 0; q < Q; ++q)
                    sumReduction += v0[i] + v1[i] + v2[i] + v3[i] + v4[i];
            }
            tReduction += omp_get_wtime() - start;
            reductionSum = sumReduction;
        }

        tSeq /= RUNS;
        tInit /= RUNS;
        tCritical /= RUNS;
        tAtomic /= RUNS;
        tReduction /= RUNS;

        std::cout << "========== Потоков: " << threads << " ==========\n"
                  << std::fixed << std::setprecision(6)
                  << "Среднее время, сек:\n"
                  << "  Последовательно: " << tSeq << "\n"
                  << "  Инициализация parallel: " << tInit << "\n"
                  << "  critical: " << tCritical << "\n"
                  << "  atomic: " << tAtomic << "\n"
                  << "  reduction: " << tReduction << "\n"
                  << std::setprecision(4)
                  << "Ускорение с инициализацией:\n"
                  << "  critical: " << tSeq / (tCritical + tInit) << "\n"
                  << "  atomic: " << tSeq / (tAtomic + tInit) << "\n"
                  << "  reduction: " << tSeq / (tReduction + tInit) << "\n"
                  << "Ускорение без инициализации:\n"
                  << "  critical: " << tSeq / tCritical << "\n"
                  << "  atomic: " << tSeq / tAtomic << "\n"
                  << "  reduction: " << tSeq / tReduction << "\n"
                  << "Контроль сумм (последовательная / critical / atomic / reduction):\n"
                  << std::setprecision(0) << seqSum << " / " << criticalSum << " / " << atomicSum << " / " << reductionSum << "\n"
                  << std::setprecision(4)
                  << "Результаты совпадают: "
                  << ((seqSum == criticalSum && seqSum == atomicSum && seqSum == reductionSum) ? "да" : "НЕТ")
                  << "\n\n";
    }

    delete[] v0;
    delete[] v1;
    delete[] v2;
    delete[] v3;
    delete[] v4;
    return 0;
}
