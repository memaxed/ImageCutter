#include <chrono>
using namespace std::chrono;

class Timer {
    high_resolution_clock::time_point start;
public:
    Timer() { reset(); }

    void reset() {
        start = high_resolution_clock::now();
    }

    long long ns() const {
        return duration_cast<nanoseconds>(
            high_resolution_clock::now() - start).count();
    }

    long long us() const {
        return duration_cast<microseconds>(
            high_resolution_clock::now() - start).count();
    }

    long long ms() const {
        return duration_cast<milliseconds>(
            high_resolution_clock::now() - start).count();
    }

    double seconds() const {
        return duration<double>(
            high_resolution_clock::now() - start).count();
    }

};
