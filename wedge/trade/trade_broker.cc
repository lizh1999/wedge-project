#include "wedge/trade/trade_broker.h"

namespace wedge {

OrderIndex TradeBroker::limit_buy_order(double quantity, double price) {
  uint64_t order_id =
      engine_->broker_.create_limit_buy_order(symbol_, quantity, price);
  return engine_->add_order(order_id);
}

OrderIndex TradeBroker::limit_sell_order(double quantity, double price) {
  uint64_t order_id =
      engine_->broker_.create_limit_sell_order(symbol_, quantity, price);
  return engine_->add_order(order_id);
}
OrderIndex TradeBroker::market_buy_order(double quantity) {
  uint64_t order_id =
      engine_->broker_.create_market_buy_order(symbol_, quantity);
  return engine_->add_order(order_id);
}
OrderIndex TradeBroker::market_sell_order(double quantity) {
  uint64_t order_id =
      engine_->broker_.create_market_sell_order(symbol_, quantity);
  return engine_->add_order(order_id);
}

void TradeBroker::cancel(OrderIndex index) {
  auto iter = engine_->orders_.find(index);
  uint16_t order_id = iter->second;
  engine_->broker_.cancel_order(symbol_, order_id);
  engine_->orders_.erase(iter);
}

Account TradeBroker::account() {
  auto account = engine_->broker_.get_account();
  size_t n = symbol_.size() - 4;  // XXXUSDT
  double balance = *account.free("USDT");
  double position = *account.free(symbol_.substr(0, n));
  return Account(balance, position);
}

}  // namespace wedge