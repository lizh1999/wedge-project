#pragma once

#include <span>
#include <vector>

#include "wedge/common/candle.h"
#include "wedge/common/enums.h"

namespace wedge {

struct FillOrderEvent {
  OrderSide side;
  double base;
  double quote;
};

struct TriggerOrderEvent {
  std::span<uint64_t> orders;
};

struct CancelOrderEvent {
  std::span<uint64_t> orders;
};

class OrderListener {
 public:
  virtual ~OrderListener() = default;
  virtual void on_event(const FillOrderEvent& event) = 0;
  virtual void on_event(const TriggerOrderEvent& event) = 0;
  virtual void on_event(const CancelOrderEvent& event) = 0;
};

class OrderBase {
 public:
  OrderBase(OrderListener* listener, OrderSide side, OrderStatus status)
      : listener_(listener), side_(side), status_(status) {}

  virtual ~OrderBase() = default;
  virtual void update(const Candle& candle) = 0;
  OrderStatus status() const { return status_; }
  bool is_status(OrderStatus status) const { return status == status_; }
  void status(OrderStatus status);

  void add_triggered(uint64_t order_id) { triggered_.push_back(order_id); }

  void add_canceled(uint64_t order_id) { canceled_.push_back(order_id); }

 protected:
  OrderListener* listener_;
  OrderSide side_;

 private:
  OrderStatus status_;
  std::vector<uint64_t> triggered_;
  std::vector<uint64_t> canceled_;
};

class LimitOrder final : public OrderBase {
 public:
  LimitOrder(OrderListener* listener, OrderSide side, double price,
             double quantity, OrderStatus status)
      : OrderBase(listener, side, status), price_(price), quantity_(quantity) {}
  void update(const Candle& candle) override;

 private:
  double price_;
  double quantity_;
};

class MarketOrder final : public OrderBase {
 public:
  MarketOrder(OrderListener* listener, OrderSide side, double quantity,
              OrderStatus status)
      : OrderBase(listener, side, status), quantity_(quantity) {}
  void update(const Candle& candle) override;

 private:
  double quantity_;
};

class StopLossOrder final : public OrderBase {
 public:
  StopLossOrder(OrderListener* listener, OrderSide side, double price,
                double quantity, OrderStatus status)
      : OrderBase(listener, side, status), price_(price), quantity_(quantity) {}
  void update(const Candle& candle) override;

 private:
  double price_;
  double quantity_;
};

}  // namespace wedge