#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <iostream>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <mutex>

class Semaphore
{
public:
    Semaphore(int N);

    void acquire();
    void release();
private:
    int N, max_N;
    bool available;
    std::condition_variable cV;
    std::mutex mtx;

};

#endif