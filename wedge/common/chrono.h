#pragma once

#include <chrono>

namespace wedge {
using TimePoint = std::chrono::system_clock::time_point;
using SystemClock = std::chrono::system_clock;
using Minutes = std::chrono::minutes;
using Hours = std::chrono::hours;
using Milliseconds = std::chrono::milliseconds;
using std::chrono::duration_cast;
using namespace std::chrono_literals;
}