#include "backtest/backtest_context.h"
#include "backtest/order/order.h"

namespace wedge {

class LimitSellOrder : public IOrder {
 public:
  explicit LimitSellOrder(double quanity, double price)
      : quanity_(quanity), price_(price) {}

  bool update(BacktestContext& context, const Candle& candle) override;

 private:
  double quanity_;
  double price_;
};

std::unique_ptr<IOrder> limit_sell_order(double quanity, double price) {
  return std::make_unique<LimitSellOrder>(quanity, price);
}

bool LimitSellOrder::update(BacktestContext& context, const Candle& candle) {
  if (price_ < candle.high_price) {
    return false;
  }
  return context.execute_sell_order(quanity_, price_);
}

}  // namespace wedge