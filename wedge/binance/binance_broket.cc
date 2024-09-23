#include <spdlog/spdlog.h>

#include "wedge/binance/binance_broker.h"
#include "wedge/binance/binance_http_client.h"
#include "wedge/binance/market/klines.h"
#include "wedge/binance/market/ping.h"
#include "wedge/binance/market/time.h"
#include "wedge/binance/trade/account.h"
#include "wedge/binance/trade/cancel_order.h"
#include "wedge/binance/trade/get_order.h"
#include "wedge/binance/trade/new_order.h"

namespace wedge {

static std::optional<json> requset_once(const Request& request,
                                        spdlog::logger& logger) {
  asio::io_context io_context;
  ssl::context ssl_context(ssl::context::tlsv12_client);
  ssl_context.set_verify_mode(ssl::verify_none);
  BinanceHttpClient client(io_context, ssl_context);
  auto ec = client.connect_sync();
  if (ec) {
    logger.warn("Binance connect error: {}", ec.message());
    return std::nullopt;
  }
  auto response = client.send_sync(request);
  if (response.has_error()) {
    logger.warn("Binance send error: {}", response.error().message());
    return std::nullopt;
  }

  client.shutdown_sync();

  if (response->result() != http::status::ok) {
    logger.warn("Binance send with error code: {} and payload: {}",
                (int)response->result(), response->body().dump());
    return std::nullopt;
  }

  auto json_data = std::move(response->body());
  logger.trace("Binance response: {}", json_data.dump());

  return json_data;
}

static std::string to_string(const Request& request) {
  std::string result;
  switch (request.method) {
    case http::verb::get:
      result.append("GET ");
      break;
    case http::verb::post:
      result.append("POST ");
      break;
    case http::verb::delete_:
      result.append("DELETE ");
      break;
    case http::verb::put:
      result.append("PUT ");
  }
  result.append(request.path);
  for (auto [key, value] : request.params) {
    result.append("\n\t" + key + ":" + value);
  }
  return result;
}

static json request_until(const Request& request, spdlog::logger& logger,
                          int retry_count) {
  while (retry_count--) {
    auto json_data = requset_once(request, logger);
    if (json_data.has_value()) {
      return *json_data;
    }
  }
  logger.error("Request failed : {}", to_string(request));
  std::abort();
}

void BinanceBroker::ping() {
  using namespace market;
  request_until(Ping(), *logger_, retry_count_);
}

int64_t BinanceBroker::get_time() {
  using namespace market;
  auto json_data = request_until(Time(), *logger_, retry_count_);
  return json_data["serverTime"];
}

static double get_double(const json& value) {
  return std::stod(value.get<std::string>());
};

std::vector<Candle> BinanceBroker::get_klines(std::string_view symbol,
                                              std::string interval,
                                              int64_t start_time,
                                              int64_t end_time) {
  using namespace market;
  auto json_data = request_until(         //
      Klines(symbol, from_str(interval))  //
          .start_time(start_time)         //
          .end_time(end_time)             //
          .limit(1000),                   //
      *logger_, retry_count_);

  std::vector<Candle> result;
  for (auto& kline : json_data) {
    Candle candle;
    candle.open_time = kline[0].get<int64_t>();
    candle.close_time = kline[6].get<int64_t>();
    candle.open_price = get_double(kline[1]);
    candle.high_price = get_double(kline[2]);
    candle.low_price = get_double(kline[3]);
    candle.close_price = get_double(kline[4]);
    candle.volume = get_double(kline[5]);
    candle.quote_volume = get_double(kline[7]);
    candle.traders = kline[8].get<int>();
    candle.taker_buy_base = get_double(kline[9]);
    candle.taker_buy_quote = get_double(kline[10]);
    result.push_back(candle);
  }
  return result;
}

std::optional<double> BinanceAccount::free(const std::string& symbol) {
  auto it = balances_.find(symbol);
  if (it == balances_.end()) {
    return std::nullopt;
  }
  return it->second.first;
}

std::optional<double> BinanceAccount::locked(const std::string& symbol) {
  auto it = balances_.find(symbol);
  if (it == balances_.end()) {
    return std::nullopt;
  }
  return it->second.second;
}

struct BinanceBalance {
  std::string asset;
  std::string free;
  std::string locked;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(BinanceBalance, asset, free, locked)
};

BinanceAccount BinanceBroker::get_account() {
  using namespace trade;
  auto json_data = request_until(            //
      Account().credentials(*credentials_),  //
      *logger_, retry_count_                 //
  );

  BinanceAccount account;
  auto& balances = account.balances_;
  for (auto& item : json_data["balances"]) {
    auto balance = item.get<BinanceBalance>();
    auto free = get_double(balance.free);
    auto locked = get_double(balance.locked);
    balances.emplace(balance.asset, std::pair(free, locked));
  }

  return account;
}

static uint64_t create_order(const Request& request, spdlog::logger& logger,
                             int retry_count) {
  return request_until(request, logger, retry_count)["orderId"];
}

uint64_t BinanceBroker::create_limit_buy_order(std::string_view symbol,
                                               double quantity, double price) {
  using namespace trade;
  return create_order(                                      //
      NewOrder(symbol, Side::kBuy, "LIMIT")                 //
          .quantity(quantity)                               //
          .price(price)                                     //
          .new_order_resp_type(NewOrderResponseType::kAck)  //
          .time_in_force(TimeInForce::kGtc)                 //
          .credentials(*credentials_),                      //
      *logger_, retry_count_);
}

uint64_t BinanceBroker::create_limit_sell_order(std::string_view symbol,
                                                double quantity, double price) {
  using namespace trade;
  return create_order(                                      //
      NewOrder(symbol, Side::kSell, "LIMIT")                //
          .quantity(quantity)                               //
          .price(price)                                     //
          .new_order_resp_type(NewOrderResponseType::kAck)  //
          .time_in_force(TimeInForce::kGtc)                 //
          .credentials(*credentials_),                      //
      *logger_, retry_count_);
}

uint64_t BinanceBroker::create_market_buy_order(std::string_view symbol,
                                                double quantity) {
  using namespace trade;
  return create_order(                                      //
      NewOrder(symbol, Side::kBuy, "MARKET")                //
          .quantity(quantity)                               //
          .new_order_resp_type(NewOrderResponseType::kAck)  //
          .time_in_force(TimeInForce::kGtc)                 //
          .credentials(*credentials_),                      //
      *logger_, retry_count_);
}

uint64_t BinanceBroker::create_market_sell_order(std::string_view symbol,
                                                 double quantity) {
  using namespace trade;
  return create_order(                                      //
      NewOrder(symbol, Side::kSell, "MARKET")               //
          .quantity(quantity)                               //
          .new_order_resp_type(NewOrderResponseType::kAck)  //
          .time_in_force(TimeInForce::kGtc)                 //
          .credentials(*credentials_),                      //
      *logger_, retry_count_);
}

bool BinanceBroker::get_order(std::string_view symbol, uint64_t order_id) {
  using namespace trade;
  auto json_data = request_until(       //
      GetOrder(symbol)                  //
          .order_id(order_id)           //
          .credentials(*credentials_),  //
      *logger_, retry_count_);
  return json_data["status"] == "FILLED";
}

void BinanceBroker::cancel_order(std::string_view symbol, uint64_t order_id) {
  using namespace trade;
  auto json_data = request_until(       //
      CancelOrder(symbol)               //
          .order_id(order_id)           //
          .credentials(*credentials_),  //
      *logger_, retry_count_);
  if (json_data["status"] != "CANCELED") {
    logger_->error("Cancel order filled with payload: {}", json_data.dump());
  }
}

}  // namespace wedge