#include <cassert>
#include <cmath>
#include <limits>
#include <unordered_map>

#include "common/optional.h"
#include "strategy/strategy.h"

namespace wedge {

struct OrderInfo {
  OrderIndex index;
  double price;
};

class GridStrategy : public IStrategy {
 public:
  GridStrategy(std::unique_ptr<IBroker> broker,
               std::shared_ptr<spdlog::logger> logger, int grid_count,
               double order_volume, double grid_spacing)
      : IStrategy(std::move(broker), logger),
        baseline_price_(0),
        grid_count_(grid_count),
        order_volume_(order_volume),
        grid_spacing_(grid_spacing) {}

  Minutes setup(const Candle& candle) override {
    baseline_price_ = candle.close_price;
    place_initial_orders(candle.close_price);
    return 1min;
  }

  Minutes update(const Candle& candle) override {
    double price = candle.close_price;
    double lower_bound = baseline_price_ - grid_spacing_ * grid_count_;
    double upper_bound = baseline_price_ + grid_spacing_ * grid_count_;
    if (price < lower_bound || upper_bound < price) {
      baseline_price_ = price;
      cancel_all_orders();
      place_initial_orders(candle.close_price);
      logger_->warn("rebalance");
    }
    return 30min;
  }

  void on_order_filled(OrderIndex index) override {
    double filled_price = std::numeric_limits<double>::quiet_NaN();
    if (buy_order_ && buy_order_->index == index) {
      filled_price = buy_order_->price;
      buy_order_ = nullopt;
    }
    if (sell_order_ && sell_order_->index == index) {
      filled_price = sell_order_->price;
      sell_order_ = nullopt;
    }
    assert(!std::isnan(filled_price));
    cancel_all_orders();
    place_order(filled_price);
  }

 private:
  void place_initial_orders(double current_price) {
    const Account& account = broker_->account();
    double balance = account.balance();
    double position = account.position();
    double total_value = balance + position * current_price;
    double target_position = total_value / current_price / 2;
    if (target_position < position) {
      place_sell_order(current_price, position - target_position);
    } else {
      place_buy_order(current_price, target_position - position);
    }
  }

  void cancel_all_orders() {
    if (buy_order_.has_value()) {
      broker_->cancel(buy_order_->index);
      buy_order_ = nullopt;
    }
    if (sell_order_.has_value()) {
      broker_->cancel(sell_order_->index);
      sell_order_ = nullopt;
    }
  }

  void place_buy_order(double price, double volume) {
    assert(!buy_order_.has_value());
    OrderIndex order_id = broker_->limit_buy_order(volume, price);
    buy_order_ = OrderInfo{order_id, price};
  }

  void place_sell_order(double price, double volume) {
    assert(!sell_order_.has_value());
    OrderIndex order_id = broker_->limit_sell_order(volume, price);
    sell_order_ = OrderInfo{order_id, price};
  }

  void place_order(double filled_price) {
    place_sell_order(filled_price + grid_spacing_, order_volume_);
    place_buy_order(filled_price - grid_spacing_, order_volume_);
  }

  double baseline_price_;
  int grid_count_;
  double order_volume_;
  double grid_spacing_;

  optional<OrderInfo> buy_order_;
  optional<OrderInfo> sell_order_;
};

std::unique_ptr<IStrategy> grid_strategy(std::unique_ptr<IBroker> broker,
                                         std::shared_ptr<spdlog::logger> logger,
                                         int grid_count, double order_volume,
                                         double grid_spacing) {
  return std::make_unique<GridStrategy>(std::move(broker), logger, grid_count,
                                        order_volume, grid_spacing);
}

}  // namespace wedge