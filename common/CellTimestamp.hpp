#ifndef __CellTimestamp_hpp__
#define __CellTimestamp_hpp__

#include <chrono>

using namespace std::chrono;

class CellTimestamp
{
private:
    /* data */
    time_point<high_resolution_clock> _begin;

public:
    CellTimestamp(/* args */)
    {
        update();
    }

    void update(){
        _begin = high_resolution_clock::now();
    }
    
    double getElapsedSecond(){
        return getElapsedTimeInMicroSec() * 0.000001;
    }

    double getElapsedMilliSec(){
        return getElapsedTimeInMicroSec() * 0.001;
    }

    long long getElapsedTimeInMicroSec(){
        return duration_cast<microseconds>(high_resolution_clock::now()-_begin).count();
    }

    ~CellTimestamp(){}
};

#endif