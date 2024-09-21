#include "wedge/trade/trade_engine.h"

#include "wedge/binance/market/klines.h"
#include "wedge/binance/trade/account.h"
#include "wedge/binance/trade/get_order.h"
#include "wedge/binance/trade/new_order.h"
#include "wedge/common/chrono.h"

namespace wedge {

std::optional<Candle> TradeEngine::get_kline(BinanceHttpClient &client) {
  using namespace market;
  auto current_time = SystemClock::now();
  auto start_time = (current_time - 30min).time_since_epoch();
  auto end_time = current_time.time_since_epoch();
  auto response = client.send_sync(
      Klines(symbol_, KlineInterval::kMinutes30)
          .start_time(duration_cast<Milliseconds>(start_time).count())
          .end_time(duration_cast<Milliseconds>(end_time).count()));
  if (response.has_error()) {
    logger_->error("Get kline error: {}", response.error().message());
    return std::nullopt;
  }
  auto &json_data = response->body();
  logger_->info("Get kline paylod: {}", json_data.dump());

  auto get_double = [](const json &value) -> double {
    return std::stod(value.get<std::string>());
  };

  std::vector<Candle> candles;
  for (auto &kline : response->body()) {
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
    candles.push_back(candle);
  }

  if (candles.empty()) {
    return std::nullopt;
  }

  return candles.front();
}

std::optional<Account> TradeEngine::get_account(BinanceHttpClient &client) {
  auto response = client.send_sync(trade::Account());
  if (response.has_error()) {
    logger_->error("Get kline error: {}", response.error().message());
    return std::nullopt;
  }
  auto &json_data = response->body();
  logger_->info("Get kline paylod: {}", json_data.dump());
}

bool TradeEngine::update_orders(BinanceHttpClient &client) {
  using namespace trade;
  auto iter = orders_.begin();
  while (iter != orders_.end()) {
    auto response = client.send_sync(GetOrder(symbol_).order_id(iter->second));
    if (response.has_error()) {
      logger_->error("Get order error: {}", response.error().message());
      return false;
    }
    auto &json_data = response->body();
    logger_->info("Get order paylod: {}", json_data.dump());

    if (json_data["status"] == "FILLED") {
      strategy_->on_order_filled(iter->first);
      iter = orders_.erase(iter);
    } else {
      ++iter;
    }
  }

  return true;
}

void TradeEngine::run() {
  for (;; std::this_thread::sleep_for(30min)) {
    asio::io_context io_context;
    ssl::context ssl_context(ssl::context::tlsv12_client);
    ssl_context.set_verify_mode(ssl::verify_none);
    BinanceHttpClient client(io_context, ssl_context);
    client.connect_sync();

    // 1. 获取当前账户信息

    // 2. 跟新所有的订单
    if (!update_orders(client)) {
      continue;
    }

    // 3. 获取kline
    if (auto candle = get_kline(client); candle) {
      strategy_->update(*candle);
    } else {
      continue;
    }

    client.shutdown_sync();
  }
}

// Candle TradeEngine::get_kline() {

// }

}  // namespace wedge