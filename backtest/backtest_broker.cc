#include "backtest/backtest_broker.h"

#include <cstdio>

#include "common/format.h"

namespace wedge {

OrderIndex BacktestBroker::limit_buy_order(double quantity, double price) {
  int index = context_->add_order(wedge::limit_buy_order(quantity, price));
  println(stdout, "{}th limit buy order quantity {} price {}", index, quantity, price);
  return OrderIndex(index);
}

OrderIndex BacktestBroker::limit_sell_order(double quantity, double price) {
  int index = context_->add_order(wedge::limit_sell_order(quantity, price));
  println(stdout, "{}th limit sell order quantity {} price {}", index, quantity, price);
  return OrderIndex(index);
}

OrderIndex BacktestBroker::market_buy_order(double quantity) {
  int index = context_->add_order(wedge::market_buy_order(quantity));
  return OrderIndex(index);
}

OrderIndex BacktestBroker::market_sell_order(double quantity) {
  int index = context_->add_order(wedge::market_sell_order(quantity));
  return OrderIndex(index);
}

void BacktestBroker::cancel(OrderIndex order_index) {
  context_->cancel(order_index.index());
  println(stdout, "cancel {}th order", order_index.index());
}

const Account& BacktestBroker::account() const { return context_->account(); }

}  // namespace wedge