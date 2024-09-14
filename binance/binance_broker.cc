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
  auto result = run<BinanceAccount>("get account", [&] {
    return client_->get_account();
  });
  double usdt = result.get("USDT");
  double target = result.get(symbol_);
  return Account(usdt, target);
}

bool BinanceBroker::query(OrderIndex index) {
  auto iter = map_.find(index);
  if (iter == map_.end())
    std::abort();

  auto result = run<OrderState>("cancel order", [&] {
    return client_->query_order(symbol_, iter->second);
  });

  if (result != OrderState::kFilled)
    return false;
  
  map_.erase(iter);
  return true;
}

template <class Ret, class Func>
Ret BinanceBroker::run(const char *name, Func &&func) {
  const int kRedoNumber = 3;
  for (int i = 0; i < kRedoNumber; i++) {
    auto result = func();
    if (result.has_value())
      return result.value();
    logger_->warn("{} error {}", name, result.error().message());
    std::this_thread::sleep_for(1s);
  }
  std::abort();
}

Candle BinanceBroker::kline(int64_t start_time, int64_t end_time, const std::string &interval) {
  auto result = run<std::vector<Candle>>("kline", [&] {
    return client_->kline(symbol_, start_time, end_time, interval);
  });
  if (result.size() != 1) 
    std::abort();
  return result[0];
}

}  // namespace wedge