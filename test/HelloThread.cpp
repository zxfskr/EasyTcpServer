#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

#include <unistd.h>

#include "CellTimestamp.hpp"

using namespace std;

mutex m;
const int tCount = 4;
atomic_int sum;

void workFunc(int index)
{
    // m.lock();
    for (int i = 0; i < 20000000; i++){
        // 自解锁
        // lock_guard<mutex> lg(m);
        // m.lock(); // 临界区域k
        sum++;
        // cout << index << "hello, other thread, num: " << i << endl;
        // m.unlock();
    }  
    // m.unlock();
}

int main()
{

    thread *t[tCount];

    for (int i = 0; i < tCount; i++)
    {
        t[i] = new thread(workFunc, i);
    }

    CellTimestamp tTime;
    for (int i = 0; i < tCount; i++)
    {
        t[i]->join();
    }
    
    // t.detach();
    // sleep(1);
    cout << tTime.getElapsedMilliSec() << ", hello, main thread, result sum: "<<sum << endl;
    
    tTime.update();
    int test_sum = 0;
    for (int i = 0; i < 80000000; i++)
    {
        test_sum++;
    }
    cout << tTime.getElapsedMilliSec() << ", hello, main thread, result sum: "<<test_sum << endl;
    
    return 0;
} 