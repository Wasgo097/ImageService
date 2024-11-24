#pragma once
#include <chrono>

class Stopwatch {
public:
    void start();
    double elapsedMilliseconds();
    double elapsedSeconds();
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _startTime;
};