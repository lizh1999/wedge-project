#pragma once

#include <vector>

#include "wedge/indicator/average/ema_algo.h"
#include "wedge/indicator/indicator.h"

namespace wedge {

class RelativeStrengthIndex final : public Indicator {
  const int kCycleSize = 24;

 public:
  RelativeStrengthIndex(int period)
      : period_(period),
        last_value_(kCycleSize),
        average_gain_(kCycleSize, period),
        average_loss_(kCycleSize, period) {}

  void update(const Candle& candle) override {
    int index = count_++ % kCycleSize;
    if (count_ <= kCycleSize) {
      last_value_[index] = candle.close_price;
      return;
    }
    double gain = candle.close_price - last_value_[index];
    last_value_[index] = candle.close_price;

    if (gain > 0) {
      average_gain_[index].update(gain);
      average_loss_[index].update(0);
    } else if (gain < 0) {
      average_gain_[index].update(0);
      average_loss_[index].update(-gain);
    } else {
      average_gain_[index].update(0);
      average_loss_[index].update(0);
    }
  }

  double value() const override {
    int index = count_ % kCycleSize;
    double average_gain_value = average_gain_[index].value();
    double average_loss_value = average_loss_[index].value();
    double rs = average_gain_value / (average_loss_value + 1e-5);
    return 100 - (100 / (1 + rs));
  }

  int period() const override { return period_ * kCycleSize; }

 private:
  int period_;
  int count_ = 0;
  std::vector<double> last_value_;
  std::vector<ExponentialMovingAverageAlgo> average_gain_;
  std::vector<ExponentialMovingAverageAlgo> average_loss_;
};

}  // namespace wedge