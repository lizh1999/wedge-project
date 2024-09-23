#pragma once

#include <spdlog/spdlog.h>

#include <optional>
#include <string>
#include <unordered_map>

#include "wedge/binance/credentials.h"
#include "wedge/common/candle.h"

namespace wedge {

class BinanceAccount {
 public:
  std::optional<double> free(const std::string& symbol);
  std::optional<double> locked(const std::string& symbol);
 private:
  friend class BinanceBroker;
  std::unordered_map<std::string, std::pair<double, double>> balances_;
};

class BinanceBroker {
 public:
  static const int kDefaultRetryCount = 3;

  BinanceBroker(std::shared_ptr<spdlog::logger> logger,
                std::optional<Credentials> credentials = std::nullopt,
                int retry_count = kDefaultRetryCount)
      : logger_(logger), credentials_(credentials), retry_count_(retry_count) {}

  void ping();

  int64_t get_time();

  std::vector<Candle> get_klines(std::string_view symbol, std::string interval,
                                 int64_t start_time, int64_t end_time);

  BinanceAccount get_account();

  uint64_t create_limit_buy_order(std::string_view symbol, double quantity,
                                  double price);

  uint64_t create_limit_sell_order(std::string_view symbol, double quantity,
                                   double price);

  uint64_t create_market_buy_order(std::string_view symbol, double quantity);

  uint64_t create_market_sell_order(std::string_view symbol, double quantity);

  bool get_order(std::string_view symbol, uint64_t order_id);

  void cancel_order(std::string_view symbol, uint64_t order_id);

 private:
  std::shared_ptr<spdlog::logger> logger_;
  std::optional<Credentials> credentials_;
  int retry_count_;
};

}  // namespace wedge