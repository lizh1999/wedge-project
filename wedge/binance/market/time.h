#pragma once

#include "wedge/binance/http.h"

namespace wedge::market {

class Time : public RequestBuilder<Time> {
 public:
  Time() : RequestBuilder(http::verb::get, "/api/v3/time") {}
};

}