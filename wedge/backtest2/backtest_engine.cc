#include "wedge/backtest2/backtest_engine.h"

#include <cassert>

namespace wedge {

uint64_t BacktestEngine::add_base_order(std::unique_ptr<OrderBase> order) {
  uint64_t order_id = orders_.size();
  if (order->is_status(OrderStatus::kNew)) {
    new_orders_.push_back(order_id);
  }
  orders_.push_back(std::move(order));
  return order_id;
}

uint64_t BacktestEngine::add_order_list(std::initializer_list<uint64_t> list) {
  uint64_t result = order_lists_.size();
  order_lists_.emplace_back(list.begin(), list.end());
  return result;
}

uint64_t BacktestEngine::add_oco(uint64_t above, uint64_t below) {
  orders_[above]->add_canceled(below);
  orders_[below]->add_canceled(above);
  return add_order_list({above, below});
}

uint64_t BacktestEngine::add_oto(uint64_t working, uint64_t pending) {
  orders_[working]->add_triggered(pending);
  return add_order_list({working, pending});
}

uint64_t BacktestEngine::add_otoco(uint64_t working, uint64_t pending_above,
                                   uint64_t pending_below) {
  orders_[working]->add_triggered(pending_above);
  orders_[working]->add_triggered(pending_below);
  orders_[pending_above]->add_canceled(pending_below);
  orders_[pending_below]->add_canceled(pending_above);
  return add_order_list({working, pending_above, pending_below});
}

void BacktestEngine::on_event(const FillOrderEvent& event) {
  switch (event.side) {
    case OrderSide::kBuy:
      base_ += event.base * (1 - 1e-4);
      quote_ -= event.quote;
      break;
    case OrderSide::kSell:
      base_ -= event.base;
      quote_ += event.quote * (1 - 1e-4);
      break;
  }
}

void BacktestEngine::on_event(const TriggerOrderEvent& event) {
  for (uint64_t order_id : event.orders) {
    assert(orders_[order_id]->is_status(OrderStatus::kPendingNew));
    orders_[order_id]->status(OrderStatus::kNew);
    new_orders_.push_back(order_id);
  }
}

void BacktestEngine::on_event(const CancelOrderEvent& event) {
  for (uint64_t order_id : event.orders) {
    orders_[order_id]->status(OrderStatus::kCanceled);
  }
}

void BacktestEngine::update_new_orders(const Candle& candle) {
  auto iter = new_orders_.begin();
  for (; iter != new_orders_.end(); ++iter) {
    OrderBase* order = orders_[*iter].get();
    if (order->is_status(OrderStatus::kNew)) {
      order->update(candle);
    }
  }
  auto new_end = std::remove_if(
      new_orders_.begin(), new_orders_.end(), [this](uint64_t order_id) {
        return !orders_[order_id]->is_status(OrderStatus::kNew);
      });
  new_orders_.erase(new_end, new_orders_.end());
}

void BacktestEngine::execute(const CancelOrder& command) {
  orders_[command.order_id]->status(OrderStatus::kCanceled);
}

void BacktestEngine::execute(const CancelOrderList& command) {
  for (uint64_t order_id : order_lists_[command.order_list_id]) {
    orders_[order_id]->status(OrderStatus::kCanceled);
  }
}

class OrderExecutor {
 public:
  OrderExecutor(BacktestEngine* self, OrderStatus status)
      : self(self), status(status) {}

  uint64_t operator()(const NewLimitOrder& order) {
    return self->add_order<LimitOrder>(order.side, order.price, order.quantity,
                                       status);
  }

  uint64_t operator()(const NewMarketOrder& order) {
    return self->add_order<MarketOrder>(order.side, order.quantity, status);
  }

  uint64_t operator()(const NewStopLossOrder& order) {
    return self->add_order<StopLossOrder>(order.side, order.price,
                                          order.quantity, status);
  }

 private:
  BacktestEngine* self;
  OrderStatus status;
};

uint64_t BacktestEngine::execute(const NewOrder& command) {
  OrderExecutor executor(this, OrderStatus::kNew);
  return std::visit(executor, command);
}

class OrderListExecutor {
 public:
  OrderListExecutor(BacktestEngine* self) : self(self) {}

  // NewOtoOrderList, NewOcoOrderList, NewOtocoOrderList
  uint64_t operator()(const NewOtoOrderList& list) {
    uint64_t working = execute(list.working, OrderStatus::kNew);
    uint64_t pending = execute(list.pending, OrderStatus::kPendingNew);
    return self->add_oto(working, pending);
  }

  uint64_t operator()(const NewOcoOrderList& list) {
    uint64_t above = execute(list.above, OrderStatus::kNew);
    uint64_t below = execute(list.below, OrderStatus::kPendingNew);
    return self->add_oco(above, below);
  }

  uint64_t operator()(const NewOtocoOrderList& list) {
    uint64_t working = execute(list.working, OrderStatus::kNew);
    uint64_t pending_above =
        execute(list.pending_above, OrderStatus::kPendingNew);
    uint64_t pending_below =
        execute(list.pending_below, OrderStatus::kPendingNew);
    return self->add_otoco(working, pending_above, pending_below);
  }

 private:
  uint64_t execute(const NewOrder& order, OrderStatus status) {
    return std::visit(OrderExecutor{self, status}, order);
  };

  BacktestEngine* self;
};

uint64_t BacktestEngine::execute(const NewOrderList& command) {
  OrderListExecutor executor(this);
  return std::visit(executor, command);
}

}  // namespace wedge