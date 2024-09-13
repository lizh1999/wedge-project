#include "binance/binance_client.h"

#include <cstdio>

#include <openssl/hmac.h>
#include <nlohmann/json.hpp>

namespace wedge {

static std::string bin2hex(const unsigned char* bin, unsigned int len) {
  std::string hex_str(len * 2, '\0');
  for (unsigned int i = 0; i < len; ++i) {
    sprintf(&hex_str[i * 2], "%02x", bin[i]);
  }
  return hex_str;
}

static int64_t current_timestamp() {
  using namespace std::chrono;
  auto duration = system_clock::now().time_since_epoch();
  return duration_cast<milliseconds>(duration).count();
}

class URLBuilder {
 public:
  URLBuilder& set(std::string_view key, std::string_view value) {
    if (!query_string_.empty()) {
      query_string_.push_back('&');
    }
    query_string_.append(key);
    query_string_.push_back('=');
    query_string_.append(value);
    return *this;
  }

  URLBuilder& set(std::string_view key, int64_t value) {
    char buffer[128];
    sprintf(buffer, "%lld", static_cast<long long>(value));
    return set(key, buffer);
  }

  URLBuilder& set(std::string_view key, double value) {
    char buffer[128];
    sprintf(buffer, "%.8lf", value);
    return set(key, buffer);
  }

  URLBuilder& sign_query_string(std::string_view secret_key) {
    std::string signature = create_signature(secret_key);
    return set("signature", signature);
  }

  std::string url(std::string_view base_url, std::string_view target) const {
    std::string result;
    result.append(base_url);
    result.append(target);
    result.push_back('?');
    result.append(query_string_);
    return result;
  }

 private:
  std::string create_signature(std::string_view secret_key) const {
    unsigned char hash[EVP_MAX_MD_SIZE];
    uint32_t length;
    HMAC(EVP_sha256(), secret_key.data(), secret_key.size(),
         (uint8_t*)query_string_.data(), query_string_.size(), hash, &length);
    return bin2hex(hash, length);
  }
  std::string query_string_;
};

static const char* kBaseURL = "https://api.binance.com";

ErrorOr<std::vector<Candle>> BinanceClient::kline(const std::string& symbol,
                                                  int64_t start_time,
                                                  int64_t end_time,
                                                  const std::string& interval,
                                                  int limit) {
  URLBuilder builder;
  // clang-format off
  std::string url = builder
    .set("interval", interval)
    .set("startTime", start_time)
    .set("endTime", end_time)
    .set("limit", (int64_t)limit)
    .url(kBaseURL, "/api/v3/klines");
  // clang-format on
  auto response = rest_client_.get(url);
  if (!response.has_value()) {
    return response.take_error();
  }
  auto json_data = nlohmann::json::parse(response.value());
  
  std::vector<Candle> result;
  for (const auto &kline : json_data) {
    Candle candle;
    auto parse_float = [&](double *data, int index) {
      std::string str = kline[index].get<std::string>();
      sscanf(str.c_str(), "%lf", data);
    };
    candle.open_price = kline[0].get<int64_t>();
    candle.close_price = kline[6].get<int64_t>();
    parse_float(&candle.open_price, 1);
    parse_float(&candle.high_price, 2);
    parse_float(&candle.low_price, 3);
    parse_float(&candle.close_price, 4);
    result.push_back(candle);
  }
  return result;
}

ErrorOr<int64_t> BinanceClient::buy_limit_order(const std::string& symbol,
                                                float quantity, float price) {
  URLBuilder builder;

  // clang-format off
  std::string url = builder
    .set("symbol", symbol)
    .set("side", "BUY")
    .set("type", "LIMIT")
    .set("timeInForce", "GTC")
    .set("quantity", quantity)
    .set("price", price)
    .set("timestamp", current_timestamp())
    .sign_query_string(secret_key_)
    .url(kBaseURL, "/api/v3/order");
  // clang-format on

  HttpHeaders headers;
  headers.add_header("X-MBX-APIKEY:" + api_key_);

  auto response = rest_client_.post(url, "", headers);
  if (!response.has_value()) {
    return response.take_error();
  }
  auto json_data = nlohmann::json::parse(response.value());
  return json_data["orderId"].get<int64_t>();
}

ErrorOr<int64_t> BinanceClient::sell_limit_order(const std::string& symbol,
                                                 float quantity, float price) {
  URLBuilder builder;

  // clang-format off
  std::string url = builder
    .set("symbol", symbol)
    .set("side", "SELL")
    .set("type", "LIMIT")
    .set("timeInForce", "GTC")
    .set("quantity", quantity)
    .set("price", price)
    .set("timestamp", current_timestamp())
    .sign_query_string(secret_key_)
    .url(kBaseURL, "/api/v3/order");
  // clang-format on

  HttpHeaders headers;
  headers.add_header("X-MBX-APIKEY:" + api_key_);

  auto response = rest_client_.post(url, "", headers);
  if (!response.has_value()) {
    return response.take_error();
  }

  auto json_data = nlohmann::json::parse(response.value());
  return json_data["orderId"].get<int64_t>();
}

ErrorOr<OrderState> BinanceClient::query_order(const std::string& symbol,
                                               int64_t order_id) {
  URLBuilder builder;

  // clang-format off
  std::string url = builder
    .set("symbol", symbol)
    .set("orderId", order_id)
    .set("timestamp", current_timestamp())
    .sign_query_string(secret_key_)
    .url(kBaseURL, "/api/v3/order");
  // clang-format on

  HttpHeaders headers;
  headers.add_header("X-MBX-APIKEY:" + api_key_);

  auto response = rest_client_.get(url, headers);
  if (!response.has_value()) {
    return response.take_error();
  }

  auto json_data = nlohmann::json::parse(response.value());
  std::string status = json_data["status"].get<std::string>();

  if (status == "NEW")
    return OrderState::kNew;
  if (status == "PARTIALLY_FILLED")
    return OrderState::kPartiallyFilled;
  if (status == "CANCELED")
    return OrderState::kCanceled;
  if (status == "PENDING_CANCEL")
    return OrderState::kPendingCancel;
  if (status == "REJECTED")
    return OrderState::kRecjected;
  if (status == "EXPIRED")
    return OrderState::kExpired;
  return OrderState::kUnknown;
}

ErrorOr<bool> BinanceClient::cancel_order(const std::string& symbol,
                                          int64_t order_id) {
  URLBuilder builder;

  // clang-format off
  std::string url = builder
    .set("symbol", symbol)
    .set("orderId", std::to_string(order_id))
    .set("timestamp", current_timestamp())
    .sign_query_string(secret_key_)
    .url(kBaseURL, "/api/v3/order");
  // clang-format on

  HttpHeaders headers;
  headers.add_header("X-MBX-APIKEY:" + api_key_);

  auto response = rest_client_.del(url, headers);
  if (!response.has_value()) {
    return response.take_error();
  }

  fprintf(stderr, "%s\n", response.value().c_str());

  auto json_data = nlohmann::json::parse(response.value());
  std::string status = json_data["status"].get<std::string>();

  return status == "CANCELED";
}

ErrorOr<std::vector<Balance>> BinanceClient::get_account() {
  URLBuilder builder;

  // clang-format off
  std::string url = builder
    .set("timestamp", current_timestamp())
    .sign_query_string(secret_key_)
    .url(kBaseURL, "/api/v3/account");
  // clang-format on

  HttpHeaders headers;
  headers.add_header("X-MBX-APIKEY:" + api_key_);

  auto response = rest_client_.get(url, headers);
  if (!response.has_value()) {
    return response.take_error();
  }

  std::vector<Balance> balances;
  auto json_data = nlohmann::json::parse(response.value());
  for (const auto& item : json_data["balances"]) {
    Balance balance;
    balance.asset = item["asset"].get<std::string>();
    auto free = item["free"].get<std::string>();
    sscanf(free.c_str(), "%lf", &balance.free);
    balances.push_back(balance);
  }
  return balances;
}

}  // namespace wedge