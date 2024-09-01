#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include "binance/binance_client.h"
#include "common/format.h"

using namespace wedge;

void test_buy(BinanceClient& client) {
  auto order_id = client.buy_limit_order("BTCUSDT", 1, 5);
  if (!order_id.has_value()) {
    std::cerr << order_id.error().message() << "\n";
    return;
  }
  std::cout << order_id.value() << "\n";
}

void test_account(BinanceClient& client) {
  auto balances = client.get_account();

  if (!balances.has_value()) {
    std::cerr << balances.error().message() << "\n";
    return;
  }

  for (auto& balance : balances.value()) {
    std::cout << balance.asset << " " << balance.free << "\n";
  }
}

struct BinanceApiKey {
  std::string api_key;
  std::string secret_key;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(BinanceApiKey, api_key, secret_key)
};

int main() {
  std::string config_path =
      format("{}/.wedge/binance_api_key.json", PROJECT_ROOT_DIR);
  std::ifstream config_file(config_path);
  if (!config_file.is_open()) {
    std::cerr << "File not found " << config_path << "\n";
    return EXIT_FAILURE;
  }
  nlohmann::json j;
  config_file >> j;
  auto binance_api_key = j.get<BinanceApiKey>();

  BinanceClient client(binance_api_key.api_key, binance_api_key.secret_key);

  client.set_proxy("127.0.0.1:7890");
  // test_buy(client);
  test_account(client);
  return 0;
}