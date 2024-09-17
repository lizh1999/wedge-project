#pragma once

#include "wedge/binance/http.h"

namespace wedge::market {

class Ping : public RequestBuilder<Ping> {
 public:
  Ping() : RequestBuilder(http::verb::get, "/api/v3/ping") {}
};

}