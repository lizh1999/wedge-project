#pragma once

#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>

#include <vector>

#include "wedge/common/candle.h"
#include "wedge/common/enums.h"
#include "wedge/strategy2/broker.h"

namespace wedge {

struct Order {
  uint64_t order_id;
  OrderStatus status;
};

struct ScheduleTaskEvent {
  Candle current_candle;
  std::vector<Order> open_orders;
};

class StrategyBase {
 public:
  StrategyBase(Broker* broker, spdlog::logger* logger)
      : broker_(broker), logger_(logger) {}

  virtual ~StrategyBase() = default;
  virtual void to_json(nlohmann::json& json) = 0;
  virtual void from_json(const nlohmann::json& json) = 0;
  virtual void on_event(const ScheduleTaskEvent& event) = 0;

 protected:
  Broker* broker_;
  spdlog::logger* logger_;
};

}  // namespace wedge