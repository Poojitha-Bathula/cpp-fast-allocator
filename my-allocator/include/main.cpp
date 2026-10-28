#include <iostream>
#include <list>
#include <thread>
#include <chrono>
#include "STLAdapter.hpp"

const int THREADS = 4;
const int N = 200000;
const int ROUNDS = 5;

// work done by one thread using the normal std::list
void workerStd(long long* result) {
    std::list<int> myList;
    for (int i = 0; i < N; i++) {
        myList.push_back(i);
    }
    long long sum = 0;
    for (int x : myList) {
        sum = sum + x;
    }
    *result = sum;
}

// same work using our allocator
void workerCustom(long long* result) {
    std::list<int, STLAdapter<int>> myList;
    for (int i = 0; i < N; i++) {
        myList.push_back(i);
    }
    long long sum = 0;
    for (int x : myList) {
        sum = sum + x;
    }
    *result = sum;
}

// start 4 threads with the normal list, return time in ms
double runStd() {
    std::thread threads[THREADS];
    long long sums[THREADS];

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < THREADS; i++) {
        threads[i] = std::thread(workerStd, &sums[i]);
    }
    for (int i = 0; i < THREADS; i++) {
        threads[i].join();
    }
    auto end = std::chrono::steady_clock::now();

    return std::chrono::duration<double, std::milli>(end - start).count();
}

// start 4 threads with our allocator, return time in ms
double runCustom() {
    std::thread threads[THREADS];
    long long sums[THREADS];

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < THREADS; i++) {
        threads[i] = std::thread(workerCustom, &sums[i]);
    }
    for (int i = 0; i < THREADS; i++) {
        threads[i].join();
    }
    auto end = std::chrono::steady_clock::now();

    return std::chrono::duration<double, std::milli>(end - start).count();
}

int main() {
    // warm-up run (not counted)
    runStd();
    runCustom();

    double stdTotal = 0;
    double customTotal = 0;

    for (int r = 1; r <= ROUNDS; r++) {
        double a = runStd();
        double b = runCustom();
        std::cout << "round " << r << ": std = " << a
                  << " ms, custom = " << b << " ms\n";
        stdTotal = stdTotal + a;
        customTotal = customTotal + b;
    }

    double stdAvg = stdTotal / ROUNDS;
    double customAvg = customTotal / ROUNDS;

    std::cout << "\nAverage std    : " << stdAvg << " ms\n";
    std::cout << "Average custom : " << customAvg << " ms\n";
    std::cout << "Speedup        : " << stdAvg / customAvg << "x\n";
}