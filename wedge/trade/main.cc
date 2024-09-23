#include <spdlog/sinks/basic_file_sink.h>

#include <fstream>

#include "wedge/strategy/strategy.h"
#include "wedge/trade/trade_engine.h"

using namespace wedge;

struct StrategyConfig {
  int grid_count;
  double grid_spacing;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(StrategyConfig, grid_count, grid_spacing)
};

int main() {
  std::ifstream api_file(PROJECT_ROOT_DIR "/.wedge/binance_api_key.json");
  std::ifstream strategy_file(PROJECT_ROOT_DIR "/.wedge/strategy.json");
  nlohmann::json json;

  api_file >> json;
  auto credentials = json.get<Credentials>();

  strategy_file >> json;
  auto strategy_config = json.get<StrategyConfig>();

  auto logger = spdlog::basic_logger_st(
      "backtest", PROJECT_ROOT_DIR "/logs/trade.log", true);

  logger->set_level(spdlog::level::trace);

  TradeEngine engine("BTCUSDT", credentials, logger);

  auto strategy =
      grid_strategy(strategy_config.grid_count, strategy_config.grid_spacing);

  engine.set_strategy(std::move(strategy));
  engine.run();

  return 0;
}