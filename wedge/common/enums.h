#pragma once

namespace wedge {
enum class OrderSide {
  kBuy,
  kSell,
};

enum class OrderStatus {
  kNew,
  kPendingNew,
  kPartiallyFilled,
  kFilled,
  kCanceled,
};
}  // namespace wedge