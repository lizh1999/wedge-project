#pragma once

#include <cstdint>
#include <variant>

#include "wedge/common/enums.h"

namespace wedge {

struct CancelOrder {
  uint64_t order_id;
};

struct CancelOrderList {
  uint64_t order_list_id;
};

struct NewLimitOrder {
  OrderSide side;
  double price;
  double quantity;
};

struct NewMarketOrder {
  OrderSide side;
  double quantity;
};

struct NewStopLossOrder {
  OrderSide side;
  double price;
  double quantity;
};

using NewOrder = std::variant<NewLimitOrder, NewMarketOrder, NewStopLossOrder>;

struct NewOtoOrderList {
  NewOrder working;
  NewOrder pending;
};

struct NewOcoOrderList {
  NewOrder above;
  NewOrder below;
};

struct NewOtocoOrderList {
  NewOrder working;
  NewOrder pending_above;
  NewOrder pending_below;
};

using NewOrderList =
    std::variant<NewOtoOrderList, NewOcoOrderList, NewOtocoOrderList>;

class Broker {
 public:
  virtual ~Broker() = default;
  virtual void execute(const CancelOrder& command) = 0;
  virtual void execute(const CancelOrderList& command) = 0;
  virtual uint64_t execute(const NewOrder& command) = 0;
  virtual uint64_t execute(const NewOrderList& command) = 0;
};

}  // namespace wedge