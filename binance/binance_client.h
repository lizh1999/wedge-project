#pragma once

#include "common/candle.h"
#include "common/error.h"
#include "network/rest_api.h"

namespace wedge {

enum class OrderState {
  kNew,              // 新订单
  kPartiallyFilled,  // 部分成交
  kFilled,           // 完全成交
  kCanceled,         // 订单已取消
  kPendingCancel,    // 订单撤销动作并未完成
  kRecjected,        // 订单被拒绝
  kExpired,          // 订单已过期
  kUnknown,          // 未知状态
};

struct Balance {
  std::string asset;
  double free;
};

class BinanceAccount {
 public:
  void set(const std::string& asset, double free) { map_.emplace(asset, free); }

  double get(const std::string& asset) {
    auto iter = map_.find(asset);
    return iter->second;
  }

 private:
  std::unordered_map<std::string, double> map_;
};

class BinanceClient {
 public:
  BinanceClient(std::string api_key, std::string secret_key)
      : api_key_(api_key), secret_key_(secret_key) {}

  void set_proxy(const std::string& proxy) { rest_client_.set_proxy(proxy); }

  ErrorOr<std::vector<Candle>> kline(const std::string& symbol,
                                     int64_t start_time, int64_t end_time,
                                     const std::string& interval,
                                     int limit = 1000);

  ErrorOr<int64_t> buy_limit_order(const std::string& symbol, float quantity,
                                   float price);
  ErrorOr<int64_t> sell_limit_order(const std::string& symbol, float quantity,
                                    float price);
  ErrorOr<OrderState> query_order(const std::string& symbol, int64_t order_id);
  ErrorOr<bool> cancel_order(const std::string& symbol, int64_t order_id);
  ErrorOr<BinanceAccount> get_account();

 private:
  RestClient rest_client_;
  std::string api_key_;
  std::string secret_key_;
};

}  // namespace wedge