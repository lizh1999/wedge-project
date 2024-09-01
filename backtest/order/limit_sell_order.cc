#include "backtest/backtest_context.h"
#include "backtest/order/order.h"

namespace wedge {

class LimitSellOrder : public IOrder {
 public:
  explicit LimitSellOrder(double quantity, double price)
      : quantity_(quantity), price_(price) {}

  bool update(BacktestContext& context, const Candle& candle) override;

 private:
  double quantity_;
  double price_;
};

std::unique_ptr<IOrder> limit_sell_order(double quantity, double price) {
  return std::make_unique<LimitSellOrder>(quantity, price);
}

bool LimitSellOrder::update(BacktestContext& context, const Candle& candle) {
  if (price_ < candle.high_price) {
    return false;
  }
  return context.execute_sell_order(quantity_, price_);
}

}  // namespace wedge