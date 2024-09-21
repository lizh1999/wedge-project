#include <cassert>
#include <cmath>
#include <deque>
#include <limits>
#include <optional>

#include "wedge/strategy/strategy.h"

namespace wedge {

class RelativeStrengthIndex {
 public:
  RelativeStrengthIndex(int period)
      : period_(period), sum_loss_(0), sum_gain_(0) {}

  void update(const Candle& candle) {
    double change = candle.close_price - candle.open_price;
    double gain = (change > 0) ? change : 0;
    double loss = (change < 0) ? -change : 0;

    gains_.push_back(gain);
    losses_.push_back(loss);

    sum_gain_ += gain;
    sum_loss_ += loss;

    if (gains_.size() > period_) {
      sum_gain_ -= gains_.front();
      sum_loss_ -= losses_.front();
      gains_.pop_front();
      losses_.pop_front();
    }
  }

  std::optional<double> value() {
    if (period_ != gains_.size()) {
      return std::nullopt;
    }
    double rs = sum_gain_ / sum_loss_;
    return 100. - (100. / (1 + rs));
  }

 private:
  int period_;
  double sum_loss_;
  double sum_gain_;
  std::deque<double> gains_;
  std::deque<double> losses_;
};

struct OrderInfo {
  OrderIndex index;
  double price;
};

class GridStrategy : public IStrategy {
 public:
  GridStrategy(int grid_count, double grid_spacing)
      : baseline_price_(0),
        grid_count_(grid_count),
        grid_spacing_(grid_spacing),
        index_(30) {}

  void update(const Candle& candle) override {
    index_.update(candle);
    auto value = index_.value();

    if (!value) {
      return;
    }
    if (*value < 20) {
      cancel_all_orders();
      return;
    }

    if (!sell_order_ && !buy_order_) {
      baseline_price_ = candle.close_price;
      logger_->warn("place initial order");
      place_initial_orders(candle.close_price);
      return;
    }

    double price = candle.close_price;
    double bound = baseline_price_ * grid_spacing_ * grid_count_;
    double lower_bound = baseline_price_ - bound;
    double upper_bound = baseline_price_ + bound;
    if (price < lower_bound || upper_bound < price) {
      baseline_price_ = price;
      cancel_all_orders();
      place_initial_orders(candle.close_price);
      logger_->warn("rebalance");
    }
    return;
  }

  int order_balance = 0;
  void on_order_filled(OrderIndex index) override {
    double filled_price = std::numeric_limits<double>::quiet_NaN();
    if (buy_order_ && buy_order_->index == index) {
      filled_price = buy_order_->price;
      buy_order_ = std::nullopt;
      order_balance++;
    }
    if (sell_order_ && sell_order_->index == index) {
      filled_price = sell_order_->price;
      sell_order_ = std::nullopt;
      order_balance--;
    }
    logger_->info("order balance {}", order_balance);
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

    order_volume_ = target_position / grid_count_;
    order_balance = 0;

    // 这里需要考虑币安的最小交易额的要求
    if (std::abs(target_position - position) * current_price < 5) {
      place_buy_order(current_price, order_volume_);
      return;
    }

    if (target_position < position) {
      place_sell_order(current_price, position - target_position);
    } else {
      place_buy_order(current_price, target_position - position);
    }
  }

  void cancel_all_orders() {
    if (buy_order_.has_value()) {
      broker_->cancel(buy_order_->index);
      buy_order_ = std::nullopt;
    }
    if (sell_order_.has_value()) {
      broker_->cancel(sell_order_->index);
      sell_order_ = std::nullopt;
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
    float spacing = grid_spacing_ * baseline_price_;
    place_sell_order(filled_price + spacing, order_volume_);
    place_buy_order(filled_price - spacing, order_volume_);
  }

  double baseline_price_;
  int grid_count_;
  double order_volume_;
  double grid_spacing_;

  std::optional<OrderInfo> buy_order_;
  std::optional<OrderInfo> sell_order_;
  RelativeStrengthIndex index_;
};

std::unique_ptr<IStrategy> grid_strategy(int grid_count, double grid_spacing) {
  return std::make_unique<GridStrategy>(grid_count, grid_spacing);
}

}  // namespace wedge