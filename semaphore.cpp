#include "semaphore.h"

using namespace std;

Semaphore::Semaphore (int N) : 
 N(N),
 max_N(N) {
     available = false;

}

void Semaphore::acquire(){
    {
        unique_lock<mutex> lk (mtx);
        cV.wait(lk, [&] {return N > 0;});
        N--;
    }
}

void Semaphore::release(){
    {
        unique_lock<mutex> lk (mtx);
        N++;
    }
    cV.notify_one();
}