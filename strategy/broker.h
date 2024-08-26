#pragma once

#include "common/account.h"

namespace wedge {

class OrderIndex {
 public:
  OrderIndex() = default;
  explicit OrderIndex(int index) : index_(index) {}
  int index() const { return index_; }

  auto operator<=>(const OrderIndex& other) const = default;

 private:
  int index_;
};

class IBroker {
 public:
  virtual ~IBroker() = default;
  virtual OrderIndex limit_buy_order(double quanity, double price) = 0;
  virtual OrderIndex limit_sell_order(double quanity, double price) = 0;
  virtual OrderIndex market_buy_order(double quanity) = 0;
  virtual OrderIndex market_sell_order(double quanity) = 0;
  virtual void cancel(OrderIndex index) = 0;
  virtual const Account& account() const = 0;
};

}  // namespace wedge

namespace std {
template <typename T>
class hash;
}

template <>
struct std::hash<wedge::OrderIndex> {
  std::size_t operator()(const wedge::OrderIndex& order) const {
    return std::hash<int>()(order.index());
  }
};