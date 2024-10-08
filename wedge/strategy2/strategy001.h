#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "wedge/common/candle.h"
#include "wedge/common/enums.h"
#include "wedge/indicator/indicator.h"
#include "wedge/indicator/volatility/range.h"
#include "wedge/strategy2/broker.h"
#include "wedge/strategy2/strategy.h"

namespace wedge {

class Strategy001 final : public StrategyBase {
 public:
  using StrategyBase::StrategyBase;

  void to_json(nlohmann::json& json) override {}

  void from_json(const nlohmann::json& json) override {
    range = std::make_unique<Range>(24);
    k = 0.1;
  }

  void on_event(const ScheduleTaskEvent& event) override {
    for (const auto& list : event.open_order_lists) {
      if (is_new_order(list.orders)) {
        broker_->execute(CancelOrderList{.order_list_id = list.order_list_id});
      }
    }

    Candle candle = event.current_candle;
    range->update(candle);

    double open_price = candle.open_price;
    double limit_price = open_price - range->value() * k;

    broker_->execute(NewOtocoOrderList {
      .working = NewLimitOrder {
        .side = OrderSide::kBuy,
        .price = limit_price,
        .quantity = 1,
      },
      .pending_above = NewStopLossOrder {
        .side = OrderSide::kSell,
        .price = limit_price * (1 - 0.0005),
        .quantity = 1,
      },
      .pending_below = NewLimitOrder {
        .side = OrderSide::kSell,
        .price = limit_price * (1 + 0.0005),
        .quantity = 1,
      }
    });
  }

  static bool is_new_order(const std::vector<Order>& orders) {
    auto is_filled = [](auto& order) {
      return order.status == OrderStatus::kFilled;
    };
    return std::none_of(orders.begin(), orders.end(), is_filled);
  }

  double k;
  std::unique_ptr<Indicator> range;
};

}  // namespace wedge