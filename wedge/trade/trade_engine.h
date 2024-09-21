#pragma once

#include <spdlog/spdlog.h>

#include <chrono>
#include <memory>
#include <thread>
#include <unordered_map>

#include "wedge/binance/binance_http_client.h"
#include "wedge/common/candle.h"
#include "wedge/strategy/broker.h"
#include "wedge/strategy/strategy.h"

namespace wedge {

class TradeEngine {
 public:
  TradeEngine(std::shared_ptr<spdlog::logger> logger) : logger_(logger) {}

  void run();

  void set_strategy(std::unique_ptr<IStrategy> strategy) {
    strategy_ = std::move(strategy);
  }

 private:
  bool update_orders(BinanceHttpClient &client);
  std::optional<Candle> get_kline(BinanceHttpClient &client);
  std::optional<Account> get_account(BinanceHttpClient &client);

  std::string symbol_;
  std::unique_ptr<IStrategy> strategy_;
  std::unordered_map<OrderIndex, uint64_t> orders_;
  int last_order_index_ = 0;
  std::shared_ptr<spdlog::logger> logger_;
};

}  // namespace wedge
