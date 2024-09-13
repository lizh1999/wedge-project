#pragma once

#include "backtest/backtest_context.h"
#include "strategy/broker.h"

namespace wedge {

class BacktestBroker : public IBroker {
 public:
  explicit BacktestBroker(BacktestContext* context) : context_(context) {}

  OrderIndex limit_buy_order(double quantity, double price) override;
  OrderIndex limit_sell_order(double quantity, double price) override;
  OrderIndex market_buy_order(double quantity) override;
  OrderIndex market_sell_order(double quantity) override;
  void cancel(OrderIndex order_index) override;
  Account account() override;

 private:
  BacktestContext* context_;
};

}  // namespace wedge