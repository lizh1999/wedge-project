#include <spdlog/sinks/basic_file_sink.h>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "binance/binance_broker.h"
#include "strategy/strategy.h"

using namespace wedge;

struct BinanceApiKey {
  std::string api_key;
  std::string secret_key;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(BinanceApiKey, api_key, secret_key)
};

struct Proxy {
  std::string proxy;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Proxy, proxy)
};

int main() {
  std::string config_path = PROJECT_ROOT_DIR "/.wedge/binance_api_key.json";
  std::ifstream config_file(config_path);
  if (!config_file.is_open()) {
    std::cerr << "File not found " << config_path << "\n";
    return EXIT_FAILURE;
  }
  nlohmann::json config_j;
  config_file >> config_j;
  auto binance_api_key = config_j.get<BinanceApiKey>();

  BinanceClient client(binance_api_key.api_key, binance_api_key.secret_key);
  std::string proxy_path = PROJECT_ROOT_DIR "/.wedge/proxy.json";
  std::ifstream proxy_file(proxy_path);
  if (proxy_file.is_open()) {
    nlohmann::json proxy_j;
    proxy_file >> proxy_j;
    auto p = proxy_j.get<Proxy>();
    if (!p.proxy.empty()) {
      client.set_proxy(p.proxy);
    }
  }

  auto logger =
      spdlog::basic_logger_st("main", PROJECT_ROOT_DIR "/logs/main.log", true);

  BinanceBroker broker("BTCUSDT", &client, logger);
  auto strategy = grid_strategy(&broker, logger, 5, 0.08);

  Timer timer;
  std::vector<OrderIndex> orders;

  std::function<void()> update_candle;
  std::function<void()> update_orders;
  
  update_orders = [&] {
    for (auto index : orders) {
      if (broker.query(index)) {
        strategy->on_order_filled(index);
      }
    }
    timer.run_after(1min, update_orders);
  };


  update_candle = [&] {
  };

  timer.run_after(0s, update_orders);
  

  return 0;
}