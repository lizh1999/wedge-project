#pragma once

#include <deque>
#include <memory>
#include <vector>

#include "wedge/backtest2/order.h"
#include "wedge/strategy2/broker.h"
#include "wedge/strategy2/strategy.h"

namespace wedge {

class BacktestEngine final : public OrderListener, public Broker {
 public:
  BacktestEngine(double base, double quote) : base_(base), quote_(quote) {}

  void run(StrategyBase* strategy);

  template <class OrderType, class... Args>
  uint64_t add_order(Args&&... args) {
    auto order = std::make_unique<OrderType>(this, args...);
    return add_base_order(std::move(order));
  }

  uint64_t add_oco(uint64_t above, uint64_t below);
  uint64_t add_oto(uint64_t working, uint64_t pending);
  uint64_t add_otoco(uint64_t working, uint64_t pending_above,
                     uint64_t pending_below);

  // OrderListener interface
  void on_event(const FillOrderEvent& event) override;
  void on_event(const TriggerOrderEvent& event) override;
  void on_event(const CancelOrderEvent& event) override;

  // Broker interface
  void execute(const CancelOrder& command) override;
  void execute(const CancelOrderList& command) override;
  uint64_t execute(const NewOrder& command) override;
  uint64_t execute(const NewOrderList& command) override;

 private:
  uint64_t add_order_list(std::initializer_list<uint64_t> list);
  uint64_t add_base_order(std::unique_ptr<OrderBase> order);
  void update_new_orders(const Candle& candle);

  double base_;
  double quote_;
  std::deque<uint64_t> new_orders_;
  std::vector<std::vector<uint64_t>> order_lists_;
  std::vector<std::unique_ptr<OrderBase>> orders_;
};

}  // namespace wedge