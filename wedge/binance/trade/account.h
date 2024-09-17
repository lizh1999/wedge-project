#pragma once

#include "wedge/binance/http.h"

namespace wedge::trade {

class Account : public RequestBuilder<Account> {
 public:
  Account() : RequestBuilder(http::verb::get, "/api/v3/account", true) {}
  using RequestBuilder::credentials;
  Account &recv_window(uint64_t value) { return add_param("recvWindow", value); }
};

}  // namespace wedge::trade