#pragma once

#include <spdlog/spdlog.h>

#include "wedge/binance/binance_broker.h"
#include "wedge/common/candle.h"
#include "wedge/strategy/broker.h"
#include "wedge/trade/trade_engine.h"

namespace wedge {

class TradeBroker : public IBroker {
 public:
  TradeBroker(const std::string &symbol, TradeEngine *engine)
      : symbol_(symbol), engine_(engine) {}

  OrderIndex limit_buy_order(double quantity, double price) override;
  OrderIndex limit_sell_order(double quantity, double price) override;
  OrderIndex market_buy_order(double quantity) override;
  OrderIndex market_sell_order(double quantity) override;
  void cancel(OrderIndex index) override;
  Account account() override;

 private:
  std::string symbol_;
  TradeEngine *engine_;
};

}  // namespace wedge