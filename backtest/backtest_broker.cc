#include "backtest/backtest_broker.h"

#include <format>
#include <iostream>

namespace wedge {

OrderIndex BacktestBroker::limit_buy_order(double quanity, double price) {
  int index = context_->add_order(wedge::limit_buy_order(quanity, price));
  std::println(std::cout, "{}th limit buy order quanity {} price {}", index, quanity, price);
  return OrderIndex(index);
}

OrderIndex BacktestBroker::limit_sell_order(double quanity, double price) {
  int index = context_->add_order(wedge::limit_sell_order(quanity, price));
  std::println(std::cout, "{}th limit sell order quanity {} price {}", index, quanity, price);
  return OrderIndex(index);
}

OrderIndex BacktestBroker::market_buy_order(double quanity) {
  int index = context_->add_order(wedge::market_buy_order(quanity));
  return OrderIndex(index);
}

OrderIndex BacktestBroker::market_sell_order(double quanity) {
  int index = context_->add_order(wedge::market_sell_order(quanity));
  return OrderIndex(index);
}

void BacktestBroker::cancel(OrderIndex order_index) {
  context_->cancel(order_index.index());
  std::println(std::cout, "cancel {}th order", order_index.index());
}

const Account& BacktestBroker::account() const { return context_->account(); }

}  // namespace wedge