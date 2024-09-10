#pragma once

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <map>
#include <memory>

#include "backtest/data_loader.h"
#include "backtest/order/order.h"
#include "strategy/broker.h"
#include "strategy/strategy.h"

namespace wedge {

class BacktestContext {
 public:
  BacktestContext(double balance, double position, double commission = 0)
      : account_(balance, position), commission_(commission) {}

  void run(std::unique_ptr<IDataLoader> data_loader);
  std::unique_ptr<IBroker> broker();

  bool execute_buy_order(double quantity, double price);
  bool execute_sell_order(double quantity, double price);

  void set_logger(std::shared_ptr<spdlog::logger> logger) { logger_ = logger; }

  void set_strategy(std::unique_ptr<IStrategy> strategy) {
    strategy_ = std::move(strategy);
  }
  Account& account() { return account_; }

 private:
  void update_orders(const Candle& candle);
  int add_order(std::unique_ptr<IOrder> order);
  void cancel(int index) { orders_.erase(index); }

  friend class BacktestBroker;

  Account account_;
  std::unique_ptr<IStrategy> strategy_;
  std::map<int, std::unique_ptr<IOrder>> orders_;
  int last_order_index_ = 0;
  double commission_;

  std::shared_ptr<spdlog::logger> logger_;
};

}  // namespace wedge