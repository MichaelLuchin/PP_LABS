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
const int CHUNK = 100;
const int THREAD_COUNTS[] = {2, 4, 8};

struct Times
{
    double seq, init, st, dyn, gd;
};

int* createVector()
{
    int* v = new int[N];
    for (int i = 0; i < N; ++i)
        v[i] = std::rand() % 201 - 100;
    return v;
}

void sumSequential(int** v, int* res, int q)
{
    for (int i = 0; i < N; ++i)
    {
        int s = 0;
        for (int r = 0; r < q; ++r)
            for (int k = 0; k < K; ++k)
                s += v[k][i];
        res[i] = s;
    }
}

void sumStatic(int** v, int* res, int q)
{
    #pragma omp parallel
    {
        #pragma omp for schedule(static, CHUNK)
        for (int i = 0; i < N; ++i)
        {
            int s = 0;
            for (int r = 0; r < q; ++r)
                for (int k = 0; k < K; ++k)
                    s += v[k][i];
            res[i] = s;
        }
    }
}

void sumDynamic(int** v, int* res, int q)
{
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic, CHUNK)
        for (int i = 0; i < N; ++i)
        {
            int s = 0;
            for (int r = 0; r < q; ++r)
                for (int k = 0; k < K; ++k)
                    s += v[k][i];
            res[i] = s;
        }
    }
}

void sumGuided(int** v, int* res, int q)
{
    #pragma omp parallel
    {
        #pragma omp for schedule(guided, CHUNK)
        for (int i = 0; i < N; ++i)
        {
            int s = 0;
            for (int r = 0; r < q; ++r)
                for (int k = 0; k < K; ++k)
                    s += v[k][i];
            res[i] = s;
        }
    }
}

bool sameResult(const int* a, const int* b)
{
    for (int i = 0; i < N; ++i)
        if (a[i] != b[i])
            return false;
    return true;
}

void printTimes(int threads, int q, const Times& t, bool ok)
{
    std::cout << "========== Потоков: " << threads << ", Q = " << q << " ==========\n"
              << std::fixed << std::setprecision(6)
              << "Среднее время, сек:\n"
              << "  tSmul (последовательно): " << t.seq << "\n"
              << "  tp    (инициализация parallel): " << t.init << "\n"
              << "  tst   (schedule static): " << t.st << "\n"
              << "  td    (schedule dynamic): " << t.dyn << "\n"
              << "  tg    (schedule guided): " << t.gd << "\n"
              << std::setprecision(4)
              << "Ускорение с учетом инициализации parallel:\n"
              << "  astp: " << t.seq / (t.st + t.init) << "\n"
              << "  adp:  " << t.seq / (t.dyn + t.init) << "\n"
              << "  agp:  " << t.seq / (t.gd + t.init) << "\n"
              << "Ускорение без учета инициализации parallel:\n"
              << "  ast: " << t.seq / t.st << "\n"
              << "  ad:  " << t.seq / t.dyn << "\n"
              << "  ag:  " << t.seq / t.gd << "\n"
              << "Результаты совпадают с последовательными: " << (ok ? "да" : "НЕТ") << "\n\n";
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(0)));

    int* v[K];
    for (int k = 0; k < K; ++k)
        v[k] = createVector();
    int* resSeq = new int[N];
    int* resPar = new int[N];

    std::cout << "Лабораторная работа 3: OpenMP\n"
              << "Тип элементов: int\nK = " << K
              << "\nN = " << N << "\nQ = " << Q
              << "\nКоличество потоков: [2, 4, 8]"
              << "\nchunk = " << CHUNK
              << "\nЧисло запусков усреднения: " << RUNS << "\n\n";

    const int qValues[] = {1, Q};
    for (int qi = 0; qi < 2; ++qi)
    {
        const int q = qValues[qi];
        for (int ti = 0; ti < 3; ++ti)
        {
            const int threads = THREAD_COUNTS[ti];
            omp_set_num_threads(threads);

            Times t = {0.0, 0.0, 0.0, 0.0, 0.0};
            bool ok = true;

            for (int run = 0; run < RUNS; ++run)
            {
                double start = omp_get_wtime();
                sumSequential(v, resSeq, q);
                t.seq += omp_get_wtime() - start;

                start = omp_get_wtime();
                #pragma omp parallel
                {
                }
                t.init += omp_get_wtime() - start;

                start = omp_get_wtime();
                sumStatic(v, resPar, q);
                t.st += omp_get_wtime() - start;
                ok = ok && sameResult(resSeq, resPar);

                start = omp_get_wtime();
                sumDynamic(v, resPar, q);
                t.dyn += omp_get_wtime() - start;
                ok = ok && sameResult(resSeq, resPar);

                start = omp_get_wtime();
                sumGuided(v, resPar, q);
                t.gd += omp_get_wtime() - start;
                ok = ok && sameResult(resSeq, resPar);
            }

            t.seq /= RUNS;
            t.init /= RUNS;
            t.st /= RUNS;
            t.dyn /= RUNS;
            t.gd /= RUNS;

            printTimes(threads, q, t, ok);
        }
    }

    for (int k = 0; k < K; ++k)
        delete[] v[k];
    delete[] resSeq;
    delete[] resPar;
    return 0;
}
