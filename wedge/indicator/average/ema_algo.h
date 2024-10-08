#pragma once

namespace wedge {

class ExponentialMovingAverageAlgo final {
 public:
  ExponentialMovingAverageAlgo(int period)
      : alpha_(2.0 / (period + 1)), value_(0), has_value_(false) {}

  void update(double price) {
    if (!has_value_) {
      has_value_ = true;
      value_ = price;
      return;
    }
    value_ = price * alpha_ + (1 - alpha_) * value_;
  }

  double value() const { return value_; }

 private:
  double alpha_;
  double value_;
  bool has_value_;
};

}  // namespace wedge