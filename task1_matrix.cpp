#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <clocale>
#include <windows.h>
#include <boost/thread.hpp>

using Matrix = std::vector<std::vector<double>>;

Matrix createRandomMatrix(std::size_t N) {
    Matrix m(N, std::vector<double>(N));
    std::mt19937_64 gen(42);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
            m[i][j] = dist(gen);
        }
    }
    return m;
}

Matrix multiplySingleThread(const Matrix &A, const Matrix &B) {
    std::size_t N = A.size();
    Matrix C(N, std::vector<double>(N, 0.0));

    for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t k = 0; k < N; ++k) {
            double a = A[i][k];
            for (std::size_t j = 0; j < N; ++j) {
                C[i][j] += a * B[k][j];
            }
        }
    }
    return C;
}

void workerMultiplyRows(const Matrix &A, const Matrix &B, Matrix &C,
                        std::size_t rowBegin, std::size_t rowEnd) {
    std::size_t N = A.size();
    for (std::size_t i = rowBegin; i < rowEnd; ++i) {
        for (std::size_t k = 0; k < N; ++k) {
            double a = A[i][k];
            for (std::size_t j = 0; j < N; ++j) {
                C[i][j] += a * B[k][j];
            }
        }
    }
}

Matrix multiplyMultiThreadBoost(const Matrix &A, const Matrix &B,
                                unsigned numThreads) {
    std::size_t N = A.size();
    Matrix C(N, std::vector<double>(N, 0.0));

    if (numThreads == 0) numThreads = 1;
    numThreads = std::min<unsigned>(numThreads, static_cast<unsigned>(N));

    std::vector<boost::thread> threads;
    threads.reserve(numThreads);

    std::size_t rowsPerThread = N / numThreads;
    std::size_t remainder = N % numThreads;

    std::size_t rowBegin = 0;
    for (unsigned t = 0; t < numThreads; ++t) {
        std::size_t take = rowsPerThread + (t < remainder ? 1 : 0);
        std::size_t rowEnd = rowBegin + take;

        threads.emplace_back(workerMultiplyRows,
                             std::cref(A), std::cref(B), std::ref(C),
                             rowBegin, rowEnd);

        rowBegin = rowEnd;
    }

    for (auto &th : threads) {
        th.join();
    }
    return C;
}

double measureSingleThread(std::size_t N) {
    Matrix A = createRandomMatrix(N);
    Matrix B = createRandomMatrix(N);

    auto start = std::chrono::high_resolution_clock::now();
    Matrix C = multiplySingleThread(A, B);
    (void)C;
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
}

double measureMultiThread(std::size_t N, unsigned numThreads) {
    Matrix A = createRandomMatrix(N);
    Matrix B = createRandomMatrix(N);

    auto start = std::chrono::high_resolution_clock::now();
    Matrix C = multiplyMultiThreadBoost(A, B, numThreads);
    (void)C;
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, "ru_RU.UTF-8");
    std::vector<std::size_t> sizes = {100, 500, 1000};
    std::vector<unsigned> threadCounts = {2, 4, 8};

    std::cout << "=== Умножение матриц (Boost.Thread) ===\n";

    for (auto N : sizes) {
        std::cout << "\nРазмер матрицы N = " << N << "\n";

        double t_single = measureSingleThread(N);
        std::cout << "Однопоточное время: " << t_single << " сек\n";

        for (auto threads : threadCounts) {
            double t_multi = measureMultiThread(N, threads);
            std::cout << "Потоков: " << threads
                      << ", многопоточное время: " << t_multi
                      << " сек, ускорение ~ " << (t_single / t_multi)
                      << "x\n";
        }
    }

    return 0;
}
