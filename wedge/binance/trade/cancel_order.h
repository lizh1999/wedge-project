#pragma once

#include "wedge/binance/http.h"

namespace wedge::trade {

class CancelOrder : public RequestBuilder<CancelOrder> {
 public:
  CancelOrder(std::string_view symbol)
      : RequestBuilder(http::verb::delete_, "/api/v3/order", true) {
    add_param("symbol", symbol);
  }

  using RequestBuilder::credentials;

  CancelOrder& order_id(uint64_t value) { return add_param("orderId", value); }

  CancelOrder& orig_client_order_id(std::string_view value) {
    return add_param("origClientOrderId", value);
  }

  CancelOrder& new_client_order_id(std::string_view value) {
    return add_param("newClientOrderId", value);
  }

  CancelOrder& recv_window(uint64_t value) { add_param("recvWindow", value); }
};

}  // namespace wedge::trade