//
// Created by m_luc on 23.10.2026.
//
#include <iostream>
#include <mpi.h>
#include <iomanip>
#include <cstdlib>
#include <ctime>

const int K = 5;
const int Q = 20;
const int RUNS = 20;
const int N_MULTIPLE = 8400000;
const int N_NON_MULTIPLE = 10009;

struct Result
{
    double tSeq, tScatter, tParallel;
    bool ok;
};

double maxTime(double local)
{
    double maximum = 0.0;
    MPI_Reduce(&local, &maximum, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    return maximum;
}

void sumVectors(int** v, int* res, int n, int q)
{
    for (int i = 0; i < n; ++i)
    {
        int s = 0;
        for (int r = 0; r < q; ++r)
            for (int k = 0; k < K; ++k)
                s += v[k][i];
        res[i] = s;
    }
}

void splitWork(int n, int procs, int* counts, int* displs)
{
    const int base = n / procs;
    const int rest = n % procs;
    int shift = 0;
    for (int p = 0; p < procs; ++p)
    {
        counts[p] = base + (p < rest ? 1 : 0);
        displs[p] = shift;
        shift += counts[p];
    }
}

Result measure(int n, int q, bool useV, int rank, int procs)
{
    int* counts = new int[procs];
    int* displs = new int[procs];
    splitWork(n, procs, counts, displs);
    const int count = counts[rank];

    int* full[K];
    int* part[K];
    int* resSeq = 0;
    int* resPar = 0;
    int* partRes = new int[count];

    for (int k = 0; k < K; ++k)
    {
        full[k] = 0;
        part[k] = new int[count];
    }
    if (rank == 0)
    {
        for (int k = 0; k < K; ++k)
        {
            full[k] = new int[n];
            for (int i = 0; i < n; ++i)
                full[k][i] = std::rand() % 201 - 100;
        }
        resSeq = new int[n];
        resPar = new int[n];
    }

    Result res = {0.0, 0.0, 0.0, true};

    for (int run = 0; run < RUNS; ++run)
    {
        MPI_Barrier(MPI_COMM_WORLD);
        double start = MPI_Wtime();
        if (rank == 0)
        {
            sumVectors(full, resSeq, n, q);
            res.tSeq += MPI_Wtime() - start;
        }

        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();
        for (int k = 0; k < K; ++k)
        {
            if (useV)
                MPI_Scatterv(full[k], counts, displs, MPI_INT, part[k], count, MPI_INT, 0, MPI_COMM_WORLD);
            else
                MPI_Scatter(full[k], count, MPI_INT, part[k], count, MPI_INT, 0, MPI_COMM_WORLD);
        }
        res.tScatter += maxTime(MPI_Wtime() - start);

        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();
        sumVectors(part, partRes, count, q);
        if (useV)
            MPI_Gatherv(partRes, count, MPI_INT, resPar, counts, displs, MPI_INT, 0, MPI_COMM_WORLD);
        else
            MPI_Gather(partRes, count, MPI_INT, resPar, count, MPI_INT, 0, MPI_COMM_WORLD);
        res.tParallel += maxTime(MPI_Wtime() - start);

        if (rank == 0)
        {
            for (int i = 0; i < n; ++i)
                if (resSeq[i] != resPar[i])
                    res.ok = false;
        }
    }

    res.tSeq /= RUNS;
    res.tScatter /= RUNS;
    res.tParallel /= RUNS;

    for (int k = 0; k < K; ++k)
    {
        delete[] part[k];
        delete[] full[k];
    }
    delete[] partRes;
    delete[] resSeq;
    delete[] resPar;
    delete[] counts;
    delete[] displs;
    return res;
}

void printResult(const Result& r, int n, bool useV)
{
    const char* nSeq = useV ? "tSnon" : "tSmul";
    const char* nScatter = useV ? "tScV " : "tSc  ";
    const char* nPar = useV ? "tnon " : "tmul ";
    const char* nAccel = useV ? "anonScV" : "amulSc";
    const char* nAccelPure = useV ? "anon" : "amul";

    std::cout << (useV ? "--- Размерность некратная, N = " : "--- Размерность кратная, N = ") << n << " ---\n"
              << std::fixed << std::setprecision(9)
              << "Среднее время, сек:\n"
              << "  " << nSeq << " (последовательно): " << r.tSeq << "\n"
              << "  " << nScatter << " (" << (useV ? "Scatterv" : "Scatter") << ", пять векторов): " << r.tScatter << "\n"
              << "  " << nPar << " (счет + " << (useV ? "Gatherv" : "Gather") << "): " << r.tParallel << "\n"
              << std::setprecision(4)
              << "Ускорение с учетом рассылки " << nAccel << ": " << r.tSeq / (r.tScatter + r.tParallel) << "\n"
              << "Ускорение без учета рассылки " << nAccelPure << ": " << r.tSeq / r.tParallel << "\n"
              << "Результаты совпадают с последовательными: " << (r.ok ? "да" : "НЕТ") << "\n\n";
}

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);

    int rank = 0, procs = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);

    std::srand(static_cast<unsigned int>(std::time(0)));

    if (rank == 0)
    {
        std::cout << "Лабораторная работа 3: MPI\n"
                  << "Тип элементов: int\nK = " << K
                  << "\nN (кратная) = " << N_MULTIPLE
                  << "\nN (некратная) = " << N_NON_MULTIPLE
                  << "\nQ = " << Q
                  << "\nКоличество процессов в этом запуске: " << procs
                  << "\nЧисло запусков усреднения: " << RUNS << "\n\n";
    }

    const int qValues[] = {1, Q};
    for (int qi = 0; qi < 2; ++qi)
    {
        const int q = qValues[qi];
        if (rank == 0)
            std::cout << "========== Процессов: " << procs << ", Q = " << q << " ==========\n";

        Result mul = measure(N_MULTIPLE, q, false, rank, procs);
        Result non = measure(N_NON_MULTIPLE, q, true, rank, procs);

        if (rank == 0)
        {
            printResult(mul, N_MULTIPLE, false);
            printResult(non, N_NON_MULTIPLE, true);
        }
    }

    MPI_Finalize();
    return 0;
}
