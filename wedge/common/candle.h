#pragma once

#include <cstdint>

namespace wedge {

struct Candle {
  int64_t open_time;       // 开盘时间
  int64_t close_time;      // 收盘时间
  double open_price;       // 开盘价
  double close_price;      // 收盘价
  double high_price;       // 最高价
  double low_price;        // 最低价
  double volume;           // 成交量
  double quote_volume;     // 报价资产成交量
  int traders;             // 交易笔数
  double taker_buy_base;   // 主动买入成交量 (基础资产)
  double taker_buy_quote;  // 主动买入成交额 (报价资产)
};

}  // namespace wedge