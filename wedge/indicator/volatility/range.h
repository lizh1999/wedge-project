#pragma once

#include <vector>

#include "wedge/indicator/indicator.h"

namespace wedge {

class Range final : public Indicator {
 public:
  Range(int period) : high_(period), low_(period) {}

  void update(const Candle& candle) override {
    int index = count_++ % high_.size();
    high_[index] = candle.high_price;
    low_[index] = candle.low_price;
  }

  double value() const override {
    return apply(high_, std::greater<void>{}) - apply(low_, std::less<void>{});
  }

  int period() const override { return high_.size(); }

 private:
  template <class Perd>
  static double apply(const std::vector<double>& vector, Perd&& perd) {
    return apply(vector.data(), vector.size(), std::forward<Perd&&>(perd));
  }

  template <class Perd>
  static double apply(const double* data, size_t size, Perd&& perd) {
    double result = data[0];
    for (size_t i = 1; i < size; i++) {
      result = std::min(result, data[i], perd);
    }
    return result;
  }

  int count_ = 0;
  std::vector<double> high_;
  std::vector<double> low_;
};

}  // namespace wedge