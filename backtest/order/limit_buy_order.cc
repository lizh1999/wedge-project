#include "backtest/backtest_context.h"
#include "backtest/order/order.h"

namespace wedge {

class LimitBuyOrder : public IOrder {
 public:
  explicit LimitBuyOrder(double quanity, double price)
      : quanity_(quanity), price_(price) {}

  bool update(BacktestContext& context, const Candle& candle) override;

 private:
  double quanity_;
  double price_;
};

std::unique_ptr<IOrder> limit_buy_order(double quanity, double price) {
  return std::make_unique<LimitBuyOrder>(quanity, price);
}

bool LimitBuyOrder::update(BacktestContext& context, const Candle& candle) {
  if (price_ < candle.low_price) {
    return false;
  }
  return context.execute_buy_order(quanity_, price_);
}

}  // namespace wedge