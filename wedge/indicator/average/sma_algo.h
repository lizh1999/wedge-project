#pragma once

#include <deque>

namespace wedge {

class SimpleMovingAverageAlgo final {
 public:
  SimpleMovingAverageAlgo(int period) : period_(period), sum_(0.0) {}

  void update(double price) {
    prices_.push_back(price);
    sum_ += price;
    if (prices_.size() > period_) {
      sum_ -= prices_.front();
      prices_.pop_front();
    }
  }

  double value() const { return sum_ / period_; }

 private:
  int period_;
  double sum_;
  std::deque<double> prices_;
};

}  // namespace wedge