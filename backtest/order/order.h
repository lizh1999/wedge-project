#pragma once

#include <memory>

#include "common/candle.h"

namespace wedge {

class BacktestContext;

class IOrder {
 public:
  virtual ~IOrder() = default;
  virtual bool update(BacktestContext& context, const Candle& candle) = 0;
};

std::unique_ptr<IOrder> limit_buy_order(double quantity, double price);
std::unique_ptr<IOrder> limit_sell_order(double quantity, double price);
std::unique_ptr<IOrder> market_buy_order(double quantity);
std::unique_ptr<IOrder> market_sell_order(double quantity);

}  // namespace wedge