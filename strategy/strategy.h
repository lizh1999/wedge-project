#pragma once

#include <spdlog/spdlog.h>

#include <memory>

#include "common/candle.h"
#include "common/chrono.h"
#include "strategy/broker.h"

namespace wedge {

class IStrategy {
 public:
  explicit IStrategy(std::unique_ptr<IBroker> broker,
                     std::shared_ptr<spdlog::logger> logger)
      : broker_(std::move(broker)), logger_(logger) {}

  virtual ~IStrategy() = default;
  virtual Minutes setup(const Candle& candle) = 0;
  virtual Minutes update(const Candle& candle) = 0;
  virtual void on_order_filled(OrderIndex index) = 0;

 protected:
  std::unique_ptr<IBroker> broker_;
  std::shared_ptr<spdlog::logger> logger_;
};

std::unique_ptr<IStrategy> grid_strategy(std::unique_ptr<IBroker> broker,
                                         std::shared_ptr<spdlog::logger> logger,
                                         int grid_count, double grid_spacing);

}  // namespace wedge