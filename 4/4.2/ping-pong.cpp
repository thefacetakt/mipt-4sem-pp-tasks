#include <atomic>
#include <thread>
#include <mutex>
#include <cstdio>
#include <vector>
#include <exception>
#include <chrono>


struct PingPongStruct {
    int a;
    int b;
    PingPongStruct() {
        a = b = 0;
    }
};

PingPongStruct pps;

void f(int &x) {
    for (int i = 0; i < 1000 * 1000 * 1000; ++i) {
        x = (1289732 * x + 19238);
    }
}

int main() {
    std::vector<std::thread> threads;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    for (size_t index = 0; index < 2; ++index) {
        if (index == 0) {
            threads.push_back(std::thread(f, std::ref(pps.a)));
        } else {
            threads.push_back(std::thread(f, std::ref(pps.b)));
        }
    }
    for (size_t index = 0; index < 2; ++index) {
       threads[index].join();
    }

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    printf("a: %d, b: %d\n", pps.a, pps.b);
    printf("Time: %.3lf\n", elapsed_seconds.count());
}
