#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <clocale>
#include <windows.h>


const std::size_t OPERATIONS_PER_CLIENT = 100000;


long long balance_no_sync = 0;

void clientNoSync() {
    for (std::size_t i = 0; i < OPERATIONS_PER_CLIENT; ++i) {
        balance_no_sync++;
        balance_no_sync--;
    }
}


std::atomic<long long> balance_atomic(0);

void clientAtomic() {
    for (std::size_t i = 0; i < OPERATIONS_PER_CLIENT; ++i) {
        balance_atomic.fetch_add(1, std::memory_order_relaxed);
        balance_atomic.fetch_sub(1, std::memory_order_relaxed);
    }
}

long long balance_mutex = 0;
std::mutex balance_mtx;

void clientMutex() {
    for (std::size_t i = 0; i < OPERATIONS_PER_CLIENT; ++i) {
        std::lock_guard<std::mutex> lock(balance_mtx);
        balance_mutex++;
        balance_mutex--;
    }
}

template <typename Func>
double runScenario(Func func, unsigned numClients) {
    std::vector<std::thread> threads;
    threads.reserve(numClients);

    auto start = std::chrono::high_resolution_clock::now();
    for (unsigned i = 0; i < numClients; ++i) {
        threads.emplace_back(func);
    }
    for (auto &t : threads) {
        t.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, "ru_RU.UTF-8");
    std::vector<unsigned> clientCounts = {2, 4, 8};

    std::cout << "=== Моделирование банковских счетов ===\n";
    std::cout << "Операций на клиента: " << OPERATIONS_PER_CLIENT << "\n\n";

    for (auto clients : clientCounts) {
        std::cout << "---- Клиентов (потоков): " << clients << " ----\n";

        balance_no_sync = 0;
        double t1 = runScenario(clientNoSync, clients);
        std::cout << "Без синхронизации: время = " << t1
                  << " сек, итоговый баланс = " << balance_no_sync << "\n";

        balance_atomic.store(0);
        double t2 = runScenario(clientAtomic, clients);
        std::cout << "std::atomic<long long>: время = " << t2
                  << " сек, итоговый баланс = " << balance_atomic.load() << "\n";

        {
            std::lock_guard<std::mutex> lock(balance_mtx);
            balance_mutex = 0;
        }
        double t3 = runScenario(clientMutex, clients);
        {
            std::lock_guard<std::mutex> lock(balance_mtx);
            std::cout << "std::mutex: время = " << t3
                      << " сек, итоговый баланс = " << balance_mutex << "\n";
        }

        std::cout << "\n";
    }

    std::cout << "Ожидаемый «идеальный» итоговый баланс во всех случаях = 0\n"
                 "(инкремент + декремент в каждой операции).\n"
                 "Без синхронизации вы будете наблюдать аномалии баланса.\n";

    return 0;
}
