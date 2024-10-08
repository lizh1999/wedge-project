#include "wedge/backtest2/order.h"

namespace wedge {

void OrderBase::status(OrderStatus new_status) {
  status_ = new_status;
  if (new_status != OrderStatus::kFilled) {
    return;
  }
  if (!triggered_.empty()) {
    TriggerOrderEvent event{
        .orders = triggered_,
    };
    listener_->on_event(event);
  }
  if (!canceled_.empty()) {
    CancelOrderEvent event{
        .orders = canceled_,
    };
    listener_->on_event(event);
  }
}

void LimitOrder::update(const Candle& candle) {
  if (side_ == OrderSide::kBuy && candle.low_price <= price_) {
    FillOrderEvent event{
        .side = OrderSide::kBuy,
        .base = quantity_,
        .quote = quantity_ * price_,
    };
    listener_->on_event(event);
    this->status(OrderStatus::kFilled);
  }
  if (side_ == OrderSide::kSell && price_ <= candle.high_price) {
    FillOrderEvent event{
        .side = OrderSide::kSell,
        .base = quantity_,
        .quote = quantity_ * price_,
    };
    listener_->on_event(event);
    this->status(OrderStatus::kFilled);
  }
}

void MarketOrder::update(const Candle& candle) {
  FillOrderEvent event{
      .side = side_,
      .base = quantity_,
      .quote = quantity_ * candle.close_price,
  };
  listener_->on_event(event);
  this->status(OrderStatus::kFilled);
}

void StopLossOrder::update(const Candle& candle) {
  if (side_ == OrderSide::kSell && candle.low_price <= price_) {
    FillOrderEvent event{
        .side = OrderSide::kSell,
        .base = quantity_,
        .quote = quantity_ * price_,
    };
    listener_->on_event(event);
    this->status(OrderStatus::kFilled);
  }
  if (side_ == OrderSide::kBuy && price_ <= candle.high_price) {
    FillOrderEvent event{
        .side = OrderSide::kBuy,
        .base = quantity_,
        .quote = quantity_ * price_,
    };
    listener_->on_event(event);
    this->status(OrderStatus::kFilled);
  }
}

}  // namespace wedge