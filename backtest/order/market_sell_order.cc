#include "backtest/backtest_context.h"
#include "backtest/order/order.h"

namespace wedge {

class MarketSellOrder : public IOrder {
 public:
  explicit MarketSellOrder(double quanity) : quanity_(quanity) {}

  bool update(BacktestContext& context, const Candle& candle) override;

 private:
  double quanity_;
};

std::unique_ptr<IOrder> market_sell_order(double quanity) {
  return std::make_unique<MarketSellOrder>(quanity);
}

bool MarketSellOrder::update(BacktestContext& context, const Candle& candle) {
  double market_price = candle.close_price;
  return context.execute_sell_order(quanity_, market_price);
}

}  // namespace wedge