#pragma once

#include <spdlog/spdlog.h>

#include <memory>

#include "wedge/common/candle.h"
#include "wedge/strategy/broker.h"

namespace wedge {

class IStrategy {
  using LoggerPtr = std::shared_ptr<spdlog::logger>;

 public:
  virtual ~IStrategy() = default;
  virtual void update(const Candle& candle) = 0;
  virtual void on_order_filled(OrderIndex index) = 0;
  void set_broker(IBroker* broker) { broker_ = broker; }
  void set_logger(LoggerPtr logger) { logger_ = logger; }

 protected:
  IBroker* broker_;
  LoggerPtr logger_;
};

std::unique_ptr<IStrategy> grid_strategy(int grid_count, double grid_spacing);

}  // namespace wedge