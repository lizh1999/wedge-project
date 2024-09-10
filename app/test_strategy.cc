#include <cstdio>

#include "backtest/backtest_context.h"
#include "backtest/data_loader.h"
#include "strategy/strategy.h"

using namespace wedge;

int main() {
  auto logger = spdlog::basic_logger_st("backtest",
                                        PROJECT_ROOT_DIR "/logs/out.log", true);
  logger->set_pattern("[%^%l%$] %v");

  BacktestContext context(100, 0, 0.001);
  auto broker = context.broker();
  auto strategy = grid_strategy(std::move(broker), logger, 50, 0.0001, 1000);
  context.set_strategy(std::move(strategy));
  auto loader =
      sql_data_loader(PROJECT_ROOT_DIR "/dataset/BTCUSDT_1m_klines.db",
                      "2024-07-01", "2024-08-01");

  context.set_logger(logger);

  context.run(std::move(loader));

  auto& account = context.account();
  return 0;
}