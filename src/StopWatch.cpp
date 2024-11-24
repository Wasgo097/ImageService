#include "StopWatch.h"

void Stopwatch::start()
{
    _startTime = std::chrono::high_resolution_clock::now();
}

double Stopwatch::elapsedMilliseconds()
{
    return std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - _startTime).count();
}

double Stopwatch::elapsedSeconds()
{
    return elapsedMilliseconds() / 1000.0;
}