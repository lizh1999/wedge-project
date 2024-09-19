#include "wedge/backtest/backtest_context.h"

#include <fmt/chrono.h>

#include <cassert>
#include <cstdio>
#include <optional>

#include "wedge/backtest/backtest_broker.h"
#include "wedge/dataset/sql_iterator.h"

namespace wedge {

std::unique_ptr<IBroker> BacktestContext::broker() {
  return std::make_unique<BacktestBroker>(this);
}

int BacktestContext::add_order(std::unique_ptr<IOrder> order) {
  int index = last_order_index_++;
  orders_.emplace(index, std::move(order));
  return index;
}

static std::string convert_unix_timestamp_ms(int64_t timestamp_ms) {
  std::chrono::milliseconds ms(timestamp_ms);
  std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(ms);
  std::time_t time_t_value = s.count();
  std::tm* tm = std::localtime(&time_t_value);
  return fmt::format("{:%Y-%m-%d %H:%M:%S}", *tm);
}

void BacktestContext::update_orders(const Candle& candle) {
  bool should_log = false;
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
      should_log = true;
    }
  }
  if (!should_log) return;
  logger_->info(
      "{} balance {:.4f} value {:.4f} price {:.4f} position {:.4f}e-5",
      convert_unix_timestamp_ms(candle.close_time), account_.balance(),
      account_.balance() + account_.position() * candle.close_price,
      candle.close_price, account_.position() * 1e5);
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
  TimePoint open_time = SystemClock::from_time_t(candle.open_time / 1000);
  TimePoint close_time = SystemClock::from_time_t(candle.close_time / 1000);
  return duration_cast<Minutes>(close_time - open_time);
}

void BacktestContext::run(SqlIterator data_loader) {
  Minutes duration = 1min;
  bool has_setup = false;
  std::optional<Candle> iterator;
  do {
    std::optional<Candle> candle;
    while ((iterator = data_loader.next())) {
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
        duration = strategy_->setup();
        has_setup = true;
      } else {
        duration = strategy_->update(*candle);
      }
    }
  } while (iterator.has_value());
}

bool BacktestContext::execute_buy_order(double quantity, double price) {
  double total_cost = quantity * price;
  if (account_.balance() < total_cost) {
    return false;
  }
  account_.update_balance(-total_cost);
  account_.update_position(quantity * (1 - commission_));
  logger_->info("buy order cost {:.4f} with price {:.4f}", -total_cost, price);
  return true;
}

bool BacktestContext::execute_sell_order(double quantity, double price) {
  if (account_.position() < quantity) {
    return false;
  }
  double total_income = quantity * price;
  account_.update_balance(total_income * (1 - commission_));
  account_.update_position(-quantity);
  logger_->info("sell order income {:.4f} with price {:.4f}", total_income,
                price);
  return true;
}

}  // namespace wedge