#include "wedge/backtest/backtest_context.h"
#include "wedge/backtest/order/order.h"

namespace wedge {

class MarketSellOrder : public IOrder {
 public:
  explicit MarketSellOrder(double quantity) : quantity_(quantity) {}

  bool update(BacktestContext& context, const Candle& candle) override;

 private:
  double quantity_;
};

std::unique_ptr<IOrder> market_sell_order(double quantity) {
  return std::make_unique<MarketSellOrder>(quantity);
}

bool MarketSellOrder::update(BacktestContext& context, const Candle& candle) {
  double market_price = candle.close_price;
  return context.execute_sell_order(quantity_, market_price);
}

}  // namespace wedge