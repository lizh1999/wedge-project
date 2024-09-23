#include "wedge/trade/trade_engine.h"

#include "wedge/common/chrono.h"
#include "wedge/trade/trade_broker.h"

namespace wedge {

void TradeEngine::update_orders() {
  auto iter = orders_.begin();
  while (iter != orders_.end()) {
    if (broker_.get_order(symbol_, iter->second)) {
      strategy_->on_order_filled(iter->first);
      iter = orders_.erase(iter);
    } else {
      ++iter;
    }
  }
}

OrderIndex TradeEngine::add_order(uint64_t order_id) {
  OrderIndex result(last_order_index_++);
  orders_.emplace(result, order_id);
  return result;
}

void TradeEngine::run() {
  TradeBroker broker(symbol_, this);
  strategy_->set_broker(&broker);
  strategy_->set_logger(logger_);
  for (;; std::this_thread::sleep_for(30min)) {
    update_orders();
    auto now = SystemClock::now();
    auto end_time = duration_cast<Milliseconds>(now.time_since_epoch()).count();
    auto start_time =
        duration_cast<Milliseconds>((now - 30min).time_since_epoch()).count();
    auto candles = broker_.get_klines("BTCUSDT", "30m", start_time, end_time);
    strategy_->update(candles.back());
  }
}

}  // namespace wedge