#pragma once

#include "wedge/binance/http.h"

namespace wedge::trade {

class GetOrder : public RequestBuilder<GetOrder> {
 public:
  GetOrder(std::string_view symbol)
      : RequestBuilder(http::verb::get, "/api/v3/order", true) {
    add_param("symbol", symbol);
  }

  using RequestBuilder::credentials;

  GetOrder& order_id(uint64_t value) { return add_param("orderId", value); }

  GetOrder& orig_client_order_id(std::string_view value) {
    return add_param("origClientOrderId", value);
  }

  GetOrder& recv_window(uint64_t value) { return add_param("recvWindow", value); }
};

}  // namespace wedge::trade