#pragma once

#include <memory>

#include "common/candle.h"
#include "common/chrono.h"
#include "strategy/broker.h"

namespace wedge {

class IStrategy {
 public:
  explicit IStrategy(std::unique_ptr<IBroker> broker)
      : broker_(std::move(broker)) {}

  virtual ~IStrategy() = default;
  virtual Minutes setup(const Candle& candle) = 0;
  virtual Minutes update(const Candle& candle) = 0;
  virtual void on_order_filled(OrderIndex index) = 0;

 protected:
  std::unique_ptr<IBroker> broker_;
};

std::unique_ptr<IStrategy> grid_strategy(std::unique_ptr<IBroker> broker,
                                         int grid_count, double order_volume,
                                         double grid_spacing);

}  // namespace wedge