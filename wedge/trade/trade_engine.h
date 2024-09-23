#pragma once

#include <spdlog/spdlog.h>

#include <chrono>
#include <memory>
#include <thread>
#include <unordered_map>

#include "wedge/binance/binance_broker.h"
#include "wedge/common/candle.h"
#include "wedge/strategy/broker.h"
#include "wedge/strategy/strategy.h"

namespace wedge {

class TradeEngine {
 public:
  TradeEngine(std::string symbol, const Credentials &credentials,
              std::shared_ptr<spdlog::logger> logger)
      : symbol_(symbol), logger_(logger), broker_(logger, credentials) {}

  void run();

  void set_strategy(std::unique_ptr<IStrategy> strategy) {
    strategy_ = std::move(strategy);
  }

 private:
  friend class TradeBroker;

  void update_orders();
  OrderIndex add_order(uint64_t order_id);

  std::string symbol_;
  std::shared_ptr<spdlog::logger> logger_;
  BinanceBroker broker_;
  std::unique_ptr<IStrategy> strategy_;
  std::unordered_map<OrderIndex, uint64_t> orders_;
  int last_order_index_ = 0;
};

}  // namespace wedge
