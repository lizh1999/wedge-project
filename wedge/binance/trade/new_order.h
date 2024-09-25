#pragma once

#include <iostream>

#include "wedge/binance/http.h"
namespace wedge::trade {

enum class Side {
  kBuy,
  kSell,
};

enum class NewOrderResponseType {
  kAck,
  kResult,
  kFull,
};

enum class TimeInForce {
  kGtc,
  kIoc,
  kFok,
};

class NewOrder : public RequestBuilder<NewOrder> {
 public:
  NewOrder(std::string_view symbol, Side side, std::string_view type)
      : RequestBuilder(http::verb::post, "/api/v3/order", true) {
    static const char* side_str[] = {"BUY", "SELL"};
    add_param("symbol", symbol);
    add_param("side", side_str[static_cast<int>(side)]);
    add_param("type", type);
  }

  using RequestBuilder::credentials;

  NewOrder& time_in_force(TimeInForce value) {
    const char* value_str[] = {"GTC", "IOC", "FOK"};
    return add_param("timeInForce", value_str[static_cast<int>(value)]);
  }

  NewOrder& quantity(double value) {
    return add_param("quantity", std::round(value * 1e5) / 1e5);
  }

  NewOrder& quote_order_qty(double value) {
    return add_param("quoteOrderQty", value);
  }

  NewOrder& price(double value) {
    return add_param("price", std::round(value * 1e2) / 1e2);
  }

  NewOrder& new_client_order_id(std::string_view value) {
    return add_param("newClientOrderId", value);
  }

  NewOrder& stop_price(double value) { return add_param("stopPrice", value); }

  NewOrder& trailing_delta(double value) {
    return add_param("trailingDelta", value);
  }

  NewOrder& iceberg_qty(double value) { return add_param("icebergQty", value); }

  NewOrder& new_order_resp_type(NewOrderResponseType value) {
    const char* value_str[] = {"ACK", "RESULT", "FULL"};
    return add_param("newOrderRespType", value_str[static_cast<int>(value)]);
  }

  NewOrder& recv_window(uint64_t value) {
    return add_param("recvWindow", value);
  }
};

}  // namespace wedge::trade