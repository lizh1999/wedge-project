#include <fmt/core.h>

#include <fstream>
#include <iostream>

#include "wedge/binance/binance_http_client.h"
#include "wedge/binance/market/klines.h"
#include "wedge/common/chrono.h"
#include "wedge/dataset/sql_dataset.h"

using namespace wedge;

asio::awaitable<void> download(ssl::context &ssl_context, SqlDataset &dataset,
                               std::string_view symbol,
                               market::KlineInterval interval) {
  auto executor = co_await asio::this_coro::executor;
  auto now = SystemClock::now();

  BinanceHttpClient client(executor, ssl_context);
  auto ec = co_await client.connect();
  if (ec) {
    std::cerr << "Connect error: " << ec.message() << std::endl;
    co_return;
  }

  int64_t start_time = dataset.get_max_start_time();
  int64_t end_time =
      duration_cast<Milliseconds>(now.time_since_epoch()).count();

  while (start_time < end_time) {
    auto response = co_await client.send(market::Klines(symbol, interval)
                                             .start_time(start_time)
                                             .end_time(end_time)
                                             .limit(1000));
    if (response.has_error()) {
      std::cerr << "Send error: " << response.error().message() << std::endl;
      static int count = 0;
      if (++count >= 3) co_return;
      co_await client.connect();
      continue;
    }

    auto get_double = [](const json &value) -> double {
      return std::stod(value.get<std::string>());
    };

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
      dataset.insert(candle);
      start_time = candle.close_time;
    }
  }
}

struct Task {
  std::string symbol;
  std::string interval;
  std::string filename;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Task, symbol, interval, filename)
};

struct Config {
  std::vector<Task> tasks;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Config, tasks)
};

asio::awaitable<void> async_main() {
  std::string config_path = PROJECT_ROOT_DIR "/.wedge/dataset.json";
  std::ifstream config_file(config_path);
  ssl::context ssl_context(ssl::context::tlsv12_client);
  ssl_context.set_verify_mode(ssl::verify_none);

  json json_data;
  config_file >> json_data;
  auto config = json_data.get<Config>();

  for (const auto &task : config.tasks) {
    auto path = fmt::format(PROJECT_ROOT_DIR "/dataset/{}", task.filename);
    SqlDataset dataset(path);
    co_await download(ssl_context, dataset, task.symbol,
                      market::from_str(task.interval));
  }

  co_return;
}

int main() {
  asio::io_context io_context;
  asio::co_spawn(io_context, async_main(), [](std::exception_ptr ptr) {
    try {
      if (ptr) std::rethrow_exception(ptr);
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
    }
  });
  io_context.run();
  return 0;
}