#pragma once

#include <functional>
#include <queue>
#include <thread>

#include "common/chrono.h"

namespace wedge {

struct Task {
  TimePoint time_point;
  std::function<void()> callback;
};

inline bool operator>(const Task& left, const Task& right) {
  return left.time_point > right.time_point;
}

class Timer {
 public:
  void run() {
    while (!stop_ && !queue_.empty()) {
      Task top = std::move(queue_.top());
      queue_.pop();
      std::this_thread::sleep_until(top.time_point);
      top.callback();
    }
  }

  template <class Duration>
  void run_after(Duration duration, std::function<void()> callback) {
    TimePoint time_point = system_clock_.now() + duration;
    queue_.emplace(Task{time_point, std::move(callback)});
  }

  void stop() { stop_ = true; }

  TimePoint now() { return system_clock_.now(); }

 private:
  bool stop_;
  SystemClock system_clock_;
  std::priority_queue<Task, std::vector<Task>, std::greater<>> queue_;
};

}  // namespace wedge