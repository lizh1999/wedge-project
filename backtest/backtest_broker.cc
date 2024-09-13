#include "backtest/backtest_broker.h"

#include "common/format.h"
#include "spdlog/spdlog.h"

namespace wedge {

OrderIndex BacktestBroker::limit_buy_order(double quantity, double price) {
  int index = context_->add_order(wedge::limit_buy_order(quantity, price));
  context_->logger_->trace("{}th LBO quantity {} price {}", index, quantity,
                           price);
  return OrderIndex(index);
}

OrderIndex BacktestBroker::limit_sell_order(double quantity, double price) {
  int index = context_->add_order(wedge::limit_sell_order(quantity, price));
  context_->logger_->trace("{}th LSO quantity {} price {}", index, quantity,
                           price);
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
  context_->logger_->trace("cacel {}th order", order_index.index());
}

Account BacktestBroker::account() { return context_->account(); }

}  // namespace wedge