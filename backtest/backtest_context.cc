#include "backtest/backtest_context.h"

#include <cassert>
#include <cstdio>

#include "backtest/backtest_broker.h"
#include "common/format.h"

namespace wedge {

std::unique_ptr<IBroker> BacktestContext::broker() {
  return std::make_unique<BacktestBroker>(this);
}

int BacktestContext::add_order(std::unique_ptr<IOrder> order) {
  int index = last_order_index_++;
  orders_.emplace(index, std::move(order));
  return index;
}

void BacktestContext::update_orders(const Candle& candle) {
  auto iterator = orders_.begin();
  int last_index = last_order_index_;
  while (iterator != orders_.end() && iterator->first < last_index) {
    OrderIndex order_index{iterator->first};
    std::unique_ptr<IOrder>& order = iterator->second;
    if (!order->update(*this, candle)) {
      ++iterator;
    } else {
      strategy_->on_order_filled(order_index);
      iterator = orders_.erase(iterator);
    }
  }
}

static Candle merge(const Candle& previous, const Candle& current) {
  if (previous.volume == 0) {
    return current;
  }
  if (current.volume == 0) {
    return previous;
  }

  assert(previous.close_time == current.open_time);

  Candle result;
  result.open_time = previous.open_time;
  result.close_time = current.close_time;
  result.open_price = previous.open_price;
  result.close_price = previous.close_price;
  result.high_price = std::max(previous.high_price, current.high_price);
  result.low_price = std::min(previous.low_price, current.low_price);
  result.volume = previous.volume + current.volume;
  result.quote_volume = previous.quote_volume + current.quote_volume;
  result.traders = previous.traders + current.traders;
  result.taker_buy_base = previous.taker_buy_base + current.taker_buy_base;
  result.taker_buy_quote = previous.taker_buy_quote + current.taker_buy_quote;
  return result;
}

static Minutes duration_of(const Candle& candle) {
  TimePoint open_time = SystemClock::from_time_t(candle.open_time);
  TimePoint close_time = SystemClock::from_time_t(candle.close_time);
  return duration_cast<Minutes>(close_time - open_time);
}

void BacktestContext::run(std::unique_ptr<IDataLoader> data_loader) {
  Minutes duration = 1min;
  bool has_setup = false;
  optional<Candle> iterator;
  do {
    optional<Candle> candle;
    while ((iterator = data_loader->next())) {
      if (!candle.has_value()) {
        candle = iterator;
      } else {
        candle = merge(*candle, *iterator);
      }
      update_orders(*iterator);
      if (duration <= duration_of(*candle)) {
        break;
      }
    }
    if (candle.has_value()) {
      if (!has_setup) {
        duration = strategy_->setup(*candle);
        has_setup = true;
      } else {
        duration = strategy_->update(*candle);
      }
      println(stdout, "balance {} position {} value {} price {}",
              account_.balance(), account_.position(),
              account_.balance() + account_.position() * candle->close_price,
              candle->close_price);
    }
  } while (iterator.has_value());
}

bool BacktestContext::execute_buy_order(double quanity, double price) {
  double total_cost = quanity * price;
  if (account_.balance() < total_cost) {
    return false;
  }
  account_.update_balance(-total_cost);
  account_.update_position(quanity);
  println(stdout, "buy order cost {}", -total_cost);
  return true;
}

bool BacktestContext::execute_sell_order(double quanity, double price) {
  if (account_.position() < quanity) {
    return false;
  }
  double total_income = quanity * price;
  account_.update_balance(total_income);
  account_.update_position(-quanity);
  println(stdout, "sell order income {}", total_income);
  return true;
}

}  // namespace wedge