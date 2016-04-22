#include <atomic>
#include <thread>
#include <mutex>
#include <cstdio>
#include <vector>
#include <exception>
#include <chrono>

class Futex {
    std::atomic<size_t> id;
public:
    Futex() : id(0) {
    }

    void lock() {
        size_t y = 0;
        std::thread::id thisId = std::this_thread::get_id();
        size_t myNumber = *((int *)(&thisId));
        while(!id.compare_exchange_weak(y, myNumber)) {
            y = 0;
            std::this_thread::yield();
        }
    }

    void unlock() {
        std::thread::id thisId = std::this_thread::get_id();
        if (id != *((int *)(&thisId))) {
            throw new std::exception();
        }
        id = 0;
    }
};

class AcquireFutex {
    std::atomic<size_t> id;
public:
    AcquireFutex() : id(0) {
    }

    void lock() {
        size_t y = 0;
        std::thread::id thisId = std::this_thread::get_id();
        size_t myNumber = *((int *)(&thisId));
        while(!id.compare_exchange_weak(y, myNumber,
                                        std::memory_order_acquire)) {
            y = 0;
            std::this_thread::yield();
        }
    }

    void unlock() {
        std::thread::id thisId = std::this_thread::get_id();
        if (id != *((int *)(&thisId))) {
            throw new std::exception();
        }
        id.store(0, std::memory_order_release);
    }
};

const int THREADS = 4;
const int MAXN = 240000000;

size_t counted[THREADS + 1];

Futex futex;
AcquireFutex afutex;

template<typename T>
void counter(size_t index, T &mutex) {
    while (true) {
        std::lock_guard<T> guard(mutex);
        if (counted[THREADS] == MAXN) {
            return;
        }
        ++counted[index];
        ++counted[THREADS];
    }
}

template<typename T>
void startCount(T &mutex) {
    for (int i = 0; i <= THREADS; ++i) {
        counted[i] = 0;
    }
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    std::vector<std::thread> threads;
    for (size_t index = 0; index < THREADS; ++index) {
        threads.push_back(std::thread(counter<T>, index, std::ref(mutex)));
    }
    size_t sum = 0;
    for (size_t index = 0; index < THREADS; ++index) {
        threads[index].join();
        sum += counted[index];
    }
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    printf("operations: %lu, value: %lu\n", sum, counted[THREADS]);
    printf("Time: %.3lf\n", elapsed_seconds.count());
}

int main() {
    startCount(futex);
    startCount(afutex);
    return 0;
}
