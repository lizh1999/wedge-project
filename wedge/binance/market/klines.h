#pragma once

#include "wedge/binance/http.h"

namespace wedge::market {

enum class KlineInterval {
  kMinutes1,
  kMinutes3,
  kMinutes5,
  kMinutes15,
  kMinutes30,
  kHours1,
  kHours2,
  kHours4,
  kHours6,
  kHours8,
  kHours12,
  kDays1,
  kDays3,
  kWeeks1,
  kMonths1,
};

inline KlineInterval from_str(std::string_view str) {
  const char* interval_str[] = {"1m", "3m", "5m", "15m",  "30m", "1h",
                                "2h", "4h", "6h", "8h",   "12h", "1d",
                                "3d", "1w", "1M", nullptr};
  int index = 0;
  while (interval_str[index] && str != interval_str[index]) index++;
  return static_cast<KlineInterval>(index);
}

class Klines : public RequestBuilder<Klines> {
 public:
  Klines(std::string_view symbol, KlineInterval interval)
      : RequestBuilder(http::verb::get, "/api/v3/klines") {
    const char* interval_str[] = {"1m",  "3m", "5m", "15m", "30m",
                                  "1h",  "2h", "4h", "6h",  "8h",
                                  "12h", "1d", "3d", "1w",  "1M"};
    add_param("symbol", symbol);
    add_param("interval", interval_str[static_cast<int>(interval)]);
  }

  Klines& start_time(int64_t value) { return add_param("startTime", value); }

  Klines& end_time(int64_t value) { return add_param("endTime", value); }

  Klines& limit(uint32_t value) { return add_param("limit", value); }
};

}  // namespace wedge::market