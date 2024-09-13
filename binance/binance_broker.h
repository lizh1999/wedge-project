#pragma once

#include <spdlog/spdlog.h>

#include <memory>

#include "binance/binance_client.h"
#include "common/timer.h"
#include "strategy/broker.h"

namespace wedge {

class BinanceBroker : public IBroker {
 public:
  BinanceBroker(std::string symbol, BinanceClient* client,
                std::shared_ptr<spdlog::logger> logger)
      : last_order_index_(0),
        symbol_(symbol),
        client_(client),
        logger_(logger) {}

  OrderIndex limit_buy_order(double quantity, double price) override;

  OrderIndex limit_sell_order(double quantity, double price) override;

  OrderIndex market_sell_order(double quantity) override { std::abort(); }
  OrderIndex market_buy_order(double quantity) override { std::abort(); }

  void cancel(OrderIndex index) override;

  Account account() override;

  bool query(OrderIndex order);

 private:
  OrderIndex add_order(int64_t index) {
    OrderIndex result{last_order_index_++};
    map_.emplace(result, index);
    return result;
  }

  int last_order_index_;
  std::string symbol_;
  std::unordered_map<OrderIndex, int64_t> map_;
  BinanceClient* client_;
  std::shared_ptr<spdlog::logger> logger_;
};
}  // namespace wedge