#include <format>
#include <iostream>

#include "backtest/backtest_context.h"
#include "backtest/data_loader.h"
#include "strategy/strategy.h"

using namespace wedge;

int main() {
  BacktestContext context(100, 0);
  auto broker = context.broker();
  auto strategy = grid_strategy(std::move(broker), 10, 1e-4, 5);
  context.set_strategy(std::move(strategy));
  auto loader = sql_data_loader("D:\\workspace\\wedge-project\\dataset\\BTCUSDT_1m_klines.db", "2024-01-01");

  context.run(std::move(loader));

  auto& account = context.account();
  std::println(std::cout, "{} {}", account.balance(), account.position());
  return 0;
}