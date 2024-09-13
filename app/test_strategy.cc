#include <cstdio>
#include <fstream>
#include <nlohmann/json.hpp>

#include "backtest/backtest_context.h"
#include "backtest/data_loader.h"
#include "common/format.h"
#include "strategy/strategy.h"

using namespace wedge;

struct StrategyConfig {
  double balance;
  std::string start_time;
  std::string end_time;
  int grid_count;
  double grid_spacing;
  std::string dataset;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(StrategyConfig, balance, start_time, end_time,
                                 grid_count, grid_spacing, dataset)
};

int main() {
  auto config_path = PROJECT_ROOT_DIR "/.wedge/strategy.json";
  std::ifstream config_file(config_path);
  nlohmann::json j;
  config_file >> j;
  auto config = j.get<StrategyConfig>();

  auto logger = spdlog::basic_logger_st("backtest",
                                        PROJECT_ROOT_DIR "/logs/out.log", true);
  logger->set_pattern("[%^%l%$] %v");

  BacktestContext context(config.balance, 0, 0.001);
  auto broker = context.broker();
  auto strategy = grid_strategy(broker.get(), logger, config.grid_count,
                                config.grid_spacing);
  context.set_strategy(std::move(strategy));

  auto dataset_path = PROJECT_ROOT_DIR "/dataset/" + config.dataset;
  auto loader =
      sql_data_loader(dataset_path, config.start_time, config.end_time);

  context.set_logger(logger);

  context.run(std::move(loader));

  auto& account = context.account();
  return 0;
}