#include "binance/binance_broker.h"

#include <unordered_map>

namespace wedge {

OrderIndex BinanceBroker::limit_buy_order(double quantity, double price) {
  auto result = client_->buy_limit_order(symbol_, quantity, price);

  int count = 0;
  while (!result.has_value()) {
    logger_->warn("limit buy order error {}", result.error().message());
    std::this_thread::sleep_for(1s);
    result = client_->buy_limit_order(symbol_, quantity, price);

    if (++count == 0)
      std::abort();
  }
  return add_order(result.value());
}

OrderIndex BinanceBroker::limit_sell_order(double quantity, double price) {
  auto result = client_->buy_limit_order(symbol_, quantity, price);

  int count = 0;
  while (!result.has_value()) {
    logger_->warn("limit sell order error {}", result.error().message());
    std::this_thread::sleep_for(1s);
    result = client_->sell_limit_order(symbol_, quantity, price);

    if (++count == 0)
      std::abort();
  }
  return add_order(result.value());
}

void BinanceBroker::cancel(OrderIndex index) {
  auto iter = map_.find(index);
  if (iter == map_.end()) {
    std::abort();
  }
  auto result = client_->cancel_order(symbol_, iter->second);

  int count = 0;
  while (!result.has_value()) {
    logger_->warn("cancel order error {}", result.error().message());
    std::this_thread::sleep_for(1s);
    result = client_->cancel_order(symbol_, iter->second);
    if (++count == 3)
      std::abort();
  }
  map_.erase(iter);
}

Account BinanceBroker::account() {
  auto result = client_->get_account();
  int count = 0;
  while (!result.has_value()) {
    logger_->warn("get account error {}", result.error().message());
    std::this_thread::sleep_for(1s);
    result = client_->get_account();
    if (++count == 3)
      std::abort();
  }

  double usdt;
  double target;
  for (auto& balance : result.value()) {
    if (balance.asset == "USDT") {
      usdt = balance.free;
    } else if (balance.asset == symbol_) {
      target = balance.free;
    }
  }
  return Account(usdt, target);
}

bool BinanceBroker::query(OrderIndex index) {
  auto iter = map_.find(index);
  if (iter == map_.end())
    std::abort();

  auto result = client_->query_order(symbol_, iter->second);

  int count = 0;
  while (!result.has_value()) {
    logger_->warn("cancel order error {}", result.error().message());
    std::this_thread::sleep_for(1s);
    result = client_->query_order(symbol_, iter->second);
    if (++count == 3)
      std::abort();
  }

  if (result.value() != OrderState::kCanceled)
    return false;
  
  map_.erase(iter);
  return true;
}

}  // namespace wedge