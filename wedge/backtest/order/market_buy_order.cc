#include "wedge/backtest/backtest_context.h"
#include "wedge/backtest/order/order.h"

namespace wedge {

class MarketBuyOrder : public IOrder {
 public:
  explicit MarketBuyOrder(double quantity) : quantity_(quantity) {}

  bool update(BacktestContext& context, const Candle& candle) override;

 private:
  double quantity_;
};

std::unique_ptr<IOrder> market_buy_order(double quantity) {
  return std::make_unique<MarketBuyOrder>(quantity);
}

bool MarketBuyOrder::update(BacktestContext& cotnext, const Candle& candle) {
  double market_price = candle.close_price;
  return cotnext.execute_buy_order(quantity_, market_price);
}

}  // namespace wedge