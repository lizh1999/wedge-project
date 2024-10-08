#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "wedge/common/candle.h"

namespace wedge {

class Indicator {
 public:
  virtual ~Indicator() = default;
  virtual void update(const Candle& candle) = 0;
  virtual double value() const = 0;
  virtual int period() const = 0;
};

class IndicatorContext {
 public:
  template <class T, class... Args>
  Indicator* add(Args&&... args) {
    auto ptr = std::make_unique<T>(args...);
    Indicator* result = ptr.get();
    indicators_.push_back(std::move(ptr));
    return result;
  }

  int max_period() {
    int result = 0;
    for (auto& indicator : indicators_) {
      result = std::max(result, indicator->period());
    }
    return result;
  }

  void update(const Candle& candle) {
    for (auto& indicator : indicators_) {
      indicator->update(candle);
    }
  }

 private:
  std::vector<std::unique_ptr<Indicator>> indicators_;
};

}  // namespace wedge