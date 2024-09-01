#include "backtest/backtest_context.h"
#include "backtest/order/order.h"

namespace wedge {

class LimitBuyOrder : public IOrder {
 public:
  explicit LimitBuyOrder(double quantity, double price)
      : quantity_(quantity), price_(price) {}

  bool update(BacktestContext& context, const Candle& candle) override;

 private:
  double quantity_;
  double price_;
};

std::unique_ptr<IOrder> limit_buy_order(double quantity, double price) {
  return std::make_unique<LimitBuyOrder>(quantity, price);
}

bool LimitBuyOrder::update(BacktestContext& context, const Candle& candle) {
  if (price_ < candle.low_price) {
    return false;
  }
  return context.execute_buy_order(quantity_, price_);
}

}  // namespace wedge