//
// Created by m_luc on 23.10.2026.
//
#include <iostream>
#include <mpi.h>
#include <iomanip>
#include <cstdlib>
#include <ctime>

const int N = 8400000;
const int K = 5;
const int Q = 20;
const int RUNS = 20;

int* createVector()
{
    int* v = new int[N];
    for (int i = 0; i < N; ++i)
        v[i] = std::rand() % 201 - 100;
    return v;
}

double elapsedMaximum(double localElapsed, int rank)
{
    double maximum = 0.0;
    MPI_Reduce(&localElapsed, &maximum, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    return rank == 0 ? maximum : 0.0;
}

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);

    int rank = 0, processCount = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &processCount);

    std::srand(static_cast<unsigned int>(std::time(0)));

    int* v0 = (rank == 0) ? createVector() : new int[N];
    int* v1 = (rank == 0) ? createVector() : new int[N];
    int* v2 = (rank == 0) ? createVector() : new int[N];
    int* v3 = (rank == 0) ? createVector() : new int[N];
    int* v4 = (rank == 0) ? createVector() : new int[N];

    if (rank == 0)
    {
        std::cout << "Лабораторная работа 2: MPI\n"
                  << "Тип элементов: int\nK = " << K
                  << "\nN = " << N << "\nQ = " << Q
                  << "\nКоличество процессов в этом запуске: " << processCount
                  << "\nЧисло запусков усреднения: " << RUNS << "\n\n";
    }

    const int block = N / processCount;
    const int begin = block * rank;
    int end = block * (rank + 1);
    if (rank == processCount - 1)
        end = N;

    double tSeq = 0.0, tBcast = 0.0, tPointToPoint = 0.0, tReduce = 0.0;
    double seqSum = 0, pointSum = 0, reduceSum = 0;

    for (int run = 0; run < RUNS; ++run)
    {
        MPI_Barrier(MPI_COMM_WORLD);
        double start = MPI_Wtime();
        double localSeq = 0;
        if (rank == 0)
        {
            for (int i = 0; i < N; ++i)
                for (int q = 0; q < Q; ++q)
                    localSeq += v0[i] + v1[i] + v2[i] + v3[i] + v4[i];
        }
        double seqElapsed = (rank == 0) ? MPI_Wtime() - start : 0.0;
        MPI_Barrier(MPI_COMM_WORLD);
        double maxElapsed = elapsedMaximum(seqElapsed, rank);
        if (rank == 0)
        {
            tSeq += maxElapsed;
            seqSum = localSeq;
        }

        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();
        MPI_Bcast(v0, N, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(v1, N, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(v2, N, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(v3, N, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(v4, N, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        maxElapsed = elapsedMaximum(MPI_Wtime() - start, rank);
        if (rank == 0)
            tBcast += maxElapsed;

        double localSum = 0;
        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();
        for (int i = begin; i < end; ++i)
            for (int q = 0; q < Q; ++q)
                localSum += v0[i] + v1[i] + v2[i] + v3[i] + v4[i];

        double totalPoint = 0;
        if (rank == 0)
        {
            totalPoint = localSum;
            for (int source = 1; source < processCount; ++source)
            {
                double received = 0;
                MPI_Status status;
                MPI_Recv(&received, 1, MPI_DOUBLE, source, 10, MPI_COMM_WORLD, &status);
                totalPoint += received;
            }
        }
        else
        {
            MPI_Send(&localSum, 1, MPI_DOUBLE, 0, 10, MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        maxElapsed = elapsedMaximum(MPI_Wtime() - start, rank);
        if (rank == 0)
        {
            tPointToPoint += maxElapsed;
            pointSum = totalPoint;
        }

        localSum = 0;
        double totalReduce = 0;
        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();
        for (int i = begin; i < end; ++i)
            for (int q = 0; q < Q; ++q)
                localSum += v0[i] + v1[i] + v2[i] + v3[i] + v4[i];
        MPI_Reduce(&localSum, &totalReduce, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        maxElapsed = elapsedMaximum(MPI_Wtime() - start, rank);
        if (rank == 0)
        {
            tReduce += maxElapsed;
            reduceSum = totalReduce;
        }
    }

    if (rank == 0)
    {
        tSeq /= RUNS;
        tBcast /= RUNS;
        tPointToPoint /= RUNS;
        tReduce /= RUNS;

        std::cout << std::fixed << std::setprecision(6)
                  << "Среднее время, сек:\n"
                  << "  Последовательно: " << tSeq << "\n"
                  << "  Bcast (пять векторов): " << tBcast << "\n"
                  << "  Point-to-point (с локальным расчетом и сбором): " << tPointToPoint << "\n"
                  << "  MPI_Reduce (с локальным расчетом и сбором): " << tReduce << "\n"
                  << std::setprecision(4)
                  << "Ускорение с Bcast:\n"
                  << "  Point-to-point: " << tSeq / (tPointToPoint + tBcast) << "\n"
                  << "  MPI_Reduce: " << tSeq / (tReduce + tBcast) << "\n"
                  << "Ускорение без Bcast:\n"
                  << "  Point-to-point: " << tSeq / tPointToPoint << "\n"
                  << "  MPI_Reduce: " << tSeq / tReduce << "\n"
                  << "Контроль сумм (последовательная / point-to-point / MPI_Reduce):\n"
                  << std::setprecision(0) << seqSum << " / " << pointSum << " / " << reduceSum << "\n" << std::setprecision(4)
                  << "Результаты совпадают: "
                  << ((seqSum == pointSum && seqSum == reduceSum) ? "да" : "НЕТ") << "\n";
    }

    delete[] v0;
    delete[] v1;
    delete[] v2;
    delete[] v3;
    delete[] v4;

    MPI_Finalize();
    return 0;
}
