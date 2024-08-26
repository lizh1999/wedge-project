#include "backtest/backtest_context.h"
#include "backtest/order/order.h"

namespace wedge {

class MarketBuyOrder : public IOrder {
 public:
  explicit MarketBuyOrder(double quanity) : quanity_(quanity) {}

  bool update(BacktestContext& context, const Candle& candle) override;

 private:
  double quanity_;
};

std::unique_ptr<IOrder> market_buy_order(double quanity) {
  return std::make_unique<MarketBuyOrder>(quanity);
}

bool MarketBuyOrder::update(BacktestContext& cotnext, const Candle& candle) {
  double market_price = candle.close_price;
  return cotnext.execute_buy_order(quanity_, market_price);
}

}  // namespace wedge