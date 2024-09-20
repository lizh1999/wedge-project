#pragma once

#include <spdlog/spdlog.h>

#include <memory>

#include "wedge/common/candle.h"
#include "wedge/strategy/broker.h"

namespace wedge {

class IStrategy {
 public:
  explicit IStrategy(IBroker* broker, std::shared_ptr<spdlog::logger> logger)
      : broker_(broker), logger_(logger) {}

  virtual ~IStrategy() = default;
  virtual void update(const Candle& candle) = 0;
  virtual void on_order_filled(OrderIndex index) = 0;

 protected:
  IBroker* broker_;
  std::shared_ptr<spdlog::logger> logger_;
};

std::unique_ptr<IStrategy> grid_strategy(IBroker* broker,
                                         std::shared_ptr<spdlog::logger> logger,
                                         int grid_count, double grid_spacing);

}  // namespace wedge